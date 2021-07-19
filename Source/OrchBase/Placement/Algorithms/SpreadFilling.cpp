#include "DevT_t.h"
#include "GraphT_t.h"
#include "SpreadFilling.h"

SpreadFilling::SpreadFilling(Placer* placer):Algorithm(placer)
{
    result.method = "spre";
}

/* Places a gi onto the engine held by a placer by spreading the devices as
 * "thinly" over the engine as possible. Is made more complicated than one
 * might think, due to:
 *
 * - The existence of device types
 *
 * - The restriction that devices of different types cannot be placed on the
 *   same core pair.
 *
 * - The caller can impose constraints on the number of devices that can be
 *   mapped to a thread, and on the number of threads to use on cores.
 *
 * Algorithm steps:
 *
 *  1. Get the total number of devices in this graph instance, and the number
 *     of each device type.
 *
 *  2. Collect every core in the engine that has no devices placed on it (we're
 *     going to use them all, selfishly).
 *
 *  3. Figure out how many cores should be "allocated" to each device type, to
 *     minimise the maximum device density across all device types.
 *
 *  4. If there aren't enough cores to hold each device type, or if there are
 *     too many devices of a given type to fit in the cores allocated to it due
 *     to constraints, throw a NoSpaceToPlaceException.
 *
 *  5. For each device type, using the information from 3, determine the cores
 *     to "reserve" * for devices of this type.
 *
 *  6. For each device type, distribute all devices of that type evenly across
 *     all of the "reserved" cores. Place devices into the first thread of each
 *     core.
 *
 *  7. For each core, distribute all devices placed on it evenly across all
 *     threads.
 *
 * This algorithm does not compute fitness, and only adheres to constraints in
 * a hardcoded manner.
 *
 * Returns zero. */
float SpreadFilling::do_it(GraphI_t* gi)
{
    result.startTime = placer->timestamp();

    /* 1. Get the total number of normal devices in this graph instance, and
     * the breakdown by device type. */
    std::map<DevT_t*, unsigned> deviceOfTypeCount;
    unsigned deviceCount = gi->DevicesByType(deviceOfTypeCount);

    /* 2. Get every unused core in the engine. */
    std::vector<P_core*> unusedCores;
    HardwareIterator coreIterator = HardwareIterator(placer->engine);
    P_core* currentCore = coreIterator.get_core();
    while (!coreIterator.has_wrapped())
    {
        unusedCores.push_back(currentCore);
        currentCore = coreIterator.next_core();
    }

    /* Don't use cores in core pairs that already have devices mapped to
     * them. */
    std::map<GraphI_t*, std::set<P_core*> >::iterator giIt;
    std::set<P_core*>::iterator coreIt;
    std::vector<P_core*>::iterator coresToRemove[2];
    for (giIt = placer->giToCores.begin();
         giIt != placer->giToCores.end(); giIt++)
    {
        for (coreIt = giIt->second.begin(); coreIt != giIt->second.end();
             coreIt++)
        {
            /* Safely remove the core and it's pair, if it has one. */
            coresToRemove[0] = std::find(unusedCores.begin(),
                                         unusedCores.end(),
                                         *coreIt);
            coresToRemove[1] = std::find(unusedCores.begin(),
                                         unusedCores.end(),
                                         (*coreIt)->pair);
            for (unsigned index = 0; index != 2; index++)
                if (coresToRemove[index] != unusedCores.end())
                    unusedCores.erase(coresToRemove[index]);
        }
    }

    /* 3. At this point, unusedCores holds every core (pointer) that has no
     * devices mapped to it, or its pair, in the engine. Consequently, we can
     * easily derive the number of available core pairs... though this is only
     * useful if cores are paired to begin with. Fortunately, we can exploit
     * the fact that, if the first core in the engine has a pair, then all of
     * the cores in the engine are paired (and vice versa).
     *
     * We care about this because we don't want to place two devices of
     * differing types on the same core pair: an even division will be less
     * efficient on a core-paired system.
     *
     * We can use the same algorithm for both the paired and unpaired cases, by
     * dividing the number of cores to distribute by two. */
    std::vector<P_core*>::size_type coreCount = unusedCores.size();
    if (currentCore->pair != PNULL) coreCount /= 2;  /* (:A:) this is a search
                                                      * marker. */

    /* This next bit finds, for each device type, how many cores should be
     * allocated to host it.
     *
     * To understand, consider an example in an unpaired system:
     *
     * Five cores available. Three device types, with device counts:
     *
     * A: 10
     * B: 20
     * C: 30
     *
     * making 60 devices total. Each of these as a fraction of 60:
     *
     * A: 1/6
     * B: 2/6
     * C: 3/5
     *
     * Fraction times core count:
     *
     * A: 5/6 ~ 0.83
     * B: 10/6 ~ 1.7
     * C: 15/6 = 2.5
     *
     * Round these ratios towards one:
     *
     * A: 1
     * B: 1
     * C: 2
     *
     * For each remaining available core, allocate it to the device type
     * with the largest device density. In this case:
     *
     *  - 'A' has 10 devices and 1 core, resulting in a density of 10.
     *  - 'B' has 20 devices and 1 core, resulting in a density of 20.
     *  - 'C' has 30 devices and 2 cores, resulting in a density of 15.
     *
     * So the remaining core goes to B, resulting in this distribution of
     * cores to device types:
     *
     * A: 1 (density 10)
     * B: 2 (density 10)
     * C: 2 (density 15) */
    std::map<DevT_t*, unsigned> coreAllocCounts;  /* Number of cores allocated
                                                   * to each device type. */
    unsigned coresAllocated = 0;
    std::map<DevT_t*, unsigned>::iterator devTypeIt;
    for (devTypeIt = deviceOfTypeCount.begin();
         devTypeIt != deviceOfTypeCount.end(); devTypeIt++)
    {
        float ratio = devTypeIt->second / deviceCount * coreCount;
        if (ratio < 1) coreAllocCounts[devTypeIt->first] = 1;
        else coreAllocCounts[devTypeIt->first] = floor(ratio);
        coresAllocated += coreAllocCounts[devTypeIt->first];
    }

    /* Allocate remaining core/pairs to device types with the greatest
     * density. Inefficient, but should only happen a couple of times at
     * most. */
    std::map<DevT_t*, unsigned>::iterator coreAllocIt;
    while (coresAllocated < coreCount)
    {
        /* Find the 'most densely packed' device type. */
        DevT_t* mostDense = PNULL;
        float greatestDensity = 0;
        for (coreAllocIt = coreAllocCounts.begin();
             coreAllocIt != coreAllocCounts.end(); coreAllocIt++)
        {
            float thisDensity = deviceOfTypeCount[coreAllocIt->first] /
                coreAllocCounts[coreAllocIt->first];
            if (thisDensity > greatestDensity)
            {
                mostDense = coreAllocIt->first;
                greatestDensity = thisDensity;
            }
        }

        /* Give it an extra core/pair. */
        coreAllocCounts[mostDense] += 1;
        coresAllocated++;
    }

    /* Undo the earlier division-by-two for core pairs at (:A:). */
    if (currentCore->pair != PNULL)
    {
        coreCount *= 2;
        coresAllocated *= 2;
        for (coreAllocIt = coreAllocCounts.begin();
             coreAllocIt != coreAllocCounts.end(); coreAllocIt++)
            coreAllocCounts[coreAllocIt->first] *= 2;
    }

    /* 4. A niche case - if there are more device types than core/pairs, then
     * this graph instance won't fit on the engine. We bail. */
    if (coresAllocated > coreCount)
        throw NoSpaceToPlaceException("[ERROR] Too many device types for the "
                                      "number of available cores!");

    /* Another niche case - if there are too many devices to fit on all the
     * (constrained) threads, we bail. */
    unsigned maxDevPerThr = placer->constrained_max_devices_per_thread(gi);
    unsigned maxThrPerCore = placer->constrained_max_threads_per_core(gi);
    for (devTypeIt = deviceOfTypeCount.begin();
         devTypeIt != deviceOfTypeCount.end(); devTypeIt++)
    {
        unsigned thisDevPerCore = devTypeIt->second /
            coreAllocCounts[devTypeIt->first];
        if (thisDevPerCore > maxDevPerThr * maxThrPerCore)
        {
            std::string dtName = devTypeIt->first->Name();
            throw NoSpaceToPlaceException(
                dformat("[ERROR] Not enough space to place all devices of type "
                        "'%s' in this gi.", dtName.c_str()));
        }
    }

    /* 5. For each normal device type, get a vector of cores to "deal" devices
     * to. We keep all devices of the same type close together - if that's not
     * suitable, change the logic that follows to distribute the devices across
     * the compute fabric. Could be a configuration option, perhaps.
     *
     * This iteration respects the declared order of device types in the graph
     * instance - earlier-occurences of device types will be placed on
     * "earlier" cores. */
    std::map<DevT_t*, std::vector<P_core*> > coreAllocs;
    coreIterator.reset_all_iterators();  /* A fresh hardware model walker */
    for (std::vector<DevT_t*>::iterator devTVIt = gi->pT->DevT_v.begin();
         devTVIt != gi->pT->DevT_v.end(); devTVIt++)
    {
        unsigned coresRemaining = coreAllocCounts[*devTVIt];
        while (coresRemaining > 0)
        {
            coreAllocs[*devTVIt].push_back(coreIterator.get_core());
            coresRemaining--;
            coreIterator.next_core();
        }
    }

    /* 6. Now we distribute all of the normal devices to these core/pairs we
     * computed earlier. The distribution is done "card-game" style to flatten
     * any remainder as much as possible.
     *
     * Each device type has an iterator associated with it, which 'spins
     * through' coreAllocs.
     *
     * Devices are all placed on the first thread of each core, and are
     * redistributed within that core after distribution is finished. */

    /* Initialise our iterators first */
    std::map<DevT_t*, std::vector<P_core*>::iterator > currentCores;
    std::map<DevT_t*, std::vector<P_core*> >::iterator tooManyDamnIterators;
    for (tooManyDamnIterators = coreAllocs.begin();
         tooManyDamnIterators != coreAllocs.end(); tooManyDamnIterators++)
    {
        DevT_t* thisDevType = tooManyDamnIterators->first;
        currentCores[thisDevType] = coreAllocs[thisDevType].begin();
    }

    /* Now we place our devices in. */
    WALKPDIGRAPHNODES(unsigned, DevI_t*,
                      unsigned, EdgeI_t*,
                      unsigned, PinI_t*, gi->G, device)
    {
        if ((*device).second.data->devTyp != 'D') continue;

        /* Grab the core to place on. */
        DevT_t* thisDevType = (*device).second.data->pT;
        std::vector<P_core*>::iterator* currentIterator;
        currentIterator = &(currentCores[thisDevType]);
        P_core* target = **currentIterator;

        /* Place the device on the first thread of that core. */
        placer->link(target->P_threadm.begin()->second, (*device).second.data);

        /* Increment the core iterator, wrapping as needed. */
        (*currentIterator)++;
        if ((*currentIterator) == coreAllocs[thisDevType].end())
            (*currentIterator) = coreAllocs[thisDevType].begin();
    }

    /* 7. Redistribute devices in cores to evenly load threads. */
    std::vector<P_core*>::iterator corevIt;
    for (corevIt = unusedCores.begin(); corevIt != unusedCores.end(); corevIt++)
        placer->redistribute_devices_in_core(*corevIt, gi);

    /* We out */
    result.endTime = placer->timestamp();
    return 0;
}

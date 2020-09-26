#include "SmartRandom.h"

SmartRandom::SmartRandom(Placer* placer):Algorithm(placer)
{
    result.method = "rand";
}

/* Places a gi onto the engine held by a placer using a random placer.
 *
 * This placer will probably fall over if you have equal numbers of devices of
 * multiple types, because this placer does not reserve cores for certain
 * device types (so it's highly likely to fall into a situation where a certain
 * device cannot be placed, because all of the cores have devices of other
 * types on them).
 *
 * Returns zero. */
float SmartRandom::do_it(GraphI_t* gi)
{
    DevI_t* device;
    P_core* core;
    P_thread* thread;
    UniqueDevT deviceType;
    std::map<UniqueDevT, std::set<P_core*> >::iterator validCoreIt, badCoreIt;

    result.startTime = placer->timestamp();

    /* Maximum number of devices to pack into a thread. */
    unsigned maxDevicesPerThread = \
        placer->constrained_max_devices_per_thread(gi);

    /* Maximum number of threads to use in each core. This constraint is
     * handled by limiting the range of thread selection. */
    unsigned maxThreadsPerCore = \
        placer->constrained_max_threads_per_core(gi);

    /* Know where we can put things. */
    placer->define_valid_cores_map(gi, &validCoresForDeviceType);

    /* Go over each device in turn */
    WALKPDIGRAPHNODES(unsigned, DevI_t*, unsigned, EdgeI_t*, unsigned,
                      PinI_t*, gi->G, deviceIterator)
    {
        device = gi->G.NodeData(deviceIterator);
        deviceType.gi = gi;
        deviceType.pT = device->pT;

        /* Ignore if it's not a normal device (we don't map those). */
        if (deviceType.pT->devTyp != 'D') continue;

        /* Keep choosing cores from the valid core map until we find one with
         * space. */
        while (true)
        {
            validCoreIt = validCoresForDeviceType.find(deviceType);

            /* If we can't place the device (because there are no cores left
             * for it), there's not much we can do. Just leave and let the
             * integrity checker help out. */
            if (validCoreIt->second.empty()) return -1;

            /* Again, back to choosing a core... */
            std::set<P_core*>::iterator coreIt;
            coreIt = validCoreIt->second.begin();
            std::advance(coreIt, rand() % validCoreIt->second.size());
            core = *coreIt;

            /* Find the first thread on that core that has space. */
            unsigned threadInCore = 1; /* Count of the thread within the
                                        * core. NB: Not the index! We start
                                        * from one, to make counting easier
                                        * w.r.t. maxThreadsPerCore. */
            std::map<AddressComponent, P_thread*>::iterator threadIt;
            std::map<AddressComponent, P_thread*>::iterator exitCondition;
            exitCondition = core->P_threadm.end();
            threadIt = core->P_threadm.begin();
            while (threadIt != exitCondition)
            {
                if (placer->threadToDevices[threadIt->second].size() <
                    maxDevicesPerThread) break;
                threadIt++;

                /* If we've gone through too many threads, leave the loop and
                 * enter the next one, which removes the core from the list and
                 * makes us choose another core. */
                threadInCore++;
                if (threadInCore > maxThreadsPerCore) threadIt = exitCondition;
            }

            /* If we couldn't find a thread on that core that has space, remove
             * that core from the valid cores map (for all device types). */
            if (threadIt == core->P_threadm.end())
            {
                for (badCoreIt = validCoresForDeviceType.begin();
                     badCoreIt != validCoresForDeviceType.end(); badCoreIt++)
                {
                    badCoreIt->second.erase(core);
                }

                /* Choose another core for this device. */
                continue;
            }

            /* Otherwise, we've found a suitable thread for this core. */
            thread = threadIt->second;
            break;
        }

        /* Put the device on the selected thread, and remove the core (pair, if
         * appropriate) from the available set of cores for other device
         * types. */
        placer->link(thread, device);
        for (badCoreIt = validCoresForDeviceType.begin();
             badCoreIt != validCoresForDeviceType.end(); badCoreIt++)
        {
            if (badCoreIt->first != deviceType)
            {
                badCoreIt->second.erase(core);
                if (core->pair != PNULL) badCoreIt->second.erase(core->pair);
            }
        }
    }

    /* Redistribute devices in cores to evenly load threads. */
    placer->redistribute_devices_in_gi(gi);

    result.endTime = placer->timestamp();
    return 0;
}

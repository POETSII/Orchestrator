#include "DevT_t.h"
#include "GraphT_t.h"
#include "ThreadFilling.h"

ThreadFilling::ThreadFilling(Placer* placer):Algorithm(placer)
{
    result.method = "tfil";
}

/* Places a gi onto the engine held by a placer using a naive thread-filling
 * algorithm.
 *
 * This algorithm is very basic - adherence to constraints are hardcoded where
 * appropriate. It does not compute fitness.
 *
 * Returns zero. */
float ThreadFilling::do_it(GraphI_t* gi)
{
    result.startTime = placer->timestamp();

    /* Start from the first core pair in the engine that has no devices
     * associated with it. This method uses one hardware iterator, and the pair
     * member of P_core to modulate the core placed on. */
    HardwareIterator hardwareIt(placer->engine);
    bool onLowerCore = true;  /* Binding unneccesary, but helps with
                               * understanding I think. */

    /* Staging area to hold all devices of a given type in a gi. The filling
     * mechanism places all devices of the first type, followed by all devices
     * of the second type, and so on until the cows come home, wherever that
     * is. */
    std::vector<DevI_t*> devicesOfType;

    /* Maximum number of devices to pack into a thread. */
    unsigned maxDevicesPerThread = \
        placer->constrained_max_devices_per_thread(gi);

    /* Maximum number of threads to use in each core. */
    unsigned maxThreadsPerCore = \
        placer->constrained_max_threads_per_core(gi);

    /* Walk through each device type in the gi. */
    std::vector<DevT_t*>::iterator deviceTypeIterator;
    for (deviceTypeIterator = gi->pT->DevT_v.begin();
         deviceTypeIterator != gi->pT->DevT_v.end();
         deviceTypeIterator++)
    {
        /* Skip this device type if it is not a normal device type. We don't
         * place those. */
        if ((*deviceTypeIterator)->devTyp != 'D') continue;

        /* Move the hardware iterator to the first available empty core pair in
         * the engine. Does not move the iterators if the current core-pair
         * is empty. */
        poke_iterator(hardwareIt);
        onLowerCore = true;

        /* Count of the thread within the core. NB: Not the index! We start
         * from one, to make counting easier w.r.t. maxThreadsPerCore. */
        unsigned threadInCore = 1;

        /* If the iterator has wrapped, we've run out of space. */
        if (hardwareIt.has_wrapped()) throw NoSpaceToPlaceException(
            "[ERROR] Ran out of space placing a gi.");

        /* Walk through each device for this device type. */
        gi->DevicesOfType(*deviceTypeIterator, devicesOfType);
        std::vector<DevI_t*>::iterator deviceIterator;
        for (deviceIterator = devicesOfType.begin();
             deviceIterator != devicesOfType.end(); deviceIterator++)
        {
            /* If the current thread is full, try the next thread (until we run
             * out of threads on this core). Note the use of indexing instead
             * of `at` for the map. */
            P_thread* thisThread = hardwareIt.get_thread();
            if (placer->threadToDevices[thisThread].size()
                >= maxDevicesPerThread)
            {
                /* Move to the next thread. */
                hardwareIt.has_core_changed();  /* Reset flag! */
                hardwareIt.next_thread();
                threadInCore++;
                bool didCoreChange = hardwareIt.has_core_changed();

                /* If we did not move to a new core using the `next_thread`
                 * call, and we've reached the limit on the number of threads
                 * we can use on this core, we move to the next core. */
                if (!didCoreChange and threadInCore > maxThreadsPerCore)
                {
                    hardwareIt.next_core();
                    didCoreChange = true;
                    if (!onLowerCore) onLowerCore = true;
                }

                /* Sort out core-pairing logic. If we moved to a new core at
                 * all for this device, */
                if (didCoreChange)
                {
                    threadInCore = 1;

                    /* If this core has no pair, or we're on the "upper" member
                     * of the core pair, move to the next empty core pair. */
                    if (hardwareIt.get_core()->pair == PNULL or
                        !onLowerCore)
                    {
                        poke_iterator(hardwareIt);
                        onLowerCore = true;
                    }
                    else onLowerCore = false;
                }

                thisThread = hardwareIt.get_thread();
            }

            /* Now we've found an appropriate thread for this device. Let's
             * drop it off... */
            placer->link(thisThread, *deviceIterator);

        }  /* End "for each device of this type" */
    }  /* End "for each device type in this problem" */

    result.endTime = placer->timestamp();
    return 0;
}

/* Returns true if none of the threads in a core hold any devices, and false
 * otherwise. */
bool ThreadFilling::is_core_empty(P_core* core)
{
    /* Iterate over the threads in this core. */
    std::map<AddressComponent, P_thread*>::iterator threadIt;
    for (threadIt = core->P_threadm.begin();
         threadIt != core->P_threadm.end(); threadIt++)
    {
        /* If the thread is not empty, we out. */
        if (!placer->threadToDevices[threadIt->second].empty()) return false;
    }
    return true;
}

/* Sets a hardware iterator to the lower core in the next empty core pair,
 * throwing if no such core pair exists. If the hardware iterator is in the
 * upper core of an empty core pair, it is left alone. Arguments:
 *
 * - hardwareIt: The hardware iterator, modified in place. */
void ThreadFilling::poke_iterator(HardwareIterator& hardwareIt)
{
    /* Reset flag. */
    hardwareIt.has_wrapped();

    /* Iterate until an empty core pair is found. Returning (or throwing) are
     * the only (intended) ways out of this loop. */
    while (true)
    {
        /* If we've gone through the entire structure and no spare core pair
         * has been found, then the engine is already full. */
        if (hardwareIt.has_wrapped()) throw NoSpaceToPlaceException(
            "[ERROR] Engine is full.");

        /* If the first core is empty, check if the paired core (if it has one)
         * is empty. If both are empty if they exist, we're done here. */
        P_core* core = hardwareIt.get_core();
        if (is_core_empty(core))
        {
            if (core->pair == PNULL) return;
            if (is_core_empty(core->pair)) return;
        }

        /* Otherwise, we conclude that this core pair has had some devices
         * placed on the contained threads. Now, we move the hardware iterator
         * to the lower core in the next core pair.
         *
         * Note that, the above construct is designed to iterate appropriately
         * regardless of whether the iterator is on a lower- or upper- member
         * of a core pair, or if the core has no pair. If it is on a
         * lower-member, two `next_core`s are called. If it is on an
         * upper-member or if there is no pair, only one `next_core` is
         * called. The pair of a pair (that exists) is itself. */
        if (hardwareIt.next_core()->pair == core) hardwareIt.next_core();
    }
}

#include "BucketFilling.h"

/* Places a task onto the engine held by a placer using a naive bucket-filling
 * algorithm.
 *
 * This algorithm is very basic - it does not adhere to constraints (for now),
 * it does not compute fitness (for now), and simply encapsulates the old
 * placement logic.
 *
 * This algorithm iterates through each device type in the task
 *
 * Returns zero (for now - later will return fitness). */
float BucketFilling::do_it(P_task* task)
{
    /* Start from the first core pair in the engine that has no devices
     * associated with it. */

    /* These two iterators represent neighbouring cores -
     * the zeroeth is always one core behind the first. */
    std::vector<HardwareIterator> hardwareIterators;
    hardwareIterators.push_back(HardwareIterator(placer->engine));  /* Odd */
    hardwareIterators.push_back(HardwareIterator(placer->engine));  /* Even */
    hardwareIterators[1].next_core();

    /* Staging area to hold all devices of a given type in a task. */
    vector<P_device*> devicesOfType;

    /* Maximum number of devices to pack into a thread. */
    unsigned maxDevicesPerThread = \
        placer->constrained_max_devices_per_thread(task);

    /* Walk through each device type in the task. */
    std::vector<P_devtyp*>::iterator deviceTypeIterator;
    for (deviceTypeIterator = task->pP_typdcl->P_devtypv.begin();
         deviceTypeIterator != task->pP_typdcl->P_devtypv.end();
         deviceTypeIterator++)
    {
        /* Skip this device type if it is a supervisor device type. */
        if (!(*deviceTypeIterator)->pOnRTS) continue;

        /* Move iterators to the next empty core pair. */
        poke_iterators(&hardwareIterators);

        /* Complain about running out of space if the ahead-iterator has
         * wrapped. */
        if (hardwareIterators[1].has_wrapped())
            throw NoSpaceToPlaceException("[ERROR] Ran out of space placing a "
                                          "task.");

        /* Walk through each device for this device type. */
        devicesOfType = task->pD->DevicesOfType(*deviceTypeIterator);
        std::vector<P_device*>::iterator deviceIterator;
        bool coreIncremented = false;
        for (deviceIterator = devicesOfType.begin();
             deviceIterator != devicesOfType.end(); deviceIterator++)
        {
            /* If the current thread is full, try the next thread (until we run
             * out of threads on this core). */
            P_thread* thisThread = hardwareIterators[0].get_thread();
            if (placer->threadToDevices[thisThread].size()
                >= maxDevicesPerThread)
            {
                hardwareIterators[0].has_core_changed();  /* Reset flag */
                thisThread = hardwareIterators[0].next_thread();

                /* Did we move to the next core? */
                if (hardwareIterators[0].has_core_changed())
                {
                    /* Keep the other iterator moving in lockstep. */
                    hardwareIterators[1].next_core();

                    /* If we have changed cores, and this is the first time,
                     * we've done that for this core pair, we'll just keep
                     * using threads as normal (we're just in the second
                     * core). However, if this is the second time we've moved,
                     * we've migrated to a new core pair, so we have to find a
                     * new empty core pair. */
                    if (coreIncremented)
                    {
                        poke_iterators(&hardwareIterators);
                        thisThread = hardwareIterators[0].get_thread();
                    }

                    else coreIncremented = true;
                }
            }

            /* Well, we've found an empty thread at last! */
            placer->link(thisThread, *deviceIterator);
        }
    }

    return 0;
}

/* Sets two hardware iterators to the next empty core pair, throwing if no such
 * core pair exists. Arguments:
 *
 * - iterators: Two HardwareIterators set exactly one core apart from each
 *   other. Elements of this vector are modified in-place. */
void BucketFilling::poke_iterators(std::vector<HardwareIterator>* iterators)
{
    /* Iterate until an empty core pair is found. Returning (or throwing) are
     * the only (intended) ways out of this loop. */
    bool areCoresEmpty = false;
    while (true)
    {
        /* Check whether or not this core pair is empty. If it is, break (we've
         * done our job).
         *
         * hardwareIt is an iterator over iterators. */
        areCoresEmpty = true;  /* Innocent until proven guilty. */
        std::vector<HardwareIterator>::iterator hardwareIt;
        std::map<AddressComponent, P_thread*>::iterator threadIt;
        std::map<AddressComponent, P_thread*>* threadMap;
        for (hardwareIt = iterators->begin();
             hardwareIt != iterators->end(); hardwareIt++)
        {
            /* Saves typing (and might help the readership). threadMap is a
             * pointer to the threadMap for this core (recall that the above
             * loop disambiguates between the two HardwareIterators). */
            threadMap = &(hardwareIt->get_core()->P_threadm);

            /* Iterating over threads in this core. */
            for (threadIt = threadMap->begin();
                 threadIt != threadMap->end(); threadIt++)

                /* If it's not empty, we exit. */
                if (not placer->threadToDevices[threadIt->second].empty())
                {
                    areCoresEmpty = false;
                    break;
                }

            /* Shortcut. */
            if (not areCoresEmpty) break;
        }

        /* Leave now if the cores are empty. */
        if (areCoresEmpty) return;

        /* If we've gone through the entire structure and no spare core pair
         * has been found, then the engine is already full. */
        if ((*iterators)[1].has_wrapped())
            throw NoSpaceToPlaceException("[ERROR] Engine is full.");

        /* Increment the iterators. */
        for (hardwareIt = iterators->begin(); hardwareIt != iterators->end();
             hardwareIt++) hardwareIt->next_core();
    }
}

/* Stub (I'm lazy) <!> */
void BucketFilling::Dump(FILE*){return;}

#include "SmartRandom.h"

SmartRandom::SmartRandom(Placer* placer):Algorithm(placer){}

/* Places a task onto the engine held by a placer using a random placer.
 *
 * This placer will probably fall over if you have equal numbers of devices of
 * multiple types, because this placer does not reserve cores for certain
 * device types (so it's highly likely to fall into a situation where a certain
 * device cannot be placed, because all of the cores have devices of other
 * types on them).
 *
 * Returns zero. */
float SmartRandom::do_it(P_task* task)
{
    P_device* device;
    P_core* core;
    P_thread* thread;
    P_devtyp* deviceType;
    std::map<P_devtyp*, std::set<P_core*>>::iterator validCoreIt, badCoreIt;

    /* Define the number of devices allowed per thread before constraints are
     * violated - used during selection. */
    devicesPerThreadSoftMax = placer->constrained_max_devices_per_thread(task);

    /* Know where we can put things. */
    placer->define_valid_cores_map(task, &validCoresForDeviceType);

    /* Go over each device in turn */
    WALKPDIGRAPHNODES(unsigned, P_device*, unsigned, P_message*, unsigned,
                      P_pin*, task->pD->G, deviceIterator)
    {
        device = task->pD->G.NodeData(deviceIterator);
        deviceType = device->pP_devtyp;

        /* Ignore if it's a supervisor device (we don't map those). */
        if (!(deviceType->pOnRTS)) continue;

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
            std::map<AddressComponent, P_thread*>::iterator threadIt;
            threadIt = core->P_threadm.begin();
            while (threadIt != core->P_threadm.end())
            {
                if (placer->threadToDevices[threadIt->second].size() <
                    devicesPerThreadSoftMax) break;
                threadIt++;
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

        /* Put the device on the selected thread, and remove the core pair
         * from the available set of cores for other device types. */
        placer->link(thread, device);
        for (badCoreIt = validCoresForDeviceType.begin();
             badCoreIt != validCoresForDeviceType.end(); badCoreIt++)
        {
            if (badCoreIt->first != deviceType)
            {
                badCoreIt->second.erase(core);
                badCoreIt->second.erase(core->pair);
            }
        }
    }

    return 0;
}

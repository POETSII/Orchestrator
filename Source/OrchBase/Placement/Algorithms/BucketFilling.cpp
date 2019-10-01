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
float BucketFilling::do_it(P_task* task, Placer* placer)
{
    /* Start from the first thread in the engine. */
    HardwareIterator hardwareIterator(placer->engine);

    /* Staging area to hold all devices of a given type in a task. */
    vector<P_device*> devicesOfType;

    /* Walk through each device type in the task. */
    std::vector<P_devtyp*>::iterator deviceTypeIterator;
    for (deviceTypeIterator = task->pP_typdcl->P_devtypv.begin();
         deviceTypeIterator != task->pP_typdcl->P_devtypv.end();
         deviceTypeIterator++)
    {
        /* Skip this device type if it is a supervisor device type. */
        if ((*deviceTypeIterator)->pOnRTS) continue;

        /* Complain about running out of space if the iterator has wrapped. */
        if (hardwareIterator.has_wrapped())
        {
            throw NoSpaceToPlaceException("[ERROR] Ran out of space placing a "
                                          "task.");
        }

        /* Walk through each device for this device type. */
        devicesOfType = task->pD->DevicesOfType(*deviceTypeIterator);
        std::vector<P_device*>::iterator deviceIterator;
        for (deviceIterator = devicesOfType.begin();
             deviceIterator != devicesOfType.end(); deviceIterator++)
        {
            /* And here we check the maxdevicesperthread constraint
             * proactively... <!> */
        }
    }

    return 0;
}

/* Stub (I'm lazy) <!> */
void BucketFilling::Dump(FILE*){return;}

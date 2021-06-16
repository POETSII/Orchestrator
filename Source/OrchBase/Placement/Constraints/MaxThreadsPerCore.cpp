#include "MaxThreadsPerCore.h"

MaxThreadsPerCore::MaxThreadsPerCore(
    bool mandatory, float penalty, GraphI_t* gi, unsigned maximum):
    Constraint(category, mandatory, penalty, gi),
    maximum(maximum)
{
    Name(dformat(
        "Maximum threads used per core must be less than or equal to %d.",
        maximum));
}

/* Returns true if there are no cores in the placer that have more than the
 * "maximum" number of threads populated, and false otherwise. */
bool MaxThreadsPerCore::is_satisfied(Placer* placer)
{
    HardwareIterator hardwareIt(placer->engine);
    unsigned threadsUsedInThisCore = 0;

    /* This is a comprehensive examination! */
    while (!hardwareIt.has_wrapped())
    {
        /* Check the current thread for devices */
        if (!placer->threadToDevices[hardwareIt.get_thread()].empty())
        {
            /* There are devices here! If we've populated too many threads in
             * this way, we out. */
            threadsUsedInThisCore += 1;

            if (threadsUsedInThisCore > maximum) return false;
        }

        /* Check next thread. */
        hardwareIt.next_thread();

        /* If we've rolled over to the next core, reset the counter. */
        if (hardwareIt.has_core_changed()) threadsUsedInThisCore = 0;
    }

    return true;
}

/* Returns true if the cores that the devices are placed do not have more
 * than the "maximum" number of threads in use, and false otherwise. */
bool MaxThreadsPerCore::is_satisfied_delta(Placer* placer,
                                             std::vector<DevI_t*> devices)
{
    std::vector<DevI_t*>::iterator deviceIt;
    for (deviceIt = devices.begin(); deviceIt != devices.end(); deviceIt++)
    {
        /* Given one device, get the core that contains it. From that core,
         * check each thread in sequence. */
        P_core* containingCore = placer->deviceToThread[*deviceIt]->parent;
        std::map<AddressComponent, P_thread*>::iterator threadIt;
        unsigned threadsUsedInThisCore = 0;
        for (threadIt = containingCore->P_threadm.begin();
             threadIt != containingCore->P_threadm.end(); threadIt++)
        {
            /* See if anything is placed here. */
            if (!placer->threadToDevices[threadIt->second].empty())
            {
                /* There are devices here! If we've populated too many threads
                 * in this way, we out. */
                threadsUsedInThisCore += 1;
                if (threadsUsedInThisCore > maximum) return false;
            }
        }

        /* If control reaches here, it means we've checked every thread
         * contained in the core containing this device. */
    }

    return true;
}

void MaxThreadsPerCore::Dump(FILE* file)
{
    std::string prefix = dformat("MaxThreadsPerCore Constraint");
    DumpUtils::open_breaker(file, prefix);

    /* Properties */
    if (mandatory) fprintf(file, "This is a hard constraint.\n");
    else fprintf(file,
                 "This is a soft constraint with penalty %f.\n", penalty);

    if (gi == PNULL) fprintf(file, "Graph instance: None\n");
    else fprintf(file, "Graph instance: %s\n", gi->Name().c_str());

    /* Close breaker and flush the dump. */
    DumpUtils::close_breaker(file, prefix);
    fflush(file);
}

#include "MaxDevicesPerThread.h"

MaxDevicesPerThread::MaxDevicesPerThread(
    bool mandatory, float penalty, P_task* task, unsigned maximum):
    Constraint(category, mandatory, penalty, task),
    maximum(maximum)
{
    Name(dformat(
        "Maximum devices per thread must be less than or equal to %d.",
        maximum));
}

/* Returns true if there are no threads in the placer that have more than
 * "maximum" devices, and false otherwise. */
bool MaxDevicesPerThread::is_satisfied(Placer* placer)
{
    std::map<P_thread*, std::list<P_device*>>::iterator it;
    for (it = placer->threadToDevices.begin();
         it != placer->threadToDevices.end(); it++)
    {
        if (it->second.size() > maximum)
        {
            satisfied = false;
            return false;
        }
    }
    satisfied = true;
    return true;  /* Innocent until proven guilty. */
}

/* Returns true if the threads that the devices are placed on do not have more
 * than "maximum" devices, and false otherwise. */
bool MaxDevicesPerThread::is_satisfied_delta(Placer* placer,
                                             std::vector<P_device*> devices)
{
    std::vector<P_device*>::iterator deviceIt;
    for (deviceIt = devices.begin(); deviceIt != devices.end(); deviceIt++)
    {
        /* Given one device, get the thread that contains it, then compare with
         * the number of devices contained by that thread. */
        P_thread* containingThread = placer->deviceToThread[*deviceIt];
        if (placer->threadToDevices[containingThread].size() > maximum)
            return false;
    }
    return true;  /* Innocent until proven guilty. */
}

void MaxDevicesPerThread::Dump(FILE* file)
{
    std::string prefix = dformat("MaxDevicesPerThread Constraint");
    DumpUtils::open_breaker(file, prefix);

    /* Properties */
    if (mandatory) fprintf(file, "This is a hard constraint.\n");
    else fprintf(file,
                 "This is a soft constraint with penalty %f.\n", penalty);

    if (task == PNULL) fprintf(file, "Task: None\n");
    else fprintf(file, "Task: %s\n", task->Name().c_str());

    /* Close breaker and flush the dump. */
    DumpUtils::close_breaker(file, prefix);
    fflush(file);
}

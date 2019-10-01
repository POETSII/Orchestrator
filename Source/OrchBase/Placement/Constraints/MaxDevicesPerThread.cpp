#include "MaxDevicesPerThread.h"

MaxDevicesPerThread::MaxDevicesPerThread(
    bool mandatory, float penalty, P_task* task, unsigned maximum):
    Constraint(category, mandatory, penalty, task),
    maximum(maximum)
{}

/* Stubs (I'm lazy) <!> */
bool MaxDevicesPerThread::is_satisfied(Placer*){return false;}
bool MaxDevicesPerThread::is_satisfied_delta(Placer*,
                                             std::vector<P_device*> devices)
{return false;}
void MaxDevicesPerThread::Dump(FILE*){return;}

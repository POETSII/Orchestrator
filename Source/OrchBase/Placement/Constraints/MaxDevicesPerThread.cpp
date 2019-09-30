#include "MaxDevicesPerThread.h"

MaxDevicesPerThread::MaxDevicesPerThread(
    float penalty, P_task* task, bool mandatory, unsigned maximum):
    maximum(maximum)
{}

/* Stubs (I'm lazy) <!> */
bool MaxDevicesPerThread::is_satisfied(Placer*){return false;}
bool MaxDevicesPerThread::is_satisfied_delta(Placer*,
                                             std::vector<P_device*> devices)
{return false;}
void MaxDevicesPerThread::Dump(FILE*){return;}

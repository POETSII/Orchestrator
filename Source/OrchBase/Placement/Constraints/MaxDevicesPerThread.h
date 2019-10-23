#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_CONSTRAINTS_MAXDEVICESPERTHREAD_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_CONSTRAINTS_MAXDEVICESPERTHREAD_H

/* Defines the logic that constraints the maximum number of devices allowed on
 * any thread in a Placer. */

#include <list>
#include <map>

#include "Constraint.h"
#include "HardwareModel.h"

#define MAX_DEVICES_PER_THREAD_DEFAULT 1024

class MaxDevicesPerThread: public Constraint
{
public:
    MaxDevicesPerThread(bool mandatory, float penalty, P_task* task,
                        unsigned maximum);
    static const constraintCategory category = maxDevicesPerThread;
    unsigned maximum;

    bool is_satisfied(Placer*);
    bool is_satisfied_delta(Placer* placer, std::vector<P_device*> devices);

    void Dump(FILE* = stdout);
};

#endif

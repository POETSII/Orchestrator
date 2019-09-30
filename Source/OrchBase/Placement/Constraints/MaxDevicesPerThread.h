#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_CONSTRAINTS_MAXDEVICESPERTHREAD_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_CONSTRAINTS_MAXDEVICESPERTHREAD_H

/* Defines the logic that constraints the maximum number of devices allowed on
 * any thread in a Placer. */

#include "Constraint.h"

class MaxDevicesPerThread: public Constraint
{
public:
    MaxDevicesPerThread(float penalty, P_task* task, bool mandatory,
                        unsigned maximum);
    static const constraintCategory category = maxDevicesPerThread;
    unsigned maximum;

    bool is_satisfied(Placer*);
    bool is_satisfied_delta(Placer* placer, std::vector<P_device*> devices);
};

#endif

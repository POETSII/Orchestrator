#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_CONSTRAINTS_MAXTHREADSPERCORE_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_CONSTRAINTS_MAXTHREADSPERCORE_H

/* Defines the logic that constraints the maximum number of threads used on
 * each core in a Placer. */

#include <list>
#include <map>

#include "Constraint.h"
#include "HardwareModel.h"


class MaxThreadsPerCore: public Constraint
{
public:
    MaxThreadsPerCore(bool mandatory, float penalty, P_task* task,
                      unsigned maximum);
    static const constraintCategory category = maxThreadsPerCore;
    unsigned maximum;

    bool is_satisfied(Placer*);
    bool is_satisfied_delta(Placer* placer, std::vector<P_device*> devices);

    void Dump(FILE* = stdout);
};

#endif

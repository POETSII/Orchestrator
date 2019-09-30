#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_CONSTRAINT_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_CONSTRAINT_H

/* Describes a constraint imposed on the placement system, at either the task
 * level or the engine level.
 *
 * See the placement documentation for further information. */

#include "ConstraintCategory.h"

class P_device;
class Placer;

class Constraint
{
public:
    Constraint(float penalty, P_task* task, bool mandatory):
        penalty(penalty),
        task(task)
        mandatory(mandatory)
    {}

    virtual const constraintCategory category;
    bool mandatory;
    float penalty;
    P_task* task;

    virtual void Dump(FILE* = stdout);

    /* Satisfaction checking, and satisfaction state. */
    bool satisfied;  /* For delta computation. */
    virtual bool is_satisfied(Placer* placer);
    virtual bool is_satisfied_delta(Placer* placer,
                                    std::vector<P_device*> devices);
};

#endif

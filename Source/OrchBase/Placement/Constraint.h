#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_CONSTRAINT_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_CONSTRAINT_H

/* Describes a constraint imposed on the placement system, at either the task
 * level or the engine level.
 *
 * See the placement documentation for further information. */

#include "ConstraintCategory.h"
#include "DumpUtils.h"
#include "NameBase.h"

#include <vector>

class P_device;
class P_task;
class Placer;

class Constraint: public NameBase
{
public:
    Constraint(constraintCategory category, bool mandatory, float penalty,
               P_task* task):
        category(category),
        mandatory(mandatory),
        penalty(penalty),
        task(task)
    {}
    virtual ~Constraint() = default;

    const constraintCategory category;
    bool mandatory;
    float penalty;
    P_task* task;

    virtual void Dump(FILE* = stdout) = 0;

    /* Satisfaction checking, and satisfaction state. */
    bool satisfied;  /* For delta computation. */
    virtual bool is_satisfied(Placer* placer) = 0;
    virtual bool is_satisfied_delta(Placer* placer,
                                    std::vector<P_device*> devices) = 0;
};

#endif

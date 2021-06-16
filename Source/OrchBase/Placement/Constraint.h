#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_CONSTRAINT_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_CONSTRAINT_H

/* Describes a constraint imposed on the placement system, at either the
 * application graph instance level, or the engine level.
 *
 * See the placement documentation for further information. */

#include "ConstraintCategory.h"
#include "DumpUtils.h"
#include "NameBase.h"

#include <vector>

class DevI_t;
class GraphI_t;
class Placer;

class Constraint: public NameBase, public DumpChan
{
public:
    Constraint(constraintCategory category, bool mandatory, float penalty,
               GraphI_t* gi):
        category(category),
        mandatory(mandatory),
        penalty(penalty),
        gi(gi)
    {}
    virtual ~Constraint(){}

    const constraintCategory category;
    bool mandatory;
    float penalty;
    GraphI_t* gi;

    virtual void Dump(FILE* = stdout) = 0;

    /* Satisfaction checking, and satisfaction state. */
    bool satisfied;  /* For delta computation. */
    virtual bool is_satisfied(Placer* placer) = 0;
    virtual bool is_satisfied_delta(Placer* placer,
                                    std::vector<DevI_t*> devices) = 0;
};

#endif

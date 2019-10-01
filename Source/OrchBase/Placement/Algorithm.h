#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHM_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHM_H

/* Describes an algorithm that will map the devices in an application to
 * threads on the hardware model.
 *
 * See the placement documentation for further information. */

#include "Result.h"

class P_engine;
class P_task;
class Placer;

class Algorithm
{
public:
    virtual ~Algorithm() = default;
    Result result;
    virtual float do_it(P_engine*, P_task*, Placer*) = 0;
    virtual void Dump(FILE* = stdout) = 0;
};

#endif

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
    Algorithm(Placer* placer):placer(placer){}
    virtual ~Algorithm() = default;
    Placer* placer;
    Result result;
    virtual float do_it(P_task*) = 0;
    virtual void Dump(FILE* = stdout) = 0;
};

#endif

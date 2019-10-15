#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHM_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHM_H

/* Describes an algorithm that will map the devices in an application to
 * threads on the hardware model.
 *
 * See the placement documentation for further information. */

#include <algorithm>  /* How ironic (used for std::max). */
#include <set>

class P_core;
class P_engine;
class P_task;
class Placer;

#include "Placer.h"
#include "Result.h"

class Algorithm
{
public:
    Algorithm(Placer* placer):placer(placer){}
    virtual ~Algorithm() = default;
    virtual float do_it(P_task*) = 0;
    void populate_result_structures(P_task* task, float score);

    Placer* placer;
    Result result;
};

#endif

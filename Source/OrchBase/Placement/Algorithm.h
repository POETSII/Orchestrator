#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHM_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHM_H

/* Describes an algorithm that will map the devices in an application to
 * threads on the hardware model.
 *
 * See the placement documentation for further information. */

#include <algorithm>  /* How ironic (used for std::max). */
#include <set>

class GraphI_t;
class Placer;

#include "Result.h"
#include "UniqueDevT.h"

class Algorithm
{
public:
    Algorithm(Placer* placer):placer(placer){}
    virtual ~Algorithm(){}
    virtual float do_it(GraphI_t*) = 0;

    Placer* placer;
    Result result;
};

#endif

#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_RESULT_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_RESULT_H

/* Defines a structure for holding the results of running a placement
 * algorithm.
 *
 * See the placement documentation for further information. */

#include <string>

struct Result
{
    unsigned maxDevicesPerThread;
    float maxEdgeCost;
    float score;
    std::string when;
};

#endif

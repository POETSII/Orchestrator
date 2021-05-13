#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_RESULT_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_RESULT_H

/* Defines a structure for holding the results of running a placement
 * algorithm.
 *
 * See the placement documentation for further information. */

#include <map>
#include <string>

struct Result
{
    std::map<std::string, std::string> args;
    unsigned maxDevicesPerThread;
    float maxEdgeCost;
    std::string method;
    float score;
    std::string startTime;
    std::string endTime;
};

#endif

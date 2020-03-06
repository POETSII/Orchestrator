#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_COREINFO_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_COREINFO_H

/* Describes the loading/unloading state of a core under the care of this
 * Mothership. See the Mothership documentation for further information on
 * core loading and unloading. */

#include <string>

#include "OSFixes.hpp"

struct CoreInfo
{
    std::string codePath;
    std::string dataPath;
    uint8_t numThreadsExpected;
    uint8_t numThreadsCurrent;
};

#endif

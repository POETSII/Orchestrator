#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_NOTASKTODUMP_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_NOTASKTODUMP_H

/* Describes an exception that is to be thrown when a task that has not been
 * placed is dumped. */

#include "OrchestratorException.h"

class NoTaskToDump: public OrchestratorException
{
public:
    NoTaskToDump(std::string message):OrchestratorException(message){};
};

#endif

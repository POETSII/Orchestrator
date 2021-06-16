#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_NOTASKTODUMP_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_NOTASKTODUMP_H

/* Describes an exception that is to be thrown when a gi that has not been
 * placed is dumped. */

#include "OrchestratorException.h"

class NoGIToDump: public OrchestratorException
{
public:
    NoGIToDump(std::string message):OrchestratorException(message){};
};

#endif

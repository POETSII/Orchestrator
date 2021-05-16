#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_NOENGINEEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_NOENGINEEXCEPTION_H

/* Describes an exception that is to be thrown when a gi that has been placed
 * is requested to be placed again. */

#include "OrchestratorException.h"

class NoEngineException: public OrchestratorException
{
public:
    NoEngineException(std::string message):OrchestratorException(message){};
};

#endif

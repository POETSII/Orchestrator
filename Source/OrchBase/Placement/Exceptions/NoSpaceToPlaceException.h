#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_NOSPACETOPLACEEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_NOSPACETOPLACEEXCEPTION_H

/* Describes an exception that is to be thrown when a task that has been placed
 * is requested to be placed again. */

#include "OrchestratorException.h"

class NoSpaceToPlaceException: public OrchestratorException
{
public:
    NoSpaceToPlaceException(std::string message):
        OrchestratorException(message){};
};

#endif

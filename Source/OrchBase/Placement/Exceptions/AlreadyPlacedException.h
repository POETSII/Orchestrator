#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_ALREADYPLACEDEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_ALREADYPLACEDEXCEPTION_H

/* Describes an exception that is to be thrown when a task that has been placed
 * is requested to be placed again. */

#include "OrchestratorException.h"

class AlreadyPlacedException: public OrchestratorException
{
public:
    AlreadyPlacedException(std::string message):
        OrchestratorException(message){};
};

#endif

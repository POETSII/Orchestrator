#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_BADINTEGRITYEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_BADINTEGRITYEXCEPTION_H

/* Describes an exception that is to be thrown after an algorithm is used to
 * place a task, if that task still has devices that are not placed. */

#include "OrchestratorException.h"

class BadIntegrityException: public OrchestratorException
{
public:
    BadIntegrityException(std::string message):
        OrchestratorException(message){};
};

#endif

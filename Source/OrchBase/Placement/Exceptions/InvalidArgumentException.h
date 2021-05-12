#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_INVALIDARGUMENTEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_INVALIDARGUMENTEXCEPTION_H

/* Describes an exception that is to be thrown if either:
 * - the argument name is invalid, or
 * - the argument name is valid but the value is invalid, or
 * - an algorithm has been given an argument that doesn't make sense. */

#include "OrchestratorException.h"

class InvalidArgumentException: public OrchestratorException
{
public:
    InvalidArgumentException(std::string message):
        OrchestratorException(message){};
};

#endif

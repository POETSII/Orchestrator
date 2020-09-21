#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_INVALIDALGORITHMDESCRIPTOREXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_INVALIDALGORITHMDESCRIPTOREXCEPTION_H

/* Describes an exception that is to be thrown when an algorithm is to be used,
 * but the algorithm is not defined in the source anywhere. */

#include "OrchestratorException.h"

class InvalidAlgorithmDescriptorException: public OrchestratorException
{
public:
    InvalidAlgorithmDescriptorException(std::string message):
        OrchestratorException(message){};
};

#endif

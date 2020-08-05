#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_ITERATOREXCEPTION_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_ITERATOREXCEPTION_H

/* Describes an exception that is to be thrown when a hardware iterator is
 * passed an engine that is not populated. */

#include "OrchestratorException.h"

class IteratorException: public OrchestratorException
{
public:
    IteratorException(std::string message):OrchestratorException(message){};
};

#endif

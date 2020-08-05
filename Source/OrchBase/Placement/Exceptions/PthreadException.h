#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_PTHREADEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_PTHREADEXCEPTION_H

/* Describes an exception that is to be thrown when pthread_create fails. */

#include "OrchestratorException.h"

class PthreadException: public OrchestratorException
{
public:
    PthreadException(std::string message):OrchestratorException(message){};
};

#endif

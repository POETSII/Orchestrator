#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_THREADEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_THREADEXCEPTION_H

/* Describes an exception that is to be thrown when a pthread operation is
 * attempted, but fails for some reason. */

#include "OrchestratorException.h"

class ThreadException: public OrchestratorException
{
public:
    ThreadException(std::string message):OrchestratorException(message){};
};

#endif

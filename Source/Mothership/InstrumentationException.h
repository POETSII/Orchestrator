#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_INSTRUMENTATIONEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_INSTRUMENTATIONEXCEPTION_H

/* Describes an exception that is to be thrown when the instrumentation has a
 * problem. */

#include "OrchestratorException.h"

class InstrumentationException: public OrchestratorException
{
public:
    InstrumentationException(std::string message):
        OrchestratorException(message){};
};

#endif

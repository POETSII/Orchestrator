#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_COSTCACHEEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_COSTCACHEEXCEPTION_H

/* Describes an exception that is to be thrown when a cost cache could not be
 * generated for an engine, due to violated assumptions. */

#include "OrchestratorException.h"

class CostCacheException: public OrchestratorException
{
public:
    CostCacheException(std::string message):OrchestratorException(message){};
};

#endif

#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_INVALIDENGINEEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_INVALIDENGINEEXCEPTION_H

/* Describes an exception that is to be thrown when the hardware file parser is
 * ordered to populate a deployer before it has loaded a file. */

#include "OrchestratorException.h"

class InvalidEngineException: public OrchestratorException
{
public:
    InvalidEngineException():OrchestratorException{""}
    {message = "[ERROR] The engine is already populated. Clear it first "
               "attempting to populate it with another topology.";}
};

#endif

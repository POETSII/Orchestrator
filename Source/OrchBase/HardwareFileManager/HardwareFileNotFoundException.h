#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_HARDWAREFILENOTFOUNDEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_HARDWAREFILENOTFOUNDEXCEPTION_H

/* Describes an exception that is to be thrown when a path is passed to the
 * hardware file parser, which does not correspond to a file. */

#include "OrchestratorException.h"

class HardwareFileNotFoundException: public OrchestratorException
{
public:
    HardwareFileNotFoundException(std::string message):
        OrchestratorException(message){};
};

#endif

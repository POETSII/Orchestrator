#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_HARDWARESEMANTICEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_HARDWARESEMANTICEXCEPTION_H

/* Describes an exception that is to be thrown when a hardware file is passed
 * to the hardware file parser, and when the file contains a semantic error. */

#include "OrchestratorException.h"

class HardwareSemanticException: public OrchestratorException
{
public:
    HardwareSemanticException(std::string message):
        OrchestratorException(message){};
};

#endif

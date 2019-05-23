#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_HARDWAREFILEPARSEREXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_HARDWAREFILEPARSEREXCEPTION_H

/* Describes an exception that is to be thrown from the parser (intended as a
 * base class). */

#include "OrchestratorException.h"
class HardwareFileParserException: public OrchestratorException
{
public:
    HardwareFileParserException(std::string message):
        OrchestratorException(message){};
};

#endif

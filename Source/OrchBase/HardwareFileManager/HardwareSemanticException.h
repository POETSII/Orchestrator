#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_HARDWARESEMANTICEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_HARDWARESEMANTICEXCEPTION_H

/* Describes an exception that is to be thrown when a hardware file is passed
 * to the hardware file parser, and when the file contains a semantic error. */

#include "HardwareFileParserException.h"

class HardwareSemanticException: public HardwareFileParserException
{
public:
    HardwareSemanticException(std::string message):
        HardwareFileParserException(message){};
};

#endif

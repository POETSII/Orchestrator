#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_HARDWARESYNTAXEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_HARDWARESYNTAXEXCEPTION_H

/* Describes an exception that is to be thrown when a hardware file is passed
 * to the hardware file parser, and when the file contains a syntax error. */

#include "HardwareFileParserException.h"

class HardwareSyntaxException: public HardwareFileParserException
{
public:
    HardwareSyntaxException(std::string message):
        HardwareFileParserException(message){};
};

#endif

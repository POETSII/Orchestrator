#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_HARDWAREFILENOTFOUNDEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_HARDWAREFILENOTFOUNDEXCEPTION_H

/* Describes an exception that is to be thrown when a path is passed to the
 * hardware file parser, which does not correspond to a file. */

#include "HardwareFileParserException.h"

class HardwareFileNotFoundException: public HardwareFileParserException
{
public:
    HardwareFileNotFoundException(std::string message):
        HardwareFileParserException(message){};
};

#endif

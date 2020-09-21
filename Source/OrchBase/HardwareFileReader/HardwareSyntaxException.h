#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEREADER_HARDWARESYNTAXEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEREADER_HARDWARESYNTAXEXCEPTION_H

/* Describes an exception that is to be thrown when a hardware file is passed
 * to the hardware file reader, and when the file contains a syntax error. */

#include "HardwareFileReaderException.h"

class HardwareSyntaxException: public HardwareFileReaderException
{
public:
    HardwareSyntaxException(std::string message):
        HardwareFileReaderException(message){};
};

#endif

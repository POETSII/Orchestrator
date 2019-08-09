#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEREADER_HARDWARESEMANTICEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEREADER_HARDWARESEMANTICEXCEPTION_H

/* Describes an exception that is to be thrown when a hardware file is passed
 * to the hardware file reader, and when the file contains a semantic error. */

#include "HardwareFileReaderException.h"

class HardwareSemanticException: public HardwareFileReaderException
{
public:
    HardwareSemanticException(std::string message):
        HardwareFileReaderException(message){};
};

#endif

#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEREADER_HARDWAREFILENOTFOUNDEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEREADER_HARDWAREFILENOTFOUNDEXCEPTION_H

/* Describes an exception that is to be thrown when a path is passed to the
 * hardware file reader, which does not correspond to a file. */

#include "HardwareFileReaderException.h"

class HardwareFileNotFoundException: public HardwareFileReaderException
{
public:
    HardwareFileNotFoundException(std::string message):
        HardwareFileReaderException(message){};
};

#endif

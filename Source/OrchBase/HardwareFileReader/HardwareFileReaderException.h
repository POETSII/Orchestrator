#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEREADER_HARDWAREFILEREADEREXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEREADER_HARDWAREFILEREADEREXCEPTION_H

/* Describes an exception that is to be thrown from the reader (intended as a
 * base class). */

#include "OrchestratorException.h"
class HardwareFileReaderException: public OrchestratorException
{
public:
    HardwareFileReaderException(std::string message):
        OrchestratorException(message){};
};

#endif

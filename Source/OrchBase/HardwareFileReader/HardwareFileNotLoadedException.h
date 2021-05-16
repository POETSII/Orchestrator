#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEREADER_HARDWAREFILENOTLOADEDEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEREADER_HARDWAREFILENOTLOADEDEXCEPTION_H

/* Describes an exception that is to be thrown when the hardware file reader is
 * ordered to populate a deployer before it has loaded a file. */

#include "HardwareFileReaderException.h"

class HardwareFileNotLoadedException: public HardwareFileReaderException
{
public:
    HardwareFileNotLoadedException():HardwareFileReaderException(
        "[ERROR] Engine-population attempted without first loading a file.\n")
        {};
};

#endif

#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_HARDWAREFILENOTLOADEDEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_HARDWAREFILENOTLOADEDEXCEPTION_H

/* Describes an exception that is to be thrown when the hardware file parser is
 * ordered to populate a deployer before it has loaded a file. */

#include "HardwareFileParserException.h"

class HardwareFileNotLoadedException: public HardwareFileParserException
{
public:
    HardwareFileNotLoadedException():HardwareFileParserException{""}
    {message = "[ERROR] Engine-population attempted without first "
               "loading a file.\n";}
};

#endif

#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_INVALIDADDRESSEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_INVALIDADDRESSEXCEPTION_H

/* Describes an exception that is to be thrown when a component of a hardware
 * address is set that is invalid with respect to it's format. */

#include "OrchestratorException.h"

class InvalidAddressException: public OrchestratorException
{
public:
    InvalidAddressException(std::string message)
        :OrchestratorException(message){};
};

#endif

#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_MISSINGADDRESSEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_MISSINGADDRESSEXCEPTION_H

/* Describes an exception that is to be thrown when an item's hardware address
 * is requested when it doesn't have one. */

#include "OrchestratorException.h"

class MissingAddressException: public OrchestratorException
{
public:
    MissingAddressException(std::string message)
        :OrchestratorException(message){};
};

#endif

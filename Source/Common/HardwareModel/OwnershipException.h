#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_OWNERSHIPEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_OWNERSHIPEXCEPTION_H

/* Describes an exception that is to be thrown when ownership is claimed of an
 * item in the hardware stack that is already owned. */

#include "OrchestratorException.h"

class OwnershipException: public OrchestratorException
{
public:
    OwnershipException(std::string message):OrchestratorException(message){};
};

#endif

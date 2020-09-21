#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_FILEOPENEXCEPTION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_EXCEPTIONS_FILEOPENEXCEPTION_H

/* Describes an exception that is to be thrown when a file fails to be
 * opened. */

#include "OrchestratorException.h"

class FileOpenException: public OrchestratorException
{
public:
    FileOpenException(std::string message):OrchestratorException(message){};
};

#endif

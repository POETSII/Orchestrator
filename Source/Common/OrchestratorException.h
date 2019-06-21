#ifndef __ORCHESTRATOR_SOURCE_COMMON_ORCHESTRATOREXCEPTION_H
#define __ORCHESTRATOR_SOURCE_COMMON_ORCHESTRATOREXCEPTION_H

/* Describes a base exception class that can be thrown by various Orchestrator
 * components. */

#include <exception>
#include <string>

class OrchestratorException: public std::exception
{
public:
    OrchestratorException(std::string message)
        :std::exception(),message(message){}
    virtual ~OrchestratorException() throw(){}
    std::string message;
    const char* what() const throw() {return message.c_str();}
};

#endif

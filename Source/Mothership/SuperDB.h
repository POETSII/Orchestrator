#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_SUPERDB_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_SUPERDB_H

/* Describes the supervisor container used by the Mothership to manage
 * supervisors running under applications. */

#include <map>
#include <fstream>
#include <string>

#include "dfprintf.h"
#include "SuperHolder.h"

/* I'm lazy. */
typedef std::map<std::string, SuperHolder*>::iterator SuperIt;

/* Convenience for finding supervisors in handlers. */
#define FIND_SUPERVISOR \
    SuperIt superFinder = supervisors.find(appName); \
    if (superFinder == supervisors.end()) return -2;

/* For supervisor handlers that have no arguments */
#define HANDLE_SUPERVISOR_FN_NAME(HANDLER) ##HANDLER_supervisor
#define HANDLE_SUPERVISOR_DECL(HANDLER) \
    int HANDLE_SUPERVISOR_FN_NAME(std::string);
#define HANDLE_SUPERVISOR_FN(HANDLER_NAME) \
int SuperDB::HANDLE_SUPERVISOR_FN_NAME(HANDLER_NAME)(std::string appName) \
{ \
    FIND_SUPERVISOR \
    pthread_mutex_lock(&(superFinder->second->lock), PNULL); \
    int rc = (*(superFinder->second->HANDLER_NAME))(); \
    pthread_mutex_unlock(&(superFinder->second->lock), PNULL); \
    return rc; \
}

/* For supervisor handlers that have message-input and message-output
 * arguments. */
#define HANDLE_SUPERVISOR_CALL_DECL(HANDLER)           \
    int HANDLE_SUPERVISOR_FN_NAME(std::string, PMsg_p*, PMsg_p*);
#define HANDLE_SUPERVISOR_CALL_FN(HANDLER_NAME) \
int SuperDB::HANDLE_SUPERVISOR_FN_NAME(HANDLER_NAME) \
    (std::string appName, PMsg_p* inputMessage, PMsg_p* outputMessage) \
{ \
    FIND_SUPERVISOR \
    pthread_mutex_lock(&(superFinder->second->lock), PNULL); \
    int rc = (*(superFinder->second->HANDLER_NAME))(inputMessage, \
                                                    outputMessage); \
    pthread_mutex_unlock(&(superFinder->second->lock), PNULL); \
    return rc; \
}

class SuperDB
{
public:
    ~SuperDB();
    std::map<std::string, SuperHolder*> supervisors;

    void idle_rotation();
    bool load_supervisor(std::string appName, std::string path,
                         std::string* errorMessage);
    bool reload_supervisor(std::string appName, std::string* errorMessage);
    bool unload_supervisor(std::string appName);
    void dump(std::ofstream*);

    /* Code repetition ahoy! (methods for declaring supervisor handlers) */
#HANDLE_SUPERVISOR_DECL(init)
#HANDLE_SUPERVISOR_DECL(exit)
#HANDLE_SUPERVISOR_DECL(idle)
#HANDLE_SUPERVISOR_CALL_DECL(call)
#HANDLE_SUPERVISOR_CALL_DECL(implicitCall)
};

#endif

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
typedef std::map<std::string, SuperHolder>::iterator SuperIt;

class SuperDB
{
public:
    std::map<std::string, SuperHolder> supervisors;
    bool load_supervisor(std::string appName, std::string path,
                         std::string* errorMessage);
    int call_supervisor(std::string appName, PMsg_p* inputMessage,
                        PMsg_p* outputMessage);
    int initialise_supervisor(std::string appName);
    bool unload_supervisor(std::string appName);
    void dump(std::ofstream*);
};

#endif

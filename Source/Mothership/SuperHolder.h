#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_SUPERHOLDER_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_SUPERHOLDER_H

/* Defines the SuperHolder object, which holds loaded supervisors. */

#include <dlfcn.h>
#include <string>
#include <fstream>

#include "PMsg_p.hpp"

class SuperHolder
{
public:
    SuperHolder(std::string path);
    ~SuperHolder();
    std::string path;
    bool error;
    bool are_all_hooks_loaded();
    void dump(std::ofstream*);
    int (*call)(PMsg_p*, PMsg_p*);
    int (*exit)();
    int (*idle)();
    int (*init)();
    int (*implicitCall)(PMsg_p*, PMsg_p*);

private:
    void* so;
};

#endif

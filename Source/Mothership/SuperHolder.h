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
    void dump(std::ofstream*);
    bool error;
    int (*entryPoint)(PMsg_p*, PMsg_p*);
    int (*initialise)();
private:
    void* so;
};

#endif

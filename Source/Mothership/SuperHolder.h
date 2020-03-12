#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_SUPERHOLDER_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_SUPERHOLDER_H

/* Defines the SuperHolder object, which holds loaded supervisors. */

#include <dlfcn.h>
#include <string>
#include <fstream>

class SuperHolder
{
public:
    SuperHolder(std::string path);
    ~SuperHolder();
    std::string path;
    void* so;
    void dump(std::ofstream*);
};

#endif

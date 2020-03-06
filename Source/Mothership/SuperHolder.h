#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_SUPERHOLDER_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_SUPERHOLDER_H

/* Defines the Superholder object, which holds loaded supervisors. */

#include <dlfcn.h>
#include <string>
#include <ofstream>

class Superholder
{
public:
    Superholder(std::string path);
    ~Superholder();
    std::string path;
    void* so;
    void dump(ofstream*);
};

#endif

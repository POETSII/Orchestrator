#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_SUPERHOLDER_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_SUPERHOLDER_H

/* Defines the SuperHolder object, which holds loaded supervisors. */

#include <dlfcn.h>
#include <pthread.h>
#include <string>
#include <fstream>

#include "PMsg_p.hpp"

#include "poets_pkt.h"

class SuperHolder
{
public:
    SuperHolder(std::string path);
    ~SuperHolder();
    std::string path;
    bool error;
    pthread_mutex_t lock;
    bool are_all_hooks_loaded();
    void dump(std::ofstream*);
    int (*call)(std::vector<P_Pkt_t>&, 
                std::vector<std::pair<uint32_t, P_Pkt_t> >&);
    int (*exit)();
    int (*idle)();
    int (*init)();

private:
    void* so;
};

#endif

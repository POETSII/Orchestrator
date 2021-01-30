#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_SUPERHOLDER_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_SUPERHOLDER_H

/* Defines the SuperHolder object, which holds loaded supervisors. */

#include <dlfcn.h>
#include <pthread.h>
#include <string>
#include <fstream>

#include "PMsg_p.hpp"
#include "SupervisorApi.h"

#include "poets_pkt.h"

class SuperHolder
{
public:
    SuperHolder(std::string path, std::string appName);
    ~SuperHolder();
    std::string path;
    bool error;
    pthread_mutex_t lock;
    SupervisorApi* api;
    bool are_all_hooks_loaded();
    void dump(std::ofstream*);
    int (*call)(std::vector<P_Pkt_t>&, std::vector<P_Addr_Pkt_t>&);
    int (*exit)();
    SupervisorApi* (*getApi)();
    int (*idle)();
    int (*init)();

private:
    void* so;
};

#endif

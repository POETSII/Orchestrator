#include "SuperHolder.h"

/* Here we load the shared objects! It is down to the creator to verify the
 * success of the loading process (i.e. by checking (error), and by calling
 * dlerror to obtain an error message). */
SuperHolder::SuperHolder(std::string path, std::string appName):
    path(path)
{
    pthread_mutex_init(&lock, NULL);
    error = false;
    so = dlopen(path.c_str(), RTLD_NOW);

    /* We aren't loading anything else without so. */
    if (so == NULL)
    {
        error = true;
        return;
    }

    /* Load hooks. */
    call = reinterpret_cast<int (*)(std::vector<P_Pkt_t>&, 
                                    std::vector<P_Addr_Pkt_t>&)>
                                        (dlsym(so, "SupervisorCall"));
    if (call == NULL) error = true;
    exit = reinterpret_cast<int (*)()>(dlsym(so, "SupervisorExit"));
    if (exit == NULL) error = true;
    idle = reinterpret_cast<int (*)()>(dlsym(so, "SupervisorIdle"));
    if (idle == NULL) error = true;
    init = reinterpret_cast<int (*)()>(dlsym(so, "SupervisorInit"));
    if (init == NULL) error = true;
    
    getAddr = reinterpret_cast<uint64_t(*)()>(dlsym(so,"SupervisorIdx2Addr"));
    if (getAddr == NULL) error = true;
    getAddrVector = reinterpret_cast<void(*)()>(dlsym(so,"SupervisorIdx2Name"));
    if (getAddrVector == NULL) error = true;

    /* Bind the method that allows the Mothership to get this Supervisor's API
     * object, if possible (before the Mothership provisions it - we want to
     * ensure it is loadable). */
    getApi = reinterpret_cast<SupervisorApi* (*)()>
        (dlsym(so, "GetSupervisorApi"));
    if (getApi == NULL) error = true;
}

/* And here we close them. */
SuperHolder::~SuperHolder()
{
    dlclose(so);
    pthread_mutex_destroy(&lock);
}

/* Convenience method returning whether or not all of the hooks have been
 * loaded. Even the default supervisor must define all of the hook functions
 * (even if they are empty). */
bool SuperHolder::are_all_hooks_loaded()
{
    return not (call == NULL or
                exit == NULL or
                idle == NULL or
                init == NULL);
}

/* And here we dump! */
void SuperHolder::dump(std::ofstream* stream)
{
    *stream << "Supervisor at \"" << path << "\" is ";
    if (are_all_hooks_loaded()) *stream << "NOT ";
    *stream << "loaded correctly.\n";
}

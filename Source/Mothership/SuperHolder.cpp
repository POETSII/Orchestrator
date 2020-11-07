#include "SuperHolder.h"

/* Here we load the shared objects! It is down to the creator to verify the
 * success of the loading process (i.e. by checking (error), and by calling
 * dlerror to obtain an error message). */
SuperHolder::SuperHolder(std::string path):
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
    call = reinterpret_cast<int (*)(PMsg_p*, PMsg_p*)>
        (dlsym(so, "SupervisorCall"));
    exit = reinterpret_cast<int (*)()>(dlsym(so, "SupervisorExit"));
    idle = reinterpret_cast<int (*)()>(dlsym(so, "SupervisorIdle"));
    implicitCall = reinterpret_cast<int (*)(PMsg_p*, PMsg_p*)>
        (dlsym(so, "SupervisorImplicitCall"));
    init = reinterpret_cast<int (*)()>(dlsym(so, "SupervisorInit"));

    if (not are_all_hooks_loaded()) error = true;
    /* We out, yo */
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
                implicitCall == NULL or
                init == NULL);
}

/* And here we dump! */
void SuperHolder::dump(std::ofstream* stream)
{
    *stream << "Supervisor at \"" << path << "\" is ";
    if (are_all_hooks_loaded()) *stream << "NOT ";
    *stream << "loaded correctly.\n";
}

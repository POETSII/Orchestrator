#include "SuperHolder.h"

/* Here we load the shared objects! It is down to the creator to verify the
 * success of the loading process (i.e. by checking (!so), and by calling
 * dlerror to obtain an error message). */

SuperHolder::SuperHolder(std::string path):
    path(path)
{
    error = false;
    so = dlopen(path.c_str(), RTLD_NOW);

    /* We aren't loading anything else without so. */
    if (so == NULL)
    {
        error = true;
        return;
    }

    /* Load hooks. */
    initialise = reinterpret_cast<int (*)()>(dlsym(so, "SupervisorInit"));
    entryPoint = reinterpret_cast<int (*)(PMsg_p*, PMsg_p*)>
        (dlsym(so, "SupervisorCall"));
    if (initialise == NULL or entryPoint == NULL)
    {
        error = true;
        return;
    }
}

/* And here we close them. */
SuperHolder::~SuperHolder()
{
    dlclose(so);
}

/* And here we dump! */
void SuperHolder::dump(std::ofstream* stream)
{
    *stream << "Supervisor at \"" << path << "\" is ";
    if (so == NULL or entryPoint == NULL or initialise == NULL)
        *stream << "NOT ";
    *stream << "loaded correctly.\n";
}

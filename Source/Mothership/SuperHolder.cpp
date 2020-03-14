#include "SuperHolder.h"

/* Here we load the shared objects! It is down to the creator to verify the
 * success of the loading process (i.e. by checking (!so), and by calling
 * dlerror to obtain an error message). */

SuperHolder::SuperHolder(std::string path):
    path(path)
{
    so = dlopen(path.c_str(), RTLD_NOW);
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
    if (so == NULL) *stream << "NOT ";
    *stream << "loaded correctly.\n";
}

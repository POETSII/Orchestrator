#include "SuperDB.h"

/* Cleanup. */
SuperDB::~SuperDB()
{
    /* Note we don't lock the deletion of supervisors here, because if we're
     * closing down, we won't get stuck. */
    for (SuperIt superIt = supervisors.begin(); superIt != supervisors.end();
         superIt++) delete superIt->second;
    supervisors.clear();
}

/* Calls the idle handlers for each loaded supervisor in turn, if that
 * supervisor is not busy doing something else. */
void SuperDB::idle_rotation()
{
    for (SuperIt superIt = supervisors.begin(); superIt != supervisors.end();
         superIt++)
    {
        /* Ignore if it's locked. */
        if (pthread_mutex_trylock(&(superIt->second->lock)) != 0) continue;

        /* Call idle method for this supervisor. */
        idle_supervisor(superIt->first);
    }
}

/* Loads a supervisor into the database, returning true on success and false on
 * failure. If there is a failure, errorMessage is written with the contents of
 * the error. */
bool SuperDB::load_supervisor(std::string appName, std::string path,
                              std::string* errorMessage)
{
    /* Firstly, check whether or not a supervisor has already been loaded for
     * this application. */
    SuperIt superIt = supervisors.find(appName);
    if (superIt != supervisors.end())
    {
        *errorMessage = dformat(
            "A supervisor has already been loaded to application '%s'.",
            appName.c_str());
        return false;
    }

    /* Otherwise, load up. */
    supervisors[appName] = new SuperHolder(path);

    /* Check for errors as per the specification... */
    if (supervisors.find(appName)->second->error)
    {
        *errorMessage = dlerror();
        return false;
    }

    return true;
}

/* Unloads a supervisor from the database, returning true on success and false
 * if no such supervisor exists. */
bool SuperDB::unload_supervisor(std::string appName)
{
    /* Check whether or not a supervisor has already been loaded for this
     * application. */
    SuperIt superIt = supervisors.find(appName);
    if (superIt != supervisors.end())
    {
        return false;
    }

    /* Otherwise, unload away (via destructor), but let it finish what it's
     * doing. */
    pthread_mutex_lock(&(superFinder->second->lock), PNULL);
    SuperHolder* toDestroy = superIt->second;
    supervisors.erase(superIt); /* If it can't be found, it can't be locked */
    pthread_mutex_unlock(&(superFinder->second->lock), PNULL);
    delete toDestroy;
    return true;
}

/* Code repetition ahoy! (methods for calling supervisor handlers) */
#HANDLE_SUPERVISOR_FN(init)
#HANDLE_SUPERVISOR_FN(exit)
#HANDLE_SUPERVISOR_FN(idle)
#HANDLE_SUPERVISOR_CALL_FN(call)
#HANDLE_SUPERVISOR_CALL_FN(implicitCall)

/* Prints a diagnostic line for each supervisor. The argument is the stream to
 * dump to. */
void SuperDB::dump(std::ofstream* stream)
{
    *stream << "SuperDB dump:\n";
    if (supervisors.empty())
    {
        *stream << "No supervisor devices defined.\n";
    }
    for (SuperIt superIt = supervisors.begin(); superIt != supervisors.end();
         superIt++)
    {
        superIt->second->dump(stream);
    }
}

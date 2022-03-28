#include "SuperDB.h"

SuperDB::SuperDB()
{
    pthread_mutex_init(&mapLock, NULL);
    nextIdle = supervisors.begin();
}

/* Cleanup. */
SuperDB::~SuperDB()
{
    /* Note we don't lock the deletion of supervisors here, because if we're
     * closing down, we won't get stuck. */
    for (SuperIt superIt = supervisors.begin(); superIt != supervisors.end();
         superIt++) delete superIt->second;
    supervisors.clear();
    nextIdle = supervisors.begin();
    pthread_mutex_destroy(&mapLock);
}

/* Gets the next supervisor in the idle rotation. Returns PNULL if there is no
 * supervisor to grab. The caller needs to ensure that the supervisor is not
 * locked, and that the application is running.*/
SuperHolder* SuperDB::get_next_idle(std::string& name)
{
    pthread_mutex_lock(&mapLock);
    SuperHolder* out;

    /* Protect against having no defined supervisors. */
    if (supervisors.empty())
    {
        pthread_mutex_unlock(&mapLock);
        return PNULL;
    }

    /* Get next supervisor, ignoring supervisors with errors or supervisors
     * that have been freed. */
    while (true)
    {
        if (nextIdle == supervisors.end()) nextIdle = supervisors.begin();
        else
        {
            nextIdle++;
            if (nextIdle == supervisors.end()) nextIdle = supervisors.begin();
        }

        /* Check the entry. */
        if (nextIdle->second != PNULL)
            if (!nextIdle->second->error)
                break;
    }

    name = nextIdle->first;
    out = nextIdle->second;
    pthread_mutex_unlock(&mapLock);
    return out;
}

/* Retrieves a reference to the api-communications object used by the
 * supervisor shared object, so it can be populated or modified. Note the lack
 * of locking. */
SupervisorApi* SuperDB::get_supervisor_api(std::string appName)
{
    SuperIt superFinder = supervisors.find(appName); \
    if (superFinder == supervisors.end()) return PNULL;
    return (*(superFinder->second->getApi))();
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
    pthread_mutex_lock(&mapLock);
    supervisors[appName] = new SuperHolder(path, appName);
    nextIdle = supervisors.begin();
    pthread_mutex_unlock(&mapLock);

    /* Check for errors as per the specification... */
    if (supervisors.find(appName)->second->error)
    {
        char* tmpErr = dlerror();
        if (tmpErr != NULL) *errorMessage = tmpErr;
        else *errorMessage = "libdl wrote no error message";
        return false;
    }

    return true;
}

/* Unloads a supervisor, then loads it again. Propagates the output of
 * load_supervisor, but fails fast if the unload fails. */
bool SuperDB::reload_supervisor(std::string appName, std::string* errorMessage)
{
    /* Before unloading the supervisor, get the path to the binary. */
    std::string binPath = supervisors.find(appName)->second->path;
    if (not unload_supervisor(appName)) return false;
    return load_supervisor(appName, binPath, errorMessage);
}

/* Unloads a supervisor from the database, returning true on success and false
 * if no such supervisor exists. */
bool SuperDB::unload_supervisor(std::string appName)
{
    /* Check whether or not a supervisor has already been loaded for this
     * application. */
    SuperIt superIt = supervisors.find(appName);
    if (superIt == supervisors.end()) return false;

    /* Otherwise, unload away (via destructor), but let it finish what it's
     * doing. */
    pthread_mutex_lock(&mapLock);
    pthread_mutex_t* superLock = &(superIt->second->lock);
    pthread_mutex_lock(superLock);
    SuperHolder* toDestroy = superIt->second;
    supervisors.erase(superIt); /* If it can't be found, it can't be locked */
    nextIdle = supervisors.begin();
    pthread_mutex_unlock(superLock);
    delete toDestroy;
    pthread_mutex_unlock(&mapLock);
    return true;
}

/* Code repetition ahoy! (methods for calling supervisor handlers) */
HANDLE_SUPERVISOR_FN(init)
HANDLE_SUPERVISOR_FN(exit)

int SuperDB::call_supervisor(std::string appName,
                    std::vector<P_Pkt_t>& inputPackets,
                    std::vector<P_Addr_Pkt_t>& outputPackets)
{
    FIND_SUPERVISOR;
    pthread_mutex_lock(&(superFinder->second->lock));
    int rc = (*(superFinder->second->call))(inputPackets, outputPackets);
    pthread_mutex_unlock(&(superFinder->second->lock));
    return rc;
}

/* Gets the SupervisorDeviceInstance_t for the dpecified device index*/
int SuperDB::get_device_instance(std::string appName, uint32_t index,
                                    const SupervisorDeviceInstance_t*& instance)
{
    int rc = 0;
    FIND_SUPERVISOR;
    pthread_mutex_lock(&(superFinder->second->lock));
    instance = (*(superFinder->second->getInstance))(index);
    pthread_mutex_unlock(&(superFinder->second->lock));
    return rc;
}

/* Calls the OnIdle handler for a supervisor with a given application
 * name. This handler is treated differently, as the caller is expected to
 * acquire the supervisor lock (see `idle_rotation`). */
int SuperDB::idle_supervisor(std::string appName)
{
    FIND_SUPERVISOR;
    int rc = (*(superFinder->second->idle))();
    return rc;
}

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

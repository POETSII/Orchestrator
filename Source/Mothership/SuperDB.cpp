#include "SuperDB.h"

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
    supervisors.insert(std::pair<std::string, SuperHolder>
                       (appName, SuperHolder(path)));

    /* Check for errors as per the specification... */
    if (supervisors.find(appName)->second.error)
    {
        *errorMessage = dlerror();
        return false;
    }

    return true;
}

/* Calls the supervisor for a given application using its entry point. */
int SuperDB::call_supervisor(std::string appName, PMsg_p* inputMessage,
                             PMsg_p* outputMessage)
{
    std::map<std::string, SuperHolder>::iterator superFinder;
    superFinder = supervisors.find(appName);
    if (superFinder == supervisors.end()) return -2;
    /* I know this is hideous, but that's function pointers for you. */
    return (*(superFinder->second.entryPoint))(inputMessage, outputMessage);
}

/* Initialises the supervisor for a given application. */
int SuperDB::initialise_supervisor(std::string appName)
{
    std::map<std::string, SuperHolder>::iterator superFinder;
    superFinder = supervisors.find(appName);
    if (superFinder == supervisors.end()) return -2;
    /* I know this is hideous, but that's function pointers for you. */
    return (*(superFinder->second.initialise))();
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

    /* Otherwise, unload away (via destructor). */
    supervisors.erase(superIt);
    return true;
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
        superIt->second.dump(stream);
    }
}

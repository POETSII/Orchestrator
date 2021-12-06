#include "AppDB.h"

/* Checks appInfos for an application of a given name. If it doesn't exist,
 * AppDB creates it and returns a pointer to it (passing distCountExpected and
 * soloApp to it as an argument). If it already exists, returns a pointer to
 * the existing application, does not use distCountExpected, but sets
 * soloApp. */
AppInfo* AppDB::check_create_app(std::string name, uint32_t distCountExpected,
                                 bool soloApp)
{
    AppInfoIt appFinder = appInfos.find(name);

    /* Branch if the application does not exist (we create it and return a
     * pointer to it). */
    if (appFinder == appInfos.end())
    {
        if (distCountExpected == 0)
        {
            appInfos.insert(std::pair<std::string, AppInfo>
                            (name, AppInfo(name)));
        }

        else
        {
            appInfos.insert(std::pair<std::string, AppInfo>
                            (name, AppInfo(name, distCountExpected, soloApp)));
        }

        return &(appInfos.find(name)->second);
    }

    /* Branch if the application does exist (we return a pointer to it). */
    else
    {
        return &(appFinder->second);
    }
}

/* Sub-synonym. */
AppInfo* AppDB::check_create_app(std::string name)
{
    return check_create_app(name, 0, false);  /* soloApp argument not used. */
}

/* Checks appInfos for an application of a given name, returning true if such
 * an application exists and false otherwise. */
bool AppDB::check_defined_app(std::string name)
{
    AppInfoIt appFinder = appInfos.find(name);
    return appFinder != appInfos.end();
}

/* Removes an application from the database. */
void AppDB::recall_app(AppInfo* app)
{
    std::set<uint32_t>::iterator threadIt;
    std::map<uint32_t, std::string>::iterator coreIt;
    std::map<uint32_t, CoreInfo>::iterator coreInfoIt;
    std::map<uint8_t, std::string>::iterator numberIt;

    /* Clear threadToCore entries. */
    for (coreInfoIt = app->coreInfos.begin();
         coreInfoIt != app->coreInfos.end(); coreInfoIt++)
    {
        for (threadIt = coreInfoIt->second.threadsExpected.begin();
             threadIt != coreInfoIt->second.threadsExpected.end(); threadIt++)
        {
            threadToCoreAddr.erase(*threadIt);
        }
    }

    /* Clear coreToApp entries (erasing while iterating). */
    coreIt = coreToApp.begin();
    while (coreIt != coreToApp.end())
    {
        if (coreIt->second == app->name) coreToApp.erase(coreIt++);
        else coreIt++;
    }

    /* Clear numberToApp entry. */
    numberIt = numberToApp.begin();
    while (numberIt != numberToApp.end())
    {
        if (numberIt->second == app->name) break;
    }
    numberToApp.erase(numberIt);

    /* Say good bye. */
    app->state = BROKEN;
    appInfos.erase(app->name);
}

/* Prints a bunch of diagnostic information. Obviously. The argument is the
 * stream to dump to. */
void AppDB::dump(std::ofstream* stream)
{
    *stream << "AppDB dump:\n";
    if (appInfos.empty())
    {
        *stream << "No applications defined.\n";
    }
    for (AppInfoIt appIt = appInfos.begin(); appIt != appInfos.end(); appIt++)
    {
        appIt->second.dump(stream);
    }

    if (coreToApp.empty())
    {
        *stream << "No cores mapped to applications.\n";
    }
    else
    {
        *stream << "Core mapping dump:\n";
        std::map<uint32_t, std::string>::iterator coreIt;
        for (coreIt = coreToApp.begin(); coreIt != coreToApp.end(); coreIt++)
        {
            *stream << "  Core \"" << std::hex << coreIt->first
                    << "\" used by application \"" << coreIt->second << "\"\n";
        }
    }
}

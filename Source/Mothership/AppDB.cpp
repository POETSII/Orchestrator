#include "AppDB.h"

/* Checks appInfos for an application of a given name. If it doesn't exist,
 * AppDB creates it and returns a pointer to it (passing distCountExpected to
 * it as an argument). If it already exists, returns a pointer to the existing
 * application, and does not use distCountExpected. */
AppInfo* AppDB::check_create_app(std::string name, uint32_t distCountExpected)
{
    AppInfoIt appFinder = appInfos.find(name);

    /* Branch if the application does not exist (we create it and return a
     * pointer to it). */
    if (appFinder == appInfos.end())
    {
        appInfos.insert(std::pair<std::string, AppInfo>
                        (name, AppInfo(name, distCountExpected)));
        return &(appInfos.find(name)->second);
    }

    /* Branch if the application does exist (we return a pointer to it). */
    else
    {
        return &(appFinder->second);
    }
}

/* Checks appInfos for an application of a given name, returning true if such
 * an application exists and false otherwise. */
bool AppDB::check_defined_app(std::string name)
{
    AppInfoIt appFinder = appInfos.find(name);
    return appFinder != appInfos.end();
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

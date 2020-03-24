/* This source file defined methods that the Mothership uses to transition
 * applications through states. */

#include "Mothership.h"

/* Stub */
void Mothership::initialise_application(AppInfo* app)
{
    printf("Initialising application '%s'!\n", app->name.c_str());
}

/* Stub */
void Mothership::run_application(AppInfo* app)
{
    printf("Running application '%s'!\n", app->name.c_str());
}

/* Stub */
void Mothership::stop_application(AppInfo* app)
{
    printf("Stopping application '%s'!\n", app->name.c_str());
}

/* Purges all mention of an application in Mothership datastructures, as well
 * as cores and threads associated with it. */
void Mothership::recall_application(AppInfo* app)
{
    std::set<uint32_t>::iterator threadIt;
    std::map<uint32_t, std::string>::iterator coreIt;
    std::map<uint32_t, CoreInfo>::iterator coreInfoIt;

    /* Supervisor */
    superdb.unload_supervisor(app->name);

    /* Clear threadToCore entries. */
    for (coreInfoIt = app->coreInfos.begin();
         coreInfoIt != app->coreInfos.end(); coreInfoIt++)
    {
        for (threadIt = coreInfoIt->second.threadsExpected.begin();
             threadIt != coreInfoIt->second.threadsExpected.end(); threadIt++)
        {
            appdb.threadToCoreAddr.erase(*threadIt);
        }
    }

    /* Clear coreToApp entries (erasing while iterating). */
    coreIt = appdb.coreToApp.begin();
    while (coreIt != appdb.coreToApp.end())
    {
        if (coreIt->second == app->name) appdb.coreToApp.erase(coreIt++);
        else coreIt++;
    }

    /* For good measure (we're going to delete it anyway...) */
    app->state = BROKEN;

    /* Say good bye. */
    appdb.appInfos.erase(app->name);
}

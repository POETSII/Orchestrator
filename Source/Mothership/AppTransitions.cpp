/* This source file defined methods that the Mothership uses to transition
 * applications through states. */

#include "Mothership.h"

/* Prepares the application for execution. This method, in order:
 *
 *  0. set the appliction state to LOADING, so BARRIER packets are processed
 *  correctly.
 *
 *  1. loads instruction and data memory for each core.
 *
 *  2. starts all of the threads for each core.
 *
 *  3. triggers backend-level application execution for each core.
 *
 * Then, if all is well, each thread sends a BARRIER packet (Tinsel message) to
 * the Mothership (which is handled by the BackendInputBroker thread). Note
 * that steps 2 and 3 are handled in different loops. This is because MFN
 * stated (at some arbitrary point in the past) that starting all of the
 * threads, then kicking them off is safer than kicking off some cores before
 * other cores' threads have started. */
void Mothership::initialise_application(AppInfo* app)
{
    std::map<uint32_t, CoreInfo>::iterator coreIt;

    /* Staging variables for address components (for Tinsel speak) */
    uint32_t meshX;
    uint32_t meshY;
    uint32_t coreId;
    uint32_t threadId;  /* Not used, as far as I can see. */

    /* Modes to reduce code repetition between steps 2 and 3. */
    bool mode;

    app->state = LOADING;  /* 0: Set the application state to LOADING, duh. */

    /* 1: For each core, call backend.loadInstrsOntoCore and
     * loadDataViaCore. */
    for (coreIt = app->coreInfos.begin(); coreIt != app->coreInfos.end();
         coreIt++)
    {
        backend.fromAddr(coreIt->first, &meshX, &meshY, &coreId, &threadId);
        backend.loadInstrsOntoCore(coreIt->second.codePath.c_str(),
                                   meshX, meshY, coreId);
        backend.loadDataViaCore(coreIt->second.dataPath.c_str(),
                                meshX, meshY, coreId);
    }

    /* 2: For each core, kick off the threads (mode=false).
     * 3: For each core, start execution (mode=true). */
    mode = false;
    do
    {
        for (coreIt = app->coreInfos.begin(); coreIt != app->coreInfos.end();
             coreIt++)
        {
            backend.fromAddr(coreIt->first, &meshX, &meshY, &coreId,
                             &threadId);

            if (!mode)  /* 2 */
                backend.startOne(meshX, meshY, coreId,
                                 coreIt->second.threadsExpected.size());
            else  /* 3 */
                backend.goOne(meshX, meshY, coreId);
        }

        mode = !mode;
    }
    while (mode);

    /* Good stuff. Now the cores will spin up and send BARRIER messages to the
     * Mothership. */
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

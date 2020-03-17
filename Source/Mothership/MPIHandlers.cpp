/* This source file defines the private Mothership methods for handling
 * incoming MPI messages. The functionality of these handlers is described in
 * the Mothership documentation. They all take an input message (and a
 * communicator index, which we don't use), and returns */

#include "Mothership.h"

unsigned Mothership::HandleExit(PMsg_p* message, unsigned commIndex)
{
    threading.set_quit();
    return 1;  /* CommonBase's Decode reads this, returning from MPISpinner. */
}

unsigned Mothership::HandleSystKill(PMsg_p* message, unsigned commIndex)
{
    /* ThreadComms notices that we haven't set_quit, so it doesn't wait for the
     * other threads to finish before leaving. */
    return 1;
}

unsigned Mothership::HandleAppSpec(PMsg_p* message, unsigned commIndex)
{
    AppInfo* appInfo;

    /* Pull message contents */
    std::string appName;
    uint32_t distCount;
    message->Get(0, appName);
    message->Get(1, distCount);

    /* Ensure application existence idempotently (it might have been created by
     * an AppDist message). */
    appInfo = appdb.check_create_app(appName, distCount);

    /* If the application is not in the UNDERDEFINED state, post angrily and do
     * nothing else. */
    if(appInfo->state != UNDERDEFINED)
    {
        Post(402, appName, appInfo->get_state_colloquial());
        return 0;
    }

    /* Otherwise, set the distCount. */
    appInfo->distCountExpected = distCount;

    /* Check for being fully defined (transition from UNDERDEFINED to
     * DEFINED). */
    appInfo->check_update_defined_state();

    /* Check for further state transitions. */
    if (appInfo->should_we_recall()) recall_application(appInfo);
    else if (appInfo->should_we_continue()) initialise_application(appInfo);
    return 0;
}

unsigned Mothership::HandleAppDist(PMsg_p* message, unsigned commIndex)
{
    AppInfo* appInfo;
    CoreInfo* coreInfo;

    /* Pull message contents. */
    std::string appName;
    std::string codePath;
    std::string dataPath;
    uint32_t coreAddr;
    uint8_t numThreads;
    message->Get(0, appName);
    message->Get(1, codePath);
    message->Get(2, dataPath);
    message->Get(3, coreAddr);
    message->Get(4, numThreads);

    /* Ensure application existence idempotently. */
    appInfo = appdb.check_create_app(appName);

    /* If the application is not in the UNDERDEFINED state, don't post angrily
     * (otherwise we'll have hundreds of post messages). Given that an AppSpec
     * message will also be received eventually, that one will do the
     * posting. */
    if(appInfo->state != UNDERDEFINED)
    {
        return 0;
    }

    /* Otherwise, add the information for this core to the application, and
     * increment the distribution message count. */
    appInfo->distCountCurrent++;
    coreInfo = &(appInfo->coreInfos[coreAddr]);
    coreInfo->codePath = codePath;
    coreInfo->dataPath = dataPath;
    coreInfo->numThreadsExpected = numThreads;
    coreInfo->numThreadsCurrent = 0;  /* He's only a baby after all. */

    /* Check for being fully defined (transition from UNDERDEFINED to
     * DEFINED). */
    appInfo->check_update_defined_state();

    /* Check for further state transitions. */
    if (appInfo->should_we_recall()) recall_application(appInfo);
    else if (appInfo->should_we_continue()) initialise_application(appInfo);
    return 0;
}

unsigned Mothership::HandleAppSupd(PMsg_p* message, unsigned commIndex)
{
    std::string errorMessage;
    AppInfo* appInfo;

    /* Pull message contents. */
    std::string appName;
    std::string soPath;
    message->Get(0, appName);
    message->Get(1, soPath);

    /* Ensure application existence idempotently, either to set its status to
     * BROKEN (on failure), or to update the distribution count. */
    appInfo = appdb.check_create_app(appName);

    /* Try to load the supervisor, marking the thing as broken if it fell
     * over. */
    if(!superdb.load_supervisor(appName, soPath, &errorMessage))
    {
        Post(403, appName, errorMessage);
        appInfo->state = BROKEN;
        return 0;
    }

    /* Otherwise, increment the distribution count. */
    appInfo->distCountCurrent++;

    /* Check for being fully defined (transition from UNDERDEFINED to
     * DEFINED). */
    appInfo->check_update_defined_state();

    /* Check for further state transitions. */
    if (appInfo->should_we_recall()) recall_application(appInfo);
    else if (appInfo->should_we_continue()) initialise_application(appInfo);
    return 0;
}

unsigned Mothership::HandleCmndRecl(PMsg_p* message, unsigned commIndex)
{
    AppInfo* appInfo;

    /* Pull message contents. */
    std::string appName;
    message->Get(0, appName);

    /* Get the application */
    appInfo = appdb.check_create_app(appName);

    /* Mark it as recalled, and check for state transitions. */
    appInfo->stage_recl();
    if (appInfo->should_we_recall()) recall_application(appInfo);
    return 0;
}

unsigned Mothership::HandleCmndInit(PMsg_p* message, unsigned commIndex)
{
    AppInfo* appInfo;

    /* Pull message contents. */
    std::string appName;
    message->Get(0, appName);

    /* Get the application */
    appInfo = appdb.check_create_app(appName);

    /* Stage the init command, and run it now if possible. */
    appInfo->stage_init();
    if (appInfo->state == DEFINED) initialise_application(appInfo);
    return 0;
}

unsigned Mothership::HandleCmndRun(PMsg_p* message, unsigned commIndex)
{
    AppInfo* appInfo;

    /* Pull message contents. */
    std::string appName;
    message->Get(0, appName);

    /* Get the application */
    appInfo = appdb.check_create_app(appName);

    /* Stage the run command, and run it now if possible. */
    appInfo->stage_run();
    if (appInfo->state == READY) run_application(appInfo);
    return 0;
}

unsigned Mothership::HandleCmndStop(PMsg_p* message, unsigned commIndex)
{
    AppInfo* appInfo;

    /* Pull message contents. */
    std::string appName;
    message->Get(0, appName);

    /* Get the application */
    appInfo = appdb.check_create_app(appName);

    /* Stage the stop command, and run it now if possible. */
    appInfo->stage_stop();
    if (appInfo->state == RUNNING) stop_application(appInfo);
    return 0;
}

unsigned Mothership::HandleBendCnc(PMsg_p* message, unsigned commIndex)
{
    printf("BendCnc message received!\n"); return 0;
}
unsigned Mothership::HandleBendSupr(PMsg_p* message, unsigned commIndex)
{
    printf("BendSupr message received!\n"); return 0;
}
unsigned Mothership::HandlePkts(PMsg_p* message, unsigned commIndex)
{
    printf("Pkts message received!\n"); return 0;
}
unsigned Mothership::HandleDump(PMsg_p* message, unsigned commIndex)
{
    printf("Dump message received!\n"); return 0;
}

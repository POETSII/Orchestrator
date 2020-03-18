/* This source file defines the private Mothership methods for handling
 * incoming MPI messages. The functionality of these handlers is described in
 * the Mothership documentation.
 *
 * Each handler is called by one of two sources (disambiguated by the dyadic
 * nature of their signature):
 *
 * - MPISpinner (and Decode): These either handle exit messages, or push
 *    messages into either MPICncQueue or MPIAppQueue.
 *
 * - Consumers of the above queues, which updates application states.
 *
 * See Mothership.h for the taxonomy. */

#include "Mothership.h"

unsigned Mothership::handle_exit(PMsg_p* message, unsigned commIndex)
{
    threading.set_quit();
    return 1;  /* CommonBase's Decode reads this, returning from MPISpinner. */
}

unsigned Mothership::handle_syst_kill(PMsg_p* message, unsigned commIndex)
{
    /* ThreadComms notices that we haven't set_quit, so it doesn't wait for the
     * other threads to finish before leaving. */
    return 1;
}

unsigned Mothership::handle_app(PMsg_p* message, unsigned commIndex)
{
    threading.push_MPI_app_queue(*message);
    return 0;
}

unsigned Mothership::handle_cnc(PMsg_p* message, unsigned commIndex)
{
    threading.push_MPI_cnc_queue(*message);
    return 0;
}

unsigned Mothership::handle_app_spec(PMsg_p* message)
{
    AppInfo* appInfo;

    /* Pull message contents. */
    std::string appName;
    uint32_t distCount;
    if (!decode_app_spec_message(message, &appName, &distCount)) return 0;

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

unsigned Mothership::handle_app_dist(PMsg_p* message)
{
    AppInfo* appInfo;
    CoreInfo* coreInfo;

    /* Pull message contents. */
    std::string appName;
    std::string codePath;
    std::string dataPath;
    uint32_t coreAddr;
    unsigned numThreads;
    if (!decode_app_dist_message(message, &appName, &codePath,
                                 &dataPath, &coreAddr, &numThreads)) return 0;

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

unsigned Mothership::handle_app_supd(PMsg_p* message)
{
    std::string errorMessage;
    AppInfo* appInfo;

    /* Pull message contents. */
    std::string appName;
    std::string soPath;
    if (!decode_app_supd_message(message, &appName, &soPath)) return 0;

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

unsigned Mothership::handle_cmnd_recl(PMsg_p* message)
{
    AppInfo* appInfo;

    /* Pull message contents. */
    std::string appName;
    if (!decode_string_message(message, &appName)) return 0;

    /* Get the application */
    appInfo = appdb.check_create_app(appName);

    /* Mark it as recalled, and check for state transitions. */
    appInfo->stage_recl();
    if (appInfo->should_we_recall()) recall_application(appInfo);
    return 0;
}

unsigned Mothership::handle_cmnd_init(PMsg_p* message)
{
    AppInfo* appInfo;

    /* Pull message contents. */
    std::string appName;
    if (!decode_string_message(message, &appName)) return 0;

    /* Get the application */
    appInfo = appdb.check_create_app(appName);

    /* Stage the init command, and run it now if possible. */
    appInfo->stage_init();
    if (appInfo->state == DEFINED) initialise_application(appInfo);
    return 0;
}

unsigned Mothership::handle_cmnd_run(PMsg_p* message)
{
    AppInfo* appInfo;

    /* Pull message contents. */
    std::string appName;
    if (!decode_string_message(message, &appName)) return 0;

    /* Get the application */
    appInfo = appdb.check_create_app(appName);

    /* Stage the run command, and run it now if possible. */
    appInfo->stage_run();
    if (appInfo->state == READY) run_application(appInfo);
    return 0;
}

unsigned Mothership::handle_cmnd_stop(PMsg_p* message)
{
    AppInfo* appInfo;

    /* Pull message contents. */
    std::string appName;
    if (!decode_string_message(message, &appName)) return 0;

    /* Get the application */
    appInfo = appdb.check_create_app(appName);

    /* Stage the stop command, and run it now if possible. */
    appInfo->stage_stop();
    if (appInfo->state == RUNNING) stop_application(appInfo);
    return 0;
}

/* Stub */
unsigned Mothership::handle_bend_cnc(PMsg_p* message)
{
    /* Pull message contents. */
    std::vector<P_Pkt_t> packets;
    if (!decode_packets_message(message, &packets)) return 0;

    printf("BendCnc message received!\n"); return 0;
}

/* Stub */
unsigned Mothership::handle_bend_supr(PMsg_p* message)
{
    /* Pull message contents. */
    std::vector<P_Pkt_t> packets;
    if (!decode_packets_message(message, &packets)) return 0;

    printf("BendSupr message received!\n"); return 0;
}

unsigned Mothership::handle_pkts(PMsg_p* message)
{
    /* Pull message contents. */
    std::vector<P_Pkt_t> packets;
    if (!decode_packets_message(message, &packets)) return 0;

    /* Queue 'em. */
    threading.push_backend_out_queue(&packets);
    return 0;
}

unsigned Mothership::handle_dump(PMsg_p* message)
{
    std::string dumpPath;
    std::ofstream outS;
    FILE* outF;

    /* Pull message contents. */
    if (!decode_string_message(message, &dumpPath)) return 0;

    /* Mothership dump */
    outS.open(dumpPath.c_str(), std::ofstream::out | std::ofstream::trunc);
    if (outS.fail())
    {
        Post(408, dumpPath, POETS::getSysErrorString(errno));
        return 0;
    }
    dump(&outS);
    outS.close();

    /* CommonBase dump (see comment heading of Mothership::dump) */
    outF = fopen(dumpPath.c_str(), "a");
    if (outF == PNULL)
    {
        Post(408, dumpPath, POETS::getSysErrorString(errno));
        return 0;
    }
    Dump(outF);
    fclose(outF);
    return 0;
}

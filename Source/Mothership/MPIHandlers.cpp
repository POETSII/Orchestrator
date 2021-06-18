/* This source file defines the Mothership methods for handling incoming MPI
 * messages. The functionality of these handlers is described in the Mothership
 * documentation.
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

unsigned Mothership::handle_msg_exit(PMsg_p* message)
{
    debug_post(598, 2, "Q::EXIT", "Exiting gracefully.");
    threading.set_quit();

    /* CommonBase's OnExit (does some MPI teardown). The returncode is read by
     * Decode, and causes MPISpinner to end. */
    OnExit(message);
    return 1;
}

unsigned Mothership::handle_msg_syst_kill(PMsg_p* message)
{
    /* ThreadComms notices that we haven't set_quit, so it doesn't wait for the
     * other threads to finish before leaving. */
    debug_post(598, 2, "Q::SYST,Q::KILL", "Exiting as quickly as possible.");

    /* CommonBase's OnExit (does some MPI teardown). The returncode is read by
     * Decode, and causes MPISpinner to end. */
    OnExit(message);
    return 1;
}

unsigned Mothership::handle_msg_app(PMsg_p* message)
{
    #if ORCHESTRATOR_DEBUG
    std::string key = "Unknown";
    if (message->Key() == PMsg_p::KEY(Q::PKTS)) key = "Q::PKTS";
    else if (message->Key() == PMsg_p::KEY(Q::BEND, Q::SUPR))
        key = "Q::BEND,Q::SUPR";
    debug_post(599, 2, key.c_str(), "MPI Application");
    #endif
    threading.push_MPI_app_queue(*message);
    return 0;
}

unsigned Mothership::handle_msg_cnc(PMsg_p* message)
{
    #if ORCHESTRATOR_DEBUG
    std::string key = "Unknown";
    if (message->Key() == PMsg_p::KEY(Q::APP,Q::SPEC))
        key = "Q::APP,Q::SPEC";
    else if (message->Key() == PMsg_p::KEY(Q::APP,Q::SPEC))
        key = "Q::APP,Q::SPEC";
    else if (message->Key() == PMsg_p::KEY(Q::APP,Q::DIST))
        key = "Q::APP,Q::DIST";
    else if (message->Key() == PMsg_p::KEY(Q::APP,Q::SUPD))
        key = "Q::APP,Q::SUPD";
    else if (message->Key() == PMsg_p::KEY(Q::CMND,Q::BRKN))
        key = "Q::CMND,Q::BRKN";
    else if (message->Key() == PMsg_p::KEY(Q::CMND,Q::RECL))
        key = "Q::CMND,Q::RECL";
    else if (message->Key() == PMsg_p::KEY(Q::CMND,Q::INIT))
        key = "Q::CMND,Q::INIT";
    else if (message->Key() == PMsg_p::KEY(Q::CMND,Q::RUN))
        key = "Q::CMND,Q::RUN";
    else if (message->Key() == PMsg_p::KEY(Q::CMND,Q::STOP))
        key = "Q::CMND,Q::STOP";
    else if (message->Key() == PMsg_p::KEY(Q::DUMP)) key = "Q::DUMP";
    debug_post(599, 2, key.c_str(), "MPI Command-and-control");
    #endif
    threading.push_MPI_cnc_queue(*message);
    return 0;
}

unsigned Mothership::handle_msg_app_spec(PMsg_p* message)
{
    AppInfo* appInfo;

    /* Pull message contents. */
    std::string appName;
    uint32_t distCount;
    uint8_t appNumber;
    if (!decode_app_spec_message(message, &appName, &distCount,
                                 &appNumber))
    {
        debug_post(597, 3, "Q::APP,Q::SPEC", hex2str(message->Key()).c_str(),
                   "Failed to decode.");
        return 0;
    }

    debug_post(597, 3, "Q::APP,Q::SPEC", hex2str(message->Key()).c_str(),
               dformat("appName=%s, distCount=%u, appNumber=%u",
                       appName.c_str(), distCount, appNumber).c_str());

    /* Ensure application existence idempotently (it might have been created by
     * an AppDist message). */
    appInfo = appdb.check_create_app(appName, distCount);

    /* If the application is not in the UNDERDEFINED state, post bossily and do
     * nothing else. */
    if(appInfo->state != UNDERDEFINED)
    {
        Post(502, appName, appName, appInfo->get_state_colloquial());
        return 0;
    }

    /* Define the application number in the database. */
    appdb.numberToApp[appNumber] = appName;

    /* Otherwise, set the distCount. */
    appInfo->distCountExpected = distCount;

    /* Check for being fully defined (transition from UNDERDEFINED to
     * DEFINED). If it is, report back to Root and Post. */
    if (appInfo->check_update_defined_state())
    {
        Post(529, int2str(Urank), appInfo->name);
        PMsg_p acknowledgement;
        acknowledgement.Src(Urank);
        acknowledgement.Put<std::string>(0, &(appInfo->name));
        acknowledgement.Tgt(pPmap->U.Root);
        acknowledgement.Key(Q::MSHP, Q::ACK, Q::DEFD);
        queue_mpi_message(&acknowledgement);
    }

    /* Check for further state transitions. */
    if (appInfo->should_we_recall()) recall_application(appInfo);
    else if (appInfo->should_we_continue()) initialise_application(appInfo);
    return 0;
}

unsigned Mothership::handle_msg_app_dist(PMsg_p* message)
{
    AppInfo* appInfo;
    CoreInfo* coreInfo;

    /* Pull message contents. */
    std::string appName;
    std::string codePath;
    std::string dataPath;
    uint32_t coreAddr;
    std::vector<uint32_t>::iterator threadAddrIt;
    std::vector<uint32_t> threadsExpected;
    if (!decode_app_dist_message(message, &appName, &codePath,
                                 &dataPath, &coreAddr,
                                 &threadsExpected))
    {
        debug_post(597, 3, "Q::APP,Q::DIST", hex2str(message->Key()).c_str(),
                   "Failed to decode.");
        return 0;
    }

    #if ORCHESTRATOR_DEBUG
    std::string threadNames = "";
    for (threadAddrIt = threadsExpected.begin();
         threadAddrIt != threadsExpected.end(); threadAddrIt++)
        threadNames.append(dformat("0x%x ", *threadAddrIt));
    debug_post(597, 3, "Q::APP,Q::DIST", hex2str(message->Key()).c_str(),
               dformat("appName=%s, codePath=%s, dataPath=%s, coreAddr=0x%x, "
                       "threadsExpected=%s",
                       appName.c_str(), codePath.c_str(), dataPath.c_str(),
                       coreAddr, threadNames.c_str()).c_str());
    #endif

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
     * increment the distribution message count. If the distribution message
     * count is too high, shout loudly, and break the application. */
    coreInfo = &(appInfo->coreInfos[coreAddr]);
    coreInfo->codePath = codePath;
    coreInfo->dataPath = dataPath;
    appdb.coreToApp[coreAddr] = appName;
    if (!appInfo->increment_dist_count_current())
    {
        Post(524, appName, uint2str(appInfo->distCountExpected));
        tell_root_app_is_broken(appName);
        appInfo->state = BROKEN;
        return 0;
    }

    /* For each thread, add it to:
     *
     * 1. The list of expected threads for the core
     *
     * 2. The "reverse" map in AppDB (so we can get the application and the
     * core from the hardware address of an incoming packet. */
    for (threadAddrIt = threadsExpected.begin();
         threadAddrIt != threadsExpected.end();
         threadAddrIt++)
    {
        coreInfo->threadsExpected.insert(*threadAddrIt);  /* (1) */
        appdb.threadToCoreAddr[*threadAddrIt] = coreAddr;  /* (2) */
    }

    /* Check for being fully defined (transition from UNDERDEFINED to
     * DEFINED). If it is, report back to Root and Post. */
    if (appInfo->check_update_defined_state())
    {
        Post(529, int2str(Urank), appInfo->name);
        PMsg_p acknowledgement;
        acknowledgement.Src(Urank);
        acknowledgement.Put<std::string>(0, &(appInfo->name));
        acknowledgement.Tgt(pPmap->U.Root);
        acknowledgement.Key(Q::MSHP, Q::ACK, Q::DEFD);
        queue_mpi_message(&acknowledgement);
    }

    /* Check for further state transitions. */
    if (appInfo->should_we_recall()) recall_application(appInfo);
    else if (appInfo->should_we_continue()) initialise_application(appInfo);
    return 0;
}

unsigned Mothership::handle_msg_app_supd(PMsg_p* message)
{
    std::string errorMessage;
    AppInfo* appInfo;

    /* Pull message contents. */
    std::string appName;
    std::string soPath;
    if (!decode_app_supd_message(message, &appName, &soPath))
    {
        debug_post(597, 3, "Q::APP,Q::SUPD", hex2str(message->Key()).c_str(),
                   "Failed to decode.");
        return 0;
    }

    debug_post(597, 3, "Q::APP,Q::SUPD", hex2str(message->Key()).c_str(),
               dformat("appName=%s, soPath=%s",
                       appName.c_str(), soPath.c_str()).c_str());

    /* Ensure application existence idempotently, either to set its status to
     * BROKEN (on failure), or to update the distribution count. */
    appInfo = appdb.check_create_app(appName);

    /* Try to load the supervisor, marking the thing as broken if it fell
     * over. */
    if(!superdb.load_supervisor(appName, soPath, &errorMessage))
    {
        Post(503, appName, errorMessage);
        tell_root_app_is_broken(appName);
        appInfo->state = BROKEN;
        return 0;
    }

    /* On loading the supervisor, provision its API. */
    if(!provision_supervisor_api(appName))
    {
        Post(525, appName);
        tell_root_app_is_broken(appName);
        appInfo->state = BROKEN;
        return 0;
    }

    /* Otherwise, increment the distribution message count. If the distribution
     * message count is too high, shout loudly, and break the application. */
    if (!appInfo->increment_dist_count_current())
    {
        Post(524, appName, uint2str(appInfo->distCountExpected));
        tell_root_app_is_broken(appName);
        appInfo->state = BROKEN;
        return 0;
    }

    /* Check for being fully defined (transition from UNDERDEFINED to
     * DEFINED). If it is, report back to Root and Post. */
    if (appInfo->check_update_defined_state())
    {
        Post(529, int2str(Urank), appInfo->name);
        PMsg_p acknowledgement;
        acknowledgement.Src(Urank);
        acknowledgement.Put<std::string>(0, &(appInfo->name));
        acknowledgement.Tgt(pPmap->U.Root);
        acknowledgement.Key(Q::MSHP, Q::ACK, Q::DEFD);
        queue_mpi_message(&acknowledgement);
    }

    /* Check for further state transitions. */
    if (appInfo->should_we_recall()) recall_application(appInfo);
    else if (appInfo->should_we_continue()) initialise_application(appInfo);
    return 0;
}

unsigned Mothership::handle_msg_cmnd_recl(PMsg_p* message)
{
    AppInfo* appInfo;

    /* Pull message contents. */
    std::string appName;
    if (!decode_string_message(message, &appName))
    {
        debug_post(597, 3, "Q::CMND,Q::RECL", hex2str(message->Key()).c_str(),
                   "Failed to decode.");
        return 0;
    }

    debug_post(597, 3, "Q::CMND,Q::RECL", hex2str(message->Key()).c_str(),
               dformat("appName=%s", appName.c_str()).c_str());

    /* Get the application */
    appInfo = appdb.check_create_app(appName);

    /* Mark it as recalled, and check for state transitions. */
    appInfo->stage_recl();
    if (appInfo->should_we_recall()) recall_application(appInfo);
    return 0;
}

unsigned Mothership::handle_msg_cmnd_brkn(PMsg_p* message)
{
    AppInfo* appInfo;

    /* Pull message contents. */
    std::string appName;
    if (!decode_string_message(message, &appName))
    {
        debug_post(597, 3, "Q::CMND,Q::BRKN", hex2str(message->Key()).c_str(),
                   "Failed to decode.");
        return 0;
    }

    debug_post(597, 3, "Q::CMND,Q::BRKN", hex2str(message->Key()).c_str(),
               dformat("appName=%s", appName.c_str()).c_str());

    /* Get the application */
    appInfo = appdb.check_create_app(appName);

    /* If the app is running, stop it. */
    if (appInfo->state == RUNNING)
    {
        appInfo->stage_stop();
        stop_application(appInfo);
    }

    /* Mark it as broken (even if it is already marked as such). Note that,
     * while the app is stopping, it will have state "STOPPING". After all of
     * the devices have reported back, it will have state "BROKEN" again.
     *
     * Breaking in this way does not inform Root, because otherwise we would be
     * bouncing messages forever. */
    appInfo->state = BROKEN;
    return 0;
}

unsigned Mothership::handle_msg_cmnd_init(PMsg_p* message)
{
    AppInfo* appInfo;

    /* Pull message contents. */
    std::string appName;
    if (!decode_string_message(message, &appName))
    {
        debug_post(597, 3, "Q::CMND,Q::INIT", hex2str(message->Key()).c_str(),
                   "Failed to decode.");
        return 0;
    }

    debug_post(597, 3, "Q::CMND,Q::INIT", hex2str(message->Key()).c_str(),
               dformat("appName=%s", appName.c_str()).c_str());

    /* Get the application */
    appInfo = appdb.check_create_app(appName);

    /* Stage the init command, and run it now if possible. */
    appInfo->stage_init();
    if (appInfo->state == DEFINED) initialise_application(appInfo);
    return 0;
}

unsigned Mothership::handle_msg_cmnd_run(PMsg_p* message)
{
    AppInfo* appInfo;

    /* Pull message contents. */
    std::string appName;
    if (!decode_string_message(message, &appName))
    {
        debug_post(597, 3, "Q::CMND,Q::RUN", hex2str(message->Key()).c_str(),
                   "Failed to decode.");
        return 0;
    }

    debug_post(597, 3, "Q::CMND,Q::RUN", hex2str(message->Key()).c_str(),
               dformat("appName=%s", appName.c_str()).c_str());

    /* Get the application */
    appInfo = appdb.check_create_app(appName);

    /* Stage the run command, and run it now if possible. */
    appInfo->stage_run();
    if (appInfo->state == READY) run_application(appInfo);
    return 0;
}

unsigned Mothership::handle_msg_cmnd_stop(PMsg_p* message)
{
    AppInfo* appInfo;

    /* Pull message contents. */
    std::string appName;
    if (!decode_string_message(message, &appName))
    {
        debug_post(597, 3, "Q::CMND,Q::STOP", hex2str(message->Key()).c_str(),
                   "Failed to decode.");
        return 0;
    }

    debug_post(597, 3, "Q::CMND,Q::STOP", hex2str(message->Key()).c_str(),
               dformat("appName=%s", appName.c_str()).c_str());

    /* Get the application */
    appInfo = appdb.check_create_app(appName);

    /* Stage the stop command, and run it now if possible. */
    appInfo->stage_stop();
    if (appInfo->state == RUNNING) stop_application(appInfo);
    return 0;
}

unsigned Mothership::handle_msg_bend_cnc(PMsg_p* message)
{
    uint8_t opcode;

    /* Pull message contents. */
    std::vector<P_Pkt_t> packets;
    if (!decode_packets_message(message, &packets))
    {
        debug_post(597, 3, "Q::BEND,Q::CNC", hex2str(message->Key()).c_str(),
                   "Failed to decode.");
        return 0;
    }

    debug_post(597, 3, "Q::BEND,Q::CNC", hex2str(message->Key()).c_str(),
               dformat("%lu packet(s).", packets.size()).c_str());

    /* For each packet, get its opcode, and call the appropriate packet
     * handling method, Posting if we don't recognise it. */
    for (std::vector<P_Pkt_t>::iterator packetIt = packets.begin();
         packetIt != packets.end(); packetIt++)
    {
        opcode = ((*packetIt).header.swAddr & P_SW_OPCODE_MASK)
            >> P_SW_OPCODE_SHIFT;
        switch (opcode)
        {
            case P_CNC_INSTR : handle_pkt_instr(&*packetIt); break;
            case P_CNC_LOG : handle_pkt_log(&*packetIt); break;
            case P_CNC_BARRIER : handle_pkt_barrier(&*packetIt); break;
            case P_CNC_STOP : handle_pkt_stop(&*packetIt); break;
            case P_CNC_KILL : handle_pkt_kill(&*packetIt); break;
            default : Post(514, hex2str(opcode));
        }
    }

    return 0;
}

unsigned Mothership::handle_msg_bend_supr(PMsg_p* message)
{
    int rc;

    /* Get the application from the message. */
    std::string appName;
    if (!decode_string_message(message, &appName))
    {
        debug_post(597, 3, "Q::BEND,Q::SUPR", hex2str(message->Key()).c_str(),
                   "Failed to decode.");
        return 0;
    }

    debug_post(597, 3, "Q::BEND,Q::SUPR", hex2str(message->Key()).c_str(),
               dformat("appName=%s", appName.c_str()).c_str());

    /* If the application is not running, we complain in debug. This may be
     * behaviour for a well-behaved application, as a normal device may send
     * packets to its supervisor before it receives the 'stop' packet from its
     * Mothership. */
    if (appdb.check_create_app(appName)->state != RUNNING)
    {
        debug_post(582, 1, appName);
        return 0;
    }

    /* Set up a vector of packets for the supervisor entry point to modify. If
     * the vector comes back with entries, then we have packets to send.
     */
    std::vector<P_Pkt_t> inputPackets;
    std::vector<P_Addr_Pkt_t> outputPackets;

    /* Get the input packets. */
    decode_packets_message(message, &inputPackets, 1);

    /* Invoke the supervisor, send the message if instructed to do so, and
     * propagate errors. */
    rc = superdb.call_supervisor(appName, inputPackets, outputPackets);
    if (outputPackets.size() > 0)
    {
        PMsg_p outputMessage;
        outputMessage.Tgt(Urank);
        outputMessage.Src(Urank);
        outputMessage.Key(Q::PKTS);
        outputMessage.Put<P_Addr_Pkt_t> (0, &(outputPackets));
        queue_mpi_message(&outputMessage);
    }
    if (rc < 0) Post(515, appName);
    return 0;
}

unsigned Mothership::handle_msg_path(PMsg_p* message)
{
    /* Pull message contents. */
    std::string userOutRoot;
    if (!decode_string_message(message, &userOutRoot))
    {
        debug_post(597, 3, "Q::PATH", hex2str(message->Key()).c_str(),
                   "Failed to decode.");
        return 0;
    }

    debug_post(597, 3, "Q::PATH", hex2str(message->Key()).c_str(),
               dformat("userOutRoot=%s", userOutRoot.c_str()).c_str());

    /* Out we go. */
    userOutDir = userOutRoot;
    return 0;
}

unsigned Mothership::handle_msg_pkts(PMsg_p* message)
{
    /* Pull message contents. */
    std::vector<P_Addr_Pkt_t> packets;
    if (!decode_addressed_packets_message(message, &packets))
    {
        debug_post(597, 3, "Q::PKTS", hex2str(message->Key()).c_str(),
                   "Failed to decode.");
        return 0;
    }

    debug_post(597, 3, "Q::BEND,Q::PKTS", hex2str(message->Key()).c_str(),
               "Various packets (pushing to queue).");

    /* Queue 'em. */
    threading.push_backend_out_queue(&packets);
    return 0;
}

unsigned Mothership::handle_msg_dump(PMsg_p* message)
{
    std::string dumpPath;
    std::ofstream outS;
    FILE* outF;

    /* Pull message contents. */
    if (!decode_string_message(message, &dumpPath))
    {
        debug_post(597, 3, "Q::DUMP", hex2str(message->Key()).c_str(),
                   "Failed to decode.");
        return 0;
    }

    debug_post(597, 3, "Q::DUMP", hex2str(message->Key()).c_str(),
               dformat("dumpPath=%s", dumpPath.c_str()).c_str());

    /* Mothership dump */
    outS.open(dumpPath.c_str(), std::ofstream::out | std::ofstream::trunc);
    if (outS.fail())
    {
        Post(508, dumpPath, OSFixes::getSysErrorString(errno));
        return 0;
    }
    dump(&outS);
    outS.close();

    /* CommonBase dump (see comment heading of Mothership::dump) */
    outF = fopen(dumpPath.c_str(), "a");
    if (outF == PNULL)
    {
        Post(508, dumpPath, OSFixes::getSysErrorString(errno));
        return 0;
    }
    Dump(0, outF);
    fclose(outF);
    return 0;
}

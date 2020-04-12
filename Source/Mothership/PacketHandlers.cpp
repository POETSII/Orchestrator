/* This source file defines the Mothership methods for handling incoming
 * packets from the compute fabric. These handlers are to be exclusively called
 * by MPICncResolver (which also handles command-and-control tasks from
 * Root).
 *
 * See the Mothership documentation (particularly the Tinsel Command and
 * Control section) for more information on how these command-and-control
 * packets are handled. */

#include "Mothership.h"

/* Handle a packet as an instrumentation packet (see
 * InstrumentationWriter::consume_instrumentation_packet) */
void Mothership::handle_pkt_instr(P_Pkt_t* packet)
{
    debug_post(481, 2, "P_CNC_INSTR", hex2str(packet->header.pinAddr).c_str());
    try
    {
        instrumentation.consume_instrumentation_packet(packet);
    }

    catch (InstrumentationException &e)
    {
        Post(409, e.message);
    }
}

/* Handle a packet as an instrumentation packet (see
 * LogHandler::consume_log_packet) */
void Mothership::handle_pkt_log(P_Pkt_t* packet)
{
    std::string message;
    debug_post(481, 2, "P_CNC_LOG", hex2str(packet->header.pinAddr).c_str());
    logging.consume_log_packet(packet, &message);
    if (!message.empty()) Post(410, message);
}

/* Handle a packet as a barrier packet. Such packets are, effectively,
 * acknowledgements from the thread confirming that they have loaded correctly,
 * and are waiting at the softswitch barrier.  */
void Mothership::handle_pkt_barrier(P_Pkt_t* packet)
{
    debug_post(481, 2, "P_CNC_BARRIER", hex2str(packet->header.pinAddr).c_str());
    handle_pkt_barrier_or_stop(packet, false);
}

/* So I noticed that there was a lot of code duplication between
 * handle_pkt_barrier and handle_pkt_stop, so I decided to consolidate them
 * together. Simply put, BARRIER-mode (stop=false) adds threads and cores to
 * things, whereas STOP-mode (stop=true) removes them. */
void Mothership::handle_pkt_barrier_or_stop(P_Pkt_t* packet, bool stop)
{
    uint32_t coreAddr;
    uint32_t threadAddr;
    std::string appName;
    AppInfo* appInfo;
    CoreInfo* coreInfo;
    std::map<uint32_t, uint32_t>::iterator coreAddrFinder;
    std::map<uint32_t, std::string>::iterator appNameFinder;
    std::string colloquialMode;

    /* For error messages and debugging. */
    if (stop) colloquialMode = "STOP";
    else colloquialMode = "BARRIER";

    /* Extract the source address for this packet, so that the compute thread
     * (and hence application and core) can be identified. */
    threadAddr = packet->header.pinAddr;

    /* Get the core address mapped for this thread. If we don't know it, error
     * out. */
    coreAddrFinder = appdb.threadToCoreAddr.find(threadAddr);
    if (coreAddrFinder == appdb.threadToCoreAddr.end())
    {
        Post(411, hex2str(threadAddr), colloquialMode);
        return;
    }
    coreAddr = coreAddrFinder->second;

    /* Get the application for this core. If we don't know it, error out. */
    appNameFinder = appdb.coreToApp.find(coreAddr);
    if (appNameFinder == appdb.coreToApp.end())
    {
        Post(412, hex2str(coreAddr), colloquialMode);
        return;
    }
    appName = appNameFinder->second;

    /* We only expect to receive barrier packets when the application state is
     * LOADING, and stop packets when the application state is STOPPING. After
     * all, if a device is waiting at the softswitch barrier in a different
     * state, that would suggest that the Mothership and that Softswitch have
     * gone out of sync somehow. We catch that here. */
    appInfo = appdb.check_create_app(appName); /* Assuming existence. */
    if ((!stop and appInfo->state != LOADING) or  /* BARRIER mode */
        (stop and appInfo->state != STOPPING))    /* STOP mode */
    {
        Post(413, colloquialMode, appInfo->get_state_colloquial());
        return;
    }

    /* Store (BARRIER) or remove (STOP) the thread address in the set of
     * addresses that have reported back for this core in this application. */
    coreInfo = &(appInfo->coreInfos[coreAddr]);
    if (!stop) coreInfo->threadsCurrent.insert(threadAddr);
    else coreInfo->threadsCurrent.erase(threadAddr);

    /* If we're waiting for more threads to respond, do nothing else. */
    if ((!stop and coreInfo->threadsCurrent != coreInfo->threadsExpected) or
        (stop and !(coreInfo->threadsCurrent.empty()))) return;

    /* Otherwise, all threads have been loaded for this core. Mark the core as
     * loaded for this application. */
    if (!stop) appInfo->coresLoaded.insert(coreAddr);
    else appInfo->coresLoaded.erase(coreAddr);

    /* If we're waiting for more cores to respond, do nothing else. The "-1"
     * exists in the BARRIER case because the supervisor device is also counted
     * in the distribution message total. */
    if ((!stop and appInfo->coresLoaded.size() <
         appInfo->distCountExpected - 1) or
        (stop and !(appInfo->coresLoaded.empty()))) return;

    /* Otherwise, all threads on all cores have responded. Mark the application
     * as READY (or STOPPED) and report back to Root. */
    PMsg_p acknowledgement;
    acknowledgement.comm = Comms[RootCIdx()];
    acknowledgement.Src(Urank);
    acknowledgement.Put<std::string>(0, &(appInfo->name));
    acknowledgement.Tgt(pPmap[RootCIdx()]->U.Root);
    if (!stop)
    {
        appInfo->state = READY;
        acknowledgement.Key(Q::MSHP, Q::ACK, Q::LOAD);
    }
    else
    {
        appInfo->state = STOPPED;
        acknowledgement.Key(Q::MSHP, Q::ACK, Q::STOP);
    }
    queue_mpi_message(&acknowledgement);

    /* Check for further state transitions. */
    if (appInfo->should_we_recall()) recall_application(appInfo);
    /* Can't "continue" from a stopped application */
    else if (appInfo->should_we_continue()) run_application(appInfo);
    return;
}

/* Handle a packet as a stop packet. Such packets are, effectively,
 * acknowledgements from the thread confirming that they have received the stop
 * command and are shutting down and stopping meaningful compute. */
void Mothership::handle_pkt_stop(P_Pkt_t* packet)
{
    debug_post(481, 2, "P_CNC_STOP", hex2str(packet->header.pinAddr).c_str());
    handle_pkt_barrier_or_stop(packet, true);
}

void Mothership::handle_pkt_kill(P_Pkt_t* packet)
{
    debug_post(481, 2, "P_CNC_KILL", hex2str(packet->header.pinAddr).c_str());

    /* We manage killing the Mothership via MPI - the message is consumed by
     * MPIInputBroker, which tells everything to stop gracefully. */
    PMsg_p message;
    message.comm = Comms[0];
    message.Key(Q::EXIT);
    message.Tgt(Urank);  /* Send to ourselves */
    queue_mpi_message(&message);
}

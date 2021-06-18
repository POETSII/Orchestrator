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
 *  4. initialises the supervisor device on this Mothership.
 *
 * After 3, if all is well, each thread sends a BARRIER packet (Tinsel message)
 * to the Mothership (which is handled by the BackendInputBroker thread). Note
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

    Post(530, int2str(Urank), app->name.c_str());

    app->state = LOADING;  /* 0: Set the application state to LOADING, duh. */

    /* 1: For each core, call backend->loadInstrsOntoCore and
     * loadDataViaCore. */
    pthread_mutex_lock(&(threading.mutex_backend_api));
    for (coreIt = app->coreInfos.begin(); coreIt != app->coreInfos.end();
         coreIt++)
    {
        backend->fromAddr(coreIt->first, &meshX, &meshY, &coreId, &threadId);
        debug_post(585, 4, coreIt->second.codePath.c_str(),
                   hex2str(meshX).c_str(), hex2str(meshY).c_str(),
                   hex2str(coreId).c_str());
        backend->loadInstrsOntoCore(coreIt->second.codePath.c_str(),
                                   meshX, meshY, coreId);
        debug_post(584, 4, coreIt->second.dataPath.c_str(),
                   hex2str(meshX).c_str(), hex2str(meshY).c_str(),
                   hex2str(coreId).c_str());
        backend->loadDataViaCore(coreIt->second.dataPath.c_str(),
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
            backend->fromAddr(coreIt->first, &meshX, &meshY, &coreId,
                              &threadId);

            if (!mode)  /* 2 */
            {
                debug_post(
                    587, 4, hex2str(meshX).c_str(), hex2str(meshY).c_str(),
                    hex2str(coreId).c_str(),
                    uint2str(coreIt->second.threadsExpected.size()).c_str());
                /* Note that startOne can hang for the Tinsel backend if the
                 * number of threads expected is greater than the number of
                 * threads that the core will start - this is because startOne
                 * waits for an acknowledgement message from the core that
                 * varies as a function of the number of threads. If you find
                 * the above 587 being the last message you see from the MPI
                 * CNC Resolver thread (for example), this is most likely your
                 * issue. */
                backend->startOne(meshX, meshY, coreId,
                                  coreIt->second.threadsExpected.size());
            }
            else  /* 3 */
            {
                debug_post(586, 3, hex2str(meshX).c_str(),
                           hex2str(meshY).c_str(), hex2str(coreId).c_str());
                backend->goOne(meshX, meshY, coreId);
            }
        }

        mode = !mode;
    } while (mode);

    /* Good stuff. Now the cores will spin up and send BARRIER messages to the
     * Mothership. */
    pthread_mutex_unlock(&(threading.mutex_backend_api));

    /* 4: Initialise the supervisor device on this Mothership for this
     * application, posting on error. */
    debug_post(583, 1, app->name.c_str());
    if (superdb.init_supervisor(app->name) != 0) Post(523, app->name);
}

/* Starts an application, by queueing BARRIER packets to each thread to be
 * processed by BackendOutputBroker. This packet, when received by the
 * softswitch, commands all of the executors under its command to start. */
void Mothership::run_application(AppInfo* app)
{
    Post(532, int2str(Urank), app->name.c_str());

    app->state = RUNNING;
    send_cnc_packet_to_all(app, P_CNC_BARRIER);

    /* Send "acknowledgement" message to root. */
    PMsg_p acknowledgement;
    acknowledgement.Src(Urank);
    acknowledgement.Key(Q::MSHP, Q::ACK, Q::RUN);
    acknowledgement.Put<std::string>(0, &(app->name));
    acknowledgement.Tgt(pPmap->U.Root);
    queue_mpi_message(&acknowledgement);
}

/* Stops an application, by:
 *
 *  - Changing the application state to STOPPING, which prevents the supervisor
 *    from receiving packets from normal devices.
 *
 *  - Queueing STOP packets to each thread to be processed by
 *    BackendOutputBroker. This packet shuts down the softswitch when
 *    received.
 *
 *  - Calls the exit handler for the supervisor, and reloads it. If the reload
 *    fails, we're not in Kansas anymore Toto. */
void Mothership::stop_application(AppInfo* app)
{
    std::string appName = app->name;
    std::string errorMessage;

    Post(533, int2str(Urank), appName.c_str());
    app->state = STOPPING;
    send_cnc_packet_to_all(app, P_CNC_STOP);
    superdb.exit_supervisor(appName);
    if (!superdb.reload_supervisor(appName, &errorMessage))
    {
        Post(503, appName, errorMessage);
        tell_root_app_is_broken(appName);
        app->state = BROKEN;
    }

    /* On (re)loading the supervisor, provision its API. */
    if (!provision_supervisor_api(appName))
    {
        Post(525, appName);
        tell_root_app_is_broken(appName);
        app->state = BROKEN;
    }
}

/* Sends a CNC packet with a given opcode to each thread in an application. */
void Mothership::send_cnc_packet_to_all(AppInfo* app, uint8_t opcode)
{
    /* Looping variables. */
    std::map<uint32_t, CoreInfo>::iterator coreIt;
    std::set<uint32_t>::iterator threadAddressIt;

    /* Assemble the BARRIER packet. */
    P_Pkt_t packet;

    /* It's a CNC packet for a softswitch. */
    packet.header.swAddr = ((0 << P_SW_MOTHERSHIP_SHIFT)
                            & P_SW_MOTHERSHIP_MASK);
    packet.header.swAddr |= ((1 << P_SW_CNC_SHIFT) & P_SW_CNC_MASK);

    /* It has the barrier opcode. */
    packet.header.swAddr |= ((opcode << P_SW_OPCODE_SHIFT) & P_SW_OPCODE_MASK);

    /* It uses a magic broadcast address to apply to all devices under the
     * control of the Softswitch. */
    packet.header.swAddr |= ((P_ADDR_BROADCAST << P_SW_DEVICE_SHIFT)
                             & P_SW_DEVICE_MASK);

    /* It goes to the __INIT__ pin (NB: Not sure what this means... was used in
     * old Mothership. GMB will find this comment in the review and tell me
     * about it I'm sure. */
    packet.header.pinAddr = ((P_SUP_PIN_INIT << P_HD_TGTPIN_SHIFT)
                             & P_HD_TGTPIN_MASK);

    /* No edge index necessary. */
    packet.header.pinAddr |= ((0 << P_HD_DESTEDGEINDEX_SHIFT)
                              & P_HD_DESTEDGEINDEX_MASK);

    /* For each thread in the task, queue a copy of this packet for that
     * thread. */
    std::vector<P_Addr_Pkt_t> allPackets;
    for (coreIt = app->coreInfos.begin(); coreIt != app->coreInfos.end();
         coreIt++)
        for (threadAddressIt = coreIt->second.threadsExpected.begin();
             threadAddressIt != coreIt->second.threadsExpected.end();
             threadAddressIt++)
        {
            P_Addr_Pkt_t pkt;
            pkt.hwAddr = *threadAddressIt;
            pkt.packet = packet;
            allPackets.push_back(pkt);
        }
    threading.push_backend_out_queue(&allPackets);
}

/* Purges all mention of an application in Mothership datastructures, as well
 * as cores and threads associated with it. */
void Mothership::recall_application(AppInfo* app)
{
    std::string appName = app->name;
    Post(535, int2str(Urank), appName);
    superdb.unload_supervisor(appName);
    appdb.recall_app(app);  /* Clears the name as well! */
    Post(536, int2str(Urank), appName);
}

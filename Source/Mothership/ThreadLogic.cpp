/* This source file defines what the threads do when started via
 * pthread_create. Keep calm; this is just a producer-consumer system. It's all
 * in the Mothership documentation. */

#include "Mothership.h"

void* ThreadComms::mpi_input_broker(void* mothershipArg)
{
    Mothership* mothership = (Mothership*)mothershipArg;
    mothership->mpi_spin();
    debug_print("Mothership: The MPI Input Broker Thread has been told to "
                "exit, and is doing so.\n");
    return mothership;
}

void* ThreadComms::mpi_cnc_resolver(void* mothershipArg)
{
    std::vector<PMsg_p> messages;
    std::vector<PMsg_p>::iterator messageIt;
    unsigned key;
    Mothership* mothership = (Mothership*)mothershipArg;

    #if ORCHESTRATOR_DEBUG
    bool firstSpin = true;
    bool spinningSlowly = false;
    #endif

    /* We spin until we're told to stop. */
    while (!mothership->threading.is_it_time_to_go())
    {
        /* Is there anything in the queue? */
        mothership->threading.pop_MPI_cnc_queue(&messages);

        /* If the queue is empty, chill for a bit before checking again. */
        if (messages.empty())
        {
            #if ORCHESTRATOR_DEBUG
            if (!spinningSlowly)
            {
                spinningSlowly = true;
                if (!firstSpin) mothership->debug_post(485, 2, "MPI Cnc",
                                                       "MPI Cnc Resolver");
                else firstSpin = false;
            }
            #endif
            sleep(1);
            continue;
        }
        #if ORCHESTRATOR_DEBUG
        spinningSlowly = false;
        #endif

        /* Otherwise, handle each message in turn. */
        for (messageIt = messages.begin(); messageIt != messages.end();
             messageIt++)
        {
            key = messageIt->Key();
            if (key == PMsg_p::KEY(Q::APP, Q::SPEC))
                mothership->handle_msg_app_spec(&*messageIt);
            else if (key == PMsg_p::KEY(Q::APP, Q::DIST))
                mothership->handle_msg_app_dist(&*messageIt);
            else if (key == PMsg_p::KEY(Q::APP, Q::SUPD))
                mothership->handle_msg_app_supd(&*messageIt);
            else if (key == PMsg_p::KEY(Q::CMND, Q::RECL))
                mothership->handle_msg_cmnd_recl(&*messageIt);
            else if (key == PMsg_p::KEY(Q::CMND, Q::INIT))
                mothership->handle_msg_cmnd_init(&*messageIt);
            else if (key == PMsg_p::KEY(Q::CMND, Q::RUN))
                mothership->handle_msg_cmnd_run(&*messageIt);
            else if (key == PMsg_p::KEY(Q::CMND, Q::STOP))
                mothership->handle_msg_cmnd_stop(&*messageIt);
            else if (key == PMsg_p::KEY(Q::BEND, Q::CNC))
                mothership->handle_msg_bend_cnc(&*messageIt);
            else if (key == PMsg_p::KEY(Q::DUMP))
                mothership->handle_msg_dump(&*messageIt);
            else
                mothership->Post(407, "MPICncResolver", hex2str(key));
        }

        /* We're done with the extracted messages - throw them away. */
        messages.clear();
    }

    debug_print("Mothership: The MPI Cnc Resolver Thread has been told to "
                "exit, and is doing so.\n");
    return mothership;
}

void* ThreadComms::mpi_application_resolver(void* mothershipArg)
{
    std::vector<PMsg_p> messages;
    std::vector<PMsg_p>::iterator messageIt;
    unsigned key;
    Mothership* mothership = (Mothership*)mothershipArg;

    #if ORCHESTRATOR_DEBUG
    bool firstSpin = true;
    bool spinningSlowly = false;
    #endif

    /* We spin until we're told to stop. */
    while (!mothership->threading.is_it_time_to_go())
    {
        /* Is there anything in the queue? */
        mothership->threading.pop_MPI_app_queue(&messages);

        /* If the queue is empty, chill for a bit before checking again.
         *
         * NB: If we want to add Supervisor OnIdle functionality in future, we
         * should do so in this block. Search tags: OnCompute OnSuperIdle
         * OnSupervisorIdle. */
        if (messages.empty())
        {
            #if ORCHESTRATOR_DEBUG
            if (!spinningSlowly)
            {
                spinningSlowly = true;
                if (!firstSpin)
                    mothership->debug_post(485, 2, "MPI Application",
                                           "MPI Application Resolver");
                else firstSpin = false;
            }
            #endif
            sleep(1);
            continue;
        }
        #if ORCHESTRATOR_DEBUG
        spinningSlowly = false;
        #endif

        /* Otherwise, handle each message in turn. */
        for (messageIt = messages.begin(); messageIt != messages.end();
             messageIt++)
        {
            key = messageIt->Key();
            if (key == PMsg_p::KEY(Q::BEND, Q::SUPR))
                mothership->handle_msg_bend_supr(&*messageIt);
            else if (key == PMsg_p::KEY(Q::PKTS))
                mothership->handle_msg_pkts(&*messageIt);
            else
                mothership->Post(407, "MPIAppResolver", hex2str(key));
        }

        /* We're done with the extracted messages - throw them away. */
        messages.clear();
    }

    debug_print("Mothership: The MPI Application Resolver Thread has been "
                "told to exit, and is doing so.\n");
    return mothership;
}

void* ThreadComms::backend_output_broker(void* mothershipArg)
{
    std::vector<std::pair<uint32_t, P_Pkt_t> >packets;
    std::vector<std::pair<uint32_t, P_Pkt_t> >::iterator packetIt;
    uint32_t numberOfFlitsForThisPacket;
    Mothership* mothership = (Mothership*)mothershipArg;

    #if ORCHESTRATOR_DEBUG
    bool firstSpin = true;
    bool spinningSlowly = false;
    #endif

    /* We spin until we're told to stop. */
    while (!mothership->threading.is_it_time_to_go())
    {
        /* Is there anything in the queue? */
        mothership->threading.pop_backend_out_queue(&packets);

        /* If the queue is empty, chill for a bit before checking again. */
        if (packets.empty())
        {
            #if ORCHESTRATOR_DEBUG
            if (!spinningSlowly)
            {
                spinningSlowly = true;
                if (!firstSpin)
                    mothership->debug_post(485, 2, "Backend Out",
                                           "Backend Output Broker");
                else firstSpin = false;
            }
            #endif
            sleep(1);
            continue;
        }
        #if ORCHESTRATOR_DEBUG
        spinningSlowly = false;
        #endif

        /* Otherwise, blocking-send each packet in turn. */
        for (packetIt = packets.begin(); packetIt != packets.end(); packetIt++)
        {
            mothership->debug_post(484, 1, hex2str(packetIt->first).c_str());

            /* Compute number of flits for this packet. */
            numberOfFlitsForThisPacket = p_hdr_size() >> TinselLogBytesPerFlit;
            if (numberOfFlitsForThisPacket == 0) ++numberOfFlitsForThisPacket;

            /* Send the packet (with that number of flits) */
            pthread_mutex_lock(&(mothership->threading.mutex_backend_api));
            mothership->backend->send(packetIt->first,
                                      numberOfFlitsForThisPacket,
                                      &(packetIt->second), true);
            pthread_mutex_unlock(&(mothership->threading.mutex_backend_api));

        }
    }

    debug_print("Mothership: The Backend Output Broker Thread has been told "
                "to exit, and is doing so.\n");
    return mothership;
}

void* ThreadComms::backend_input_broker(void* mothershipArg)
{
    Mothership* mothership = (Mothership*)mothershipArg;

    /* For dealing with packets and their receipt. */
    std::vector<P_Pkt_t> packets;
    std::vector<P_Pkt_t>::iterator packetIt;
    uint8_t opcode;
    void* receiveBuffer;
    char* dynamicBuffer;  /* For cleanup */
    uint8_t packetAppNumber;
    bool canRecv;

    /* Staging CNC packets, and packets to the supervisor, to be sent. */
    std::string appName;
    std::map<uint8_t, std::string>::iterator appFinder;
    std::vector<P_Pkt_t> cncBuffer;
    std::map<uint8_t, std::vector<P_Pkt_t> > superBuffer;  /* Key = app ID. */
    std::map<uint8_t, std::vector<P_Pkt_t> >::iterator appIt;

    dynamicBuffer = new char[p_pkt_size()];
    receiveBuffer = static_cast<void*>(dynamicBuffer);

    /* We spin until we're told to stop. */
    while (!mothership->threading.is_it_time_to_go())
    {
        /* Attempt to receive packets from the backend and push them into the
         * backend-input queue. */
        pthread_mutex_lock(&(mothership->threading.mutex_backend_api));
        canRecv = mothership->backend->canRecv();
        if (canRecv)
        {
            mothership->backend->recv(receiveBuffer);
            pthread_mutex_unlock(&(mothership->threading.mutex_backend_api));

            /* Dump the packet in a hacky way.
            P_Pkt_t* convenience = static_cast<P_Pkt_t*>(receiveBuffer);
            printf("=== We got a packet! swAddr: %u, pinAddr: %u, payload: ",
                   convenience->header.swAddr,
                   convenience->header.pinAddr);
            for (unsigned payloadIndex = 0;
                 payloadIndex < P_PKT_MAX_SIZE-sizeof(P_Pkt_Hdr_t);
                 payloadIndex++)
                printf("%u ", convenience->payload[payloadIndex]);
            printf("\n"); */

            mothership->threading.push_backend_in_queue(
                *(static_cast<P_Pkt_t*>(receiveBuffer)));

            /* Once we've received a packet, immediatly check again for another
             * (while also respecting the exit condition). */
            continue;
        }

        /* Nothing to receive, so drain the queue and handle all packets
         * therein. */
        else
        {
            pthread_mutex_unlock(&(mothership->threading.mutex_backend_api));

            /* Shortcut - if there's nothing to do. */
            if (!(mothership->threading.pop_backend_in_queue(&packets)))
                continue;

            for (packetIt = packets.begin(); packetIt != packets.end();
                 packetIt++)
            {
                /* If we've received a packet bound for an external, write a
                 * not-yet-implemented message. Such a packet has the
                 * isMothership bit set and the isCnc bit unset in its software
                 * address field. */
                if (packetIt->header.swAddr & P_SW_MOTHERSHIP_MASK and
                    !(packetIt->header.swAddr & P_SW_CNC_MASK))
                {
                    /* NB: But for now, we only warn, because we have no
                       external support. */
                    mothership->Post(418, hex2str(packetIt->header.swAddr));
                    continue;
                }

                /* If the packet is destined for the Mothership, determine
                 * whether it is a special CNC packet (driven by its opcode),
                 * or a packet for the supervisor to parse. */
                if (packetIt->header.swAddr & P_SW_MOTHERSHIP_MASK and
                    packetIt->header.swAddr & P_SW_CNC_MASK)
                {
                    opcode = (packetIt->header.swAddr & P_SW_OPCODE_MASK)
                        >> P_SW_OPCODE_SHIFT;

                    /* It's a CNC packet (brrr). Wrap as message and add to the
                     * buffer of CNC messages to push to the queue (we'll do
                     * them all at once when we've drained our consumer
                     * queue) */
                    if (opcode == P_CNC_INSTR or
                        opcode == P_CNC_LOG or
                        opcode == P_CNC_BARRIER or
                        opcode == P_CNC_STOP or
                        opcode == P_CNC_KILL)
                    {
                        cncBuffer.push_back(*packetIt);
                    }

                    /* It's for a supervisor device. Decode the application
                     * name from the task field, and send the packet (now) with
                     * this application name as a message to the MPI entrypoint
                     * (MPISpinner). */
                    else
                    {
                        /* Grab the task ID. */
                        packetAppNumber =
                            (packetIt->header.swAddr & P_SW_TASK_MASK) >>
                            P_SW_TASK_SHIFT;

                        /* Stage the packet for sending once we've drained the
                         * queue. */
                        superBuffer[packetAppNumber].push_back(*packetIt);
                    }
                }

                /* Otherwise, there's been some kind of mistake (Motherships
                 * should not be sent packets when the isMothership bit in the
                 * software address is unset. Ignore the packet, but warn the
                 * operator. */
                else mothership->Post(417, hex2str(packetIt->header.swAddr));
            }

            /* After draining our queue, push all of the CNC packets we've
             * received to the CNC queue as a single message. */
            if (!(cncBuffer.empty()))
            {
                mothership->debug_post(483, 1,
                                       uint2str(cncBuffer.size()).c_str());
                PMsg_p message;
                message.Key(Q::BEND, Q::CNC);
                message.Put<std::vector<P_Pkt_t> >(0, &cncBuffer);
                mothership->threading.push_MPI_cnc_queue(message);
                cncBuffer.clear();
            }

            /* Likewise, after draining the queue, push all of the packets (now
             * packaged as messages) destined for the supervisor over MPI. */
            if (!(superBuffer.empty()))
            {
                for (appIt = superBuffer.begin(); appIt != superBuffer.end();
                     appIt++)
                {
                    /* Construct a message that holds all of the packets for
                     * this supervisor. */
                    PMsg_p message;
                    message.Key(Q::BEND, Q::SUPR);
                    message.Src(mothership->Urank);
                    message.Tgt(mothership->Urank);

                    /* Get the app name using the ID, complaining if we can't
                     * find it. */
                    appFinder = mothership->appdb.numberToApp.find(
                        appIt->first);
                    if (appFinder == mothership->appdb.numberToApp.end())
                    {
                        mothership->Post(420, hex2str(appIt->first));
                    }

                    /* Put the app name. */
                    message.Put<std::string>(0, &(appFinder->second));

                    /* Put the packets themselves. */
                    message.Put<std::vector<P_Pkt_t> >(1, &(appIt->second));

                    /* Out it goes. */
                    mothership->debug_post(
                        482, 2, appFinder->second.c_str(),
                        uint2str(appIt->second.size()).c_str());
                    mothership->queue_mpi_message(message);
                }

                /* We've sent them all now, so we're done here. */
                superBuffer.clear();
            }
        }
    }

    delete[] dynamicBuffer;
    debug_print("Mothership: The Backend Input Broker Thread has been told to "
                "exit, and is doing so.\n");
    return mothership;
}


void* ThreadComms::debug_input_broker(void* mothershipArg)
{
    /* Packet sender data and payload */
    uint32_t brdX, brdY, coreId, threadId;
    uint8_t payload;

    /* Packet queueing */
    P_Debug_Pkt_t debugPacket;
    std::vector<P_Debug_Pkt_t> debugPackets;
    std::vector<P_Debug_Pkt_t>::iterator packetIt;

    Mothership* mothership = (Mothership*)mothershipArg;
    DebugLink* debug = mothership->backend->debugLink;

    #if ORCHESTRATOR_DEBUG
    bool firstSpin = true;
    bool spinningSlowly = false;
    #endif

    /* We spin until we're told to stop. */
    while (!mothership->threading.is_it_time_to_go())
    {
        /* Drain the debug channel as quickly as possible, until there's
         * nothing left. */
        while (debug->canGet())
        {
            debug->get(&brdX, &brdY, &coreId, &threadId, &payload);
            debugPacket.origin = mothership->backend->toAddr(
                brdX, brdY, coreId, threadId);
            debugPacket.payload = payload;
            mothership->threading.push_debug_in_queue(debugPacket);
        }

        /* If the queue is empty, it means we've not received anything on the
         * debug channel. We chill, because debug output is not time
         * critical. */
        if (!(mothership->threading.pop_debug_in_queue(&debugPackets)))
        {
            #if ORCHESTRATOR_DEBUG
            if (!spinningSlowly)
            {
                spinningSlowly = true;
                if (!firstSpin) mothership->debug_post(485, 2, "Debug Input",
                                                       "Debug Input Broker");
                else firstSpin = false;
            }
            #endif

            sleep(1);
            continue;
        }

        /* Otherwise, post them all one at a time. */
        else
        {
            #if ORCHESTRATOR_DEBUG
            spinningSlowly = false;
            #endif

            for (packetIt = debugPackets.begin();
                 packetIt != debugPackets.end(); packetIt++)
            {
                mothership->Post(421, hex2str(packetIt->origin),
                                 hex2str(packetIt->payload));
            }

            debugPackets.clear();
        }
    }

    debug_print("Mothership: The Debug Input Broker Thread has been told to "
                "exit, and is doing so.\n");
    return mothership;
}

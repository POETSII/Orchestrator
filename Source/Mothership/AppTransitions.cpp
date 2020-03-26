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

/* Starts an application, by queueing BARRIER packets to each thread to be
 * processed by BackendOutputBroker. This packet, when received by the
 * softswitch, commands all of the executors under its command to start. */
void Mothership::run_application(AppInfo* app)
{
    /* Looping variables. */
    std::map<uint32_t, CoreInfo>::iterator coreIt;
    std::set<uint32_t>::iterator threadAddressIt;

    /* Assemble the BARRIER packet. */
    P_Pkt_t packet;

    /* It's a CNC packet for a softswitch. */
    packet.header.swAddr = ((0 << P_SW_MOTHERSHIP_SHIFT)
                            & P_SW_MOTHERSHIP_MASK);
    packet.header.swAddr |= ((1 << P_SW_CNC_SHIFT)
                             & P_SW_CNC_MASK);

    /* It has the barrier opcode. */
    packet.header.swAddr |= ((P_CNC_BARRIER << P_SW_OPCODE_SHIFT)
                             & P_SW_OPCODE_MASK);

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
    std::vector<std::pair<uint32_t, P_Pkt_t> > allPackets;
    for (coreIt = app->coreInfos.begin(); coreIt != app->coreInfos.end();
         coreIt++)
        for (threadAddressIt = coreIt->second.threadsExpected.begin();
             threadAddressIt != coreIt->second.threadsExpected.end();
             threadAddressIt++)
        {
            allPackets.push_back(std::pair<uint32_t, P_Pkt_t>
                                 (*threadAddressIt, packet));
        }
    threading.push_backend_out_queue(&allPackets);
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

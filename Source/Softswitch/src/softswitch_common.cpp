#include "softswitch_common.h"
#include "tinsel.h"
#include <cstring>
#include "stdint.h"


void softswitch_init(ThreadCtxt_t* ThreadContext)
{
    // Allocate Tinsel receive slots.
    for (uint32_t i=P_RXSLOT_START; i < (1<<TinselLogMsgsPerThread); i++)
    {
        tinselAlloc(tinselSlot(i)); // allocate receive slots.
    }

#ifndef DISABLE_SOFTSWITCH_INSTRUMENTATION
    // Initialise instrumentation
    ThreadContext->lastCycles = 0;
    ThreadContext->pendCycles = 0;
    ThreadContext->txCount = 0;
    ThreadContext->superCount = 0;
    ThreadContext->rxCount = 0;
    ThreadContext->txHandlerCount = 0;
    ThreadContext->rxHandlerCount = 0;
    ThreadContext->idleCount = 0;
    ThreadContext->idleHandlerCount = 0;
    ThreadContext->blockCount = 0;
    ThreadContext->cycleIdx = 0;

 #if TinselEnablePerfCount == true
    // Initialise the optional Tinsel instrumentation
    ThreadContext->lastmissCount = 0;
    ThreadContext->lasthitCount = 0;
    ThreadContext->lastwritebackCount = 0;
    ThreadContext->lastCPUIdleCount = 0;
 #endif
#endif

    // Initialise device types
    for (uint32_t deviceType = 0;
            deviceType < ThreadContext->numDevTyps;
            deviceType++)
    {
        deviceType_init(deviceType, ThreadContext);
    }

    // Initialise devices
    for (uint32_t device = 0; device < ThreadContext->numDevInsts; device++)
    {
        device_init(&ThreadContext->devInsts[device], ThreadContext);
    }
}

// <!> You know it's a hack when the "pragma GCC" comes out.
#pragma GCC push_options
#pragma GCC optimize ("O0")
void softswitch_delay(){for (uint32_t i=0; i<500; i++);}
#pragma GCC pop_options


/*------------------------------------------------------------------------------
 * softswitch_barrier: Block until told to continue by the mothership
 *----------------------------------------------------------------------------*/
void softswitch_barrier(ThreadCtxt_t* ThreadContext)
{
    // Create RX buffer pointer and get Tinsel Slots
    volatile void *recv_buf = PNULL;
    volatile void *send_buf = tinselSlot(P_PKT_SLOT);     // Send slot

    // first phase of barrier: set up a standard packet to send to the supervisor
    volatile P_Pkt_Hdr_t* hdr = static_cast<volatile P_Pkt_Hdr_t*>(send_buf); // Header

    // block until we can send it,
    while (!tinselCanSend());

    // Set a Supervisor header with the correct Opcode.
    hdr->swAddr = P_SW_MOTHERSHIP_MASK | P_SW_CNC_MASK;
    hdr->swAddr |= ((P_CNC_BARRIER << P_SW_OPCODE_SHIFT) & P_SW_OPCODE_MASK);
    hdr->pinAddr = tinselId();          // usurp Pin Addr for the source HW addr

    // and then issue the packet indicating this thread's startup is complete.
    tinselSetLen((p_hdr_size() - 1) >> TinselLogBytesPerFlit);
    tinselSend(tinselHostId(), send_buf);

    // second phase of barrier: wait for the supervisor's response
    ThreadContext->ctlEnd = 1;
    while (ThreadContext->ctlEnd)
    {
        // by blocking awaiting a receive.
        while (!tinselCanRecv());
        recv_buf = tinselRecv();

        volatile P_Pkt_t* rcv_pkt = static_cast<volatile P_Pkt_t*>(recv_buf);
        // look for barrier packet
        if (((rcv_pkt->header.swAddr & P_SW_OPCODE_MASK) >> P_SW_OPCODE_SHIFT)
                == P_CNC_BARRIER)
        {
            // *Debug: send packet out to show we have passed the barrier*
            // softswitch_alive(send_buf);
            // and once it's been received, process it as a startup packet
            
            ThreadContext->ctlEnd = 0;
            
            // Sleep for X to allow everything to start
        }
        tinselAlloc(recv_buf);
    }
    softswitch_delay();     // Delay to let other softswitches pass the barrier
}
//------------------------------------------------------------------------------



/*------------------------------------------------------------------------------
 * Two utility inline functions to reduce code duplication when configuring
 * loop execution order.
 *----------------------------------------------------------------------------*/
inline void receiveInline(ThreadCtxt_t* ThreadContext, volatile void* recvBuffer)
{
    recvBuffer=tinselRecv();
    softswitch_onReceive(ThreadContext, recvBuffer); // decode the receive and handle
    tinselAlloc(recvBuffer); // return control of the receive buffer to the hardware
}

inline void sendInline(ThreadCtxt_t* ThreadContext, volatile void* sendBuffer)
{
    if (!tinselCanSend())
    {
        // Channel is blocked. Wait until we have something to do.
        ThreadContext->blockCount++;    // Increment Softswitch block count
        tinselWaitUntil(TINSEL_CAN_SEND | TINSEL_CAN_RECV);
    }
    else
    {
        // Let's send something.
        softswitch_onSend(ThreadContext, sendBuffer);
    }
}
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * softswitch_loop: Where the work happens
 *----------------------------------------------------------------------------*/
void softswitch_loop(ThreadCtxt_t* ThreadContext)
{
    // Create RX buffer pointer and get Tinsel Slots
    volatile void *recvBuffer = PNULL;
    volatile void *sendBuffer = tinselSlot(P_PKT_SLOT);     // Send slot
    volatile void *superBuffer = tinselSlot(P_SUPPKT_SLOT); // Supervisor send slot

#ifndef DISABLE_SOFTSWITCH_INSTRUMENTATION
    uint32_t cycles = tinselCycleCount();		// cycle counter is per-core
    ThreadContext->lastCycles = cycles;         // save initial cycle count
 #if TinselEnablePerfCount == true
    // Save initial extended instrumentation counts.
    ThreadContext->lastmissCount = tinselMissCount();
    ThreadContext->lasthitCount = tinselHitCount();
    ThreadContext->lastwritebackCount = tinselWritebackCount();
    ThreadContext->lastCPUIdleCount = tinselCPUIdleCount();
 #endif
#endif

    while (!ThreadContext->ctlEnd)
    {
    #ifndef DISABLE_SOFTSWITCH_INSTRUMENTATION
        cycles = tinselCycleCount();
        if((cycles - ThreadContext->lastCycles) > P_INSTR_INTERVAL)
        {   // Trigger a packet to supervisor.
            ThreadContext->pendCycles = 1;
        }
    #endif

/* Support reordering of the execution order to prioritise the sending of
 * instrumentation. This is useful where an application expects to be inundated
 * with receives.
 *
 * This could be extended to allow sending to be prioritised over receive (and
 * instrumentation to be prioritised over that), however when I attempted this,
 * the examples broke so more work is needed to implement.
 */
#if !defined DISABLE_SOFTSWITCH_INSTRUMENTATION \
      && defined SOFTSWITCH_PRIORITISE_INSTRUMENTATION
// Prioritise Instrumentation: Send Instrumentation, RX, TX
        if(ThreadContext->pendCycles && tinselCanSend())
        {   // Time to send Instrumentation
            softswitch_instrumentation(ThreadContext, superBuffer);
        }
        else if (tinselCanRecv())
        {   // Something to receive
            receiveInline(ThreadContext, recvBuffer);
        }
        else if (ThreadContext->rtsStart != ThreadContext->rtsEnd)
        {   // Something to send
            sendInline(ThreadContext, sendBuffer);
        }

#else
// Default order: RX, Send Instrumentation, TX
        if (tinselCanRecv())
        {   // Something to receive
            receiveInline(ThreadContext, recvBuffer);
        }
    #ifndef DISABLE_SOFTSWITCH_INSTRUMENTATION
        else if(ThreadContext->pendCycles && tinselCanSend())
        {   // Time to send Instrumentation
            softswitch_instrumentation(ThreadContext, superBuffer);
        }
    #endif
        else if (ThreadContext->rtsStart != ThreadContext->rtsEnd)
        {   // Something to send
            sendInline(ThreadContext, sendBuffer);
        }
#endif


        // Nothing to RX, nothing to TX: iterate through all devices until
        // something happens or all OnComputes have returned 0.
        //softswitch_onIdle(ThreadContext);
        else if(!softswitch_onIdle(ThreadContext))
        {
            tinselWaitUntil(TINSEL_CAN_RECV);
        }
    }
}
//------------------------------------------------------------------------------


/*------------------------------------------------------------------------------
 * softswitch_finalise: Gracefully tear everything down
 *----------------------------------------------------------------------------*/
void softswitch_finalise(ThreadCtxt_t* ThreadContext)
{
    // Loop through everything in the RTS buffer and clear it
    uint32_t maxIdx = ThreadContext->rtsBuffSize;   // # devices we are looping through
    while(ThreadContext->rtsStart != ThreadContext->rtsEnd)
    {
        uint32_t rtsStart = ThreadContext->rtsStart;

        ThreadContext->rtsBuf[rtsStart]->idxEdges = 0;
        ThreadContext->rtsBuf[rtsStart]->sendPending = 0;
        ThreadContext->rtsBuf[rtsStart] = PNULL;

        ThreadContext->rtsStart++;
        if(ThreadContext->rtsStart == maxIdx)
        {
            ThreadContext->rtsStart = 0;
        }
    }

    // free mailbox slots
    for (uint32_t i = 0; i < (1<<TinselLogMsgsPerThread); i++) tinselAlloc(tinselSlot(i));
}
//------------------------------------------------------------------------------





/*==============================================================================
 * Softswitch Initialisation.
 *============================================================================*/
/*------------------------------------------------------------------------------
 * device_init
 *
 * Initialises the devices and their pins.
 *----------------------------------------------------------------------------*/
inline void device_init(devInst_t* device, ThreadCtxt_t* ThreadContext)
{
    device->thread = ThreadContext;     // Set the device's thread back pointer

    for (uint32_t out = 0; out < device->numOutputs; out++)
    {
        outPin_t* o_pin = &device->outputPins[out];

        o_pin->device = device;
        for (uint32_t edge = 0; edge < o_pin->numEdges; edge++)
        {
            outEdge_t* i_tgt = &o_pin->outEdges[edge];
            i_tgt->pin = o_pin;
        }
    }

    for (uint32_t pkt_typ = 0; pkt_typ < device->numInputs; pkt_typ++)
    {
        inPin_t* i_pin = &device->inputPins[pkt_typ];

        i_pin->device = device;
        for (uint32_t src = 0; src < i_pin->numEdges; src++)
        {
            inEdge_t* i_src = &i_pin->inEdges[src];
            i_src->pin = i_pin;
        }
    }
    
    // Execute the OnInit handler
    if(device->devType->OnInit_Handler(ThreadContext->properties, device))
    {
        softswitch_onRTS(ThreadContext, device);  // Call RTS for device
    }
}
//------------------------------------------------------------------------------


inline void deviceType_init(uint32_t deviceType_num, ThreadCtxt_t* ThreadContext)
{
    devTyp_t* deviceType OS_ATTRIBUTE_UNUSED= &ThreadContext->devTyps[deviceType_num];
    OS_PRAGMA_UNUSED(deviceType)
    /*
    Handler addresses ought to reside in instruction memory and thus remain valid: to be seen.
    deviceType->RTS_Handler = RTS_Handlers[ThreadContext->threadID.PThread][deviceType_num]; // RTS_Handlers is a table that must be built by the Orchestrator.
    deviceType->OnIdle_Handler = OnIdle_Handlers[ThreadContext->threadID.PThread][deviceType_num];
    deviceType->OnCtl_Handler = OnCtl_Handlers[ThreadContext->threadID.PThread][deviceType_num]; // OnCtl_Handler should amongst other things handle monitoring.
    */
    //deviceType->outputTypes = offset_ptr<out_pintyp_t*>(ThreadContext, deviceType->outputTypes);
    //deviceType->inputTypes = offset_ptr<in_pintyp_t*>(ThreadContext, deviceType->inputTypes);
    // for (uint32_t o = 0; o < deviceType->numOutputTypes; o++) outputPinType_init(o, deviceType_num, ThreadContext);
    // for (uint32_t i = 0; i < deviceType->numInputTypes; i++) inputPinType_init(i, deviceType_num, ThreadContext);
}



/*==============================================================================
 * Softswitch handlers.
 *============================================================================*/
 /*------------------------------------------------------------------------------
 * softswitch_instrumentation: Sends instrumentation to the Mothership
 *----------------------------------------------------------------------------*/
inline void softswitch_instrumentation(ThreadCtxt_t* ThreadContext, volatile void* send_buf)
{
    uint32_t cycles = tinselCycleCount();

    P_Pkt_t* pkt = static_cast<P_Pkt_t*>(const_cast<void*>(send_buf));
    P_Pkt_Hdr_t* hdr = &(pkt->header);
    P_Instr_Pkt_Pyld_t* pyld = reinterpret_cast<P_Instr_Pkt_Pyld_t*>(pkt->payload);

    // Set the Header
    hdr->swAddr = P_SW_MOTHERSHIP_MASK | P_SW_CNC_MASK;
    hdr->swAddr |= ((P_CNC_INSTR << P_SW_OPCODE_SHIFT) & P_SW_OPCODE_MASK);
    hdr->pinAddr = tinselId(); // usurp Pin Addr for the source HW addr


    // Load the Message
    pyld->cIDX = ThreadContext->cycleIdx++;
    pyld->cycles = cycles - ThreadContext->lastCycles;
    pyld->txCnt = ThreadContext->txCount;
    pyld->supCnt = ThreadContext->superCount;
    pyld->rxCnt = ThreadContext->rxCount;
    pyld->txHanCnt = ThreadContext->txHandlerCount;
    pyld->rxHanCnt = ThreadContext->rxHandlerCount;
    pyld->idleCnt = ThreadContext->idleCount;
    pyld->blockCnt = ThreadContext->blockCount;
    pyld->idleHanCnt = ThreadContext->idleHandlerCount;

#if TinselEnablePerfCount == true
    // Optional Tinsel instrumentation
    uint32_t missCount = tinselMissCount();
    uint32_t hitCount = tinselHitCount();
    uint32_t writebackCount = tinselWritebackCount();
    uint32_t CPUIdleCount = tinselCPUIdleCount();

    pyld->missCount = missCount - ThreadContext->lastmissCount;
    pyld->hitCount = hitCount - ThreadContext->lasthitCount;
    pyld->writebackCount = writebackCount - ThreadContext->lastwritebackCount;
    pyld->CPUIdleCount = CPUIdleCount - ThreadContext->lastCPUIdleCount;
#endif

    // Send it
    uint32_t len = p_hdr_size() + p_instrpkt_pyld_size;
    tinselSetLen((len - 1) >> TinselLogBytesPerFlit);    // Set the packet length
    tinselSend(tinselHostId(), send_buf); // Send it


    // Reset fields.
    ThreadContext->lastCycles = cycles;
    ThreadContext->pendCycles = 0;
    ThreadContext->txCount = 0;
    ThreadContext->superCount = 0;
    ThreadContext->rxCount = 0;
    ThreadContext->txHandlerCount = 0;
    ThreadContext->rxHandlerCount = 0;
    ThreadContext->idleCount = 0;
    ThreadContext->blockCount = 0;
    ThreadContext->idleHandlerCount = 0;

#if TinselEnablePerfCount == true
    ThreadContext->lastmissCount = missCount;
    ThreadContext->lasthitCount = hitCount;
    ThreadContext->lastwritebackCount = writebackCount;
    ThreadContext->lastCPUIdleCount = CPUIdleCount;
#endif

    // Clear the pending packet
    ThreadContext->pendCycles = 0;
}


/*------------------------------------------------------------------------------
 * softswitch_onSend: Executed any time there is an entry in the RTS list.
 *
 * tinselCanSend() has already been passed prior to this method being called,
 * so we can futz with send_buf as much as we want and send when ready.
 *----------------------------------------------------------------------------*/
inline uint32_t softswitch_onSend(ThreadCtxt_t* ThreadContext, volatile void* send_buf)
{
    outPin_t* pin = ThreadContext->rtsBuf[ThreadContext->rtsStart]; // Get the next pin

    // Pointers of convenience
    devInst_t* device = pin->device;                                // Get the Device
    const outEdge_t* target = &pin->outEdges[pin->idxEdges];        // Get the edge
    volatile char* buf = static_cast<volatile char*>(send_buf);     // Send Buffer
    volatile P_Pkt_Hdr_t* hdr = static_cast<volatile P_Pkt_Hdr_t*>(send_buf); // Header

    size_t hdrSize = p_hdr_size(); //Size of the header.

    if(pin->numEdges > 0)     // Sanity check: make sure the pin has edges
    {
        //--------------------------------------------------------------------------
        // First target, need to run pin's OnSend.
        //--------------------------------------------------------------------------
        if(pin->idxEdges == 0)
        {
            char* pkt = const_cast<char*>(buf)+hdrSize; // Pointer to the packet, after headers, etc.
            pin->pinType->Send_Handler(ThreadContext->properties, device, pkt);
            ThreadContext->txHandlerCount++;         // Increment SendHandler count
        }
        //--------------------------------------------------------------------------

        //--------------------------------------------------------------------------
        // Set the addresses and send the packet
        //--------------------------------------------------------------------------
        hdr->swAddr = target->swAddr;
        hdr->pinAddr = target->pinAddr;

        uint32_t flt = ((hdrSize + pin->pinType->sz_pkt)-1) >> TinselLogBytesPerFlit;
        tinselSetLen((flt < TinselMaxFlitsPerMsg)? flt : (TinselMaxFlitsPerMsg-1));    // Set the packet length

        if(hdr->swAddr & P_SW_MOTHERSHIP_MASK)
        {   // Message to the Supervisor or External (this goes via the Supervisor)
            tinselSend(tinselHostId(), send_buf);   // Goes to the tinselHost
            ThreadContext->superCount++;         // Increment Supervisor Pkt count
        }
        else
        {   // Message to another device.
            tinselSend(target->hwAddr, send_buf);   // Goes to the target's hwAddr
            ThreadContext->txCount++;            // Increment normal Pkt count

            /*
            //TEMPORARY DEBUGGING BODGE: echo packet to supervisor
            volatile uint32_t* last = static_cast<volatile uint32_t*>(send_buf); // TEMPORARY BODGE:

            while(!tinselCanSend());
            tinselSetLen(TinselMaxFlitsPerMsg-1);    // Set the packet length
            last[11] = 0x00ABBA00;
            last[12] = target->hwAddr;
            last[13] = target->swAddr;
            last[14] = tinselId();
            last[15] = device->deviceID;

            tinselSetLen(TinselMaxFlitsPerMsg-1);
            tinselSend(tinselHostId(), send_buf);
            */
        }
        //--------------------------------------------------------------------------
    }

    pin->idxEdges++;     // Increment the target index.

    if(pin->idxEdges >= pin->numEdges)  // Reached the end of the send list.
    {
        pin->idxEdges = 0;       // Reset index,
        pin->sendPending = 0;   // Reset pending

        // Move the circular RTS buffer index
        ThreadContext->rtsStart++;
        if(ThreadContext->rtsStart == ThreadContext->rtsBuffSize)
        {
            ThreadContext->rtsStart = 0;
        }

        softswitch_onRTS(ThreadContext, device); // Run the Device's RTS handler. This could be conditional.
    }

    return pin->idxEdges;
}
//------------------------------------------------------------------------------


/*------------------------------------------------------------------------------
 * softswitch_onReceive: Executed any time a packet is received.
 *----------------------------------------------------------------------------*/
void softswitch_onReceive(ThreadCtxt_t* ThreadContext, volatile void* recv_buf)
{
    devInst_t* recvDevBegin;
    devInst_t* recvDevEnd;

    ThreadContext->rxCount++;                  // Increment received packet count

    // Grab the packet header
    volatile P_Pkt_Hdr_t* recvHdr = static_cast<volatile P_Pkt_Hdr_t*>(recv_buf);

    // Decode the header
    uint32_t devAdr = (recvHdr->swAddr & P_SW_DEVICE_MASK) >> P_SW_DEVICE_SHIFT;
    uint8_t opcode = (recvHdr->swAddr & P_SW_OPCODE_MASK) >> P_SW_OPCODE_SHIFT;
    uint8_t pinIdx = (recvHdr->pinAddr & P_HD_TGTPIN_MASK) >> P_HD_TGTPIN_SHIFT;
    uint32_t edgeIdx = (recvHdr->pinAddr & P_HD_DESTEDGEINDEX_MASK)
                        >> P_HD_DESTEDGEINDEX_SHIFT;

    // Decode the destination address
    if(devAdr == P_ADDR_BROADCAST)
    {   // Message is a broadcast to all devices.
        recvDevBegin = ThreadContext->devInsts;
        recvDevEnd = ThreadContext->devInsts + ThreadContext->numDevInsts;
    }
    else if (devAdr < ThreadContext->numDevInsts)
    {   // Message is for a single device in range.
        recvDevBegin = &ThreadContext->devInsts[devAdr];
        recvDevEnd = recvDevBegin + 1;
    }
    else
    {
        handler_log(5, "dIDX OOR %d", devAdr);
        return;    // Message target is out of range. //TODO: - log/flag
    }


    if((recvHdr->swAddr & P_SW_CNC_MASK) && (opcode > P_CNC_MAX_USER))
    {   // Check for reserved Opcodes
        switch(opcode)
        {
            case P_CNC_STOP:    // Stop packet ends the simulation
                ThreadContext->ctlEnd = 1;
                break;
#ifndef DISABLE_SOFTSWITCH_INSTRUMENTATION
            case P_CNC_INSTR:   // Request Instrumentation from the Softswitch
                ThreadContext->pendCycles = 1;
                break;
#endif
            default:            // Unused reserved Opcode - log it.
                                handler_log(5, "BAD-OP %d", opcode);

        }
        return;
    }

    // Loop through each target device (1 unless a broadcast packet)
    for (devInst_t* device = recvDevBegin; device != recvDevEnd; device++)
    {
        if(pinIdx >= device->numInputs)
        {   // Sanity check the pin index
            handler_log(5, "pIDX OOR %d %d", device, pinIdx);
            continue;               //TODO: - log/flag
        }
        inPin_t* pin = &device->inputPins[pinIdx];    // Get the pin

        ThreadContext->rxHandlerCount++;     // Increment received handler count


        // Supervisor/CNC/Control Message
        if(recvHdr->swAddr & P_SW_CNC_MASK)
        {  // Trigger OnCtl. Strip volatile at this point.
            device->devType->OnCtl_Handler(ThreadContext, device, opcode,
                                               static_cast<const uint8_t*>(
                                               const_cast<const void*>(recv_buf)
                                               )+p_hdr_size());
        }
        else
        {   // Handle as a normal packet
            if(edgeIdx >= pin->numEdges)
            {   // Sanity check the Edge Index
                handler_log(5, "eIDX OOR %d %d %d", device, pinIdx, edgeIdx);
                continue;               //TODO: - log/flag
            }
            pin->pinType->Recv_handler(ThreadContext->properties, device,
                                        &pin->inEdges[edgeIdx],
                                        static_cast<const uint8_t*>(
                                        const_cast<const void*>(recv_buf)
                                        )+p_hdr_size());
        }
        softswitch_onRTS(ThreadContext, device);    // Run OnRTS for the device
    }
}
//------------------------------------------------------------------------------


inline bool softswitch_onIdle(ThreadCtxt_t* ThreadContext)
{
    uint32_t maxIdx = ThreadContext->numDevInsts - 1;   // # devices we are looping through
    //devInst_t* devices = ThreadContext->devInsts;       // Shortcut to the devices array

    uint32_t idleStart = ThreadContext->idleStart;      // pickup where we left off
    uint32_t idleIdx = idleStart;
    bool notIdle = false;                           // Return val

    ThreadContext->idleCount++;                  // Increment Softswitch Idle count

    do {        // Do-while for exit-controlled loop.
        //bool rtsFlagged = false;
        devInst_t* device = &ThreadContext->devInsts[idleIdx];

        if (tinselCanRecv())     // Something to RX, bail!
        {
            notIdle = true;       // return 1 as "something" has happened
            break;
        }

        if (device->devType->OnIdle_Handler(ThreadContext->properties, device))
        {
            notIdle = true;                                         // Something interesting happened
            //rtsFlagged |= softswitch_onRTS(ThreadContext, device);  // Call RTS for device
            softswitch_onRTS(ThreadContext, device);  // Call RTS for device
        }
        ThreadContext->idleHandlerCount++;       // Increment Idle Handler count

        if(idleIdx < maxIdx) ++idleIdx;     // Increment the index,
        else idleIdx = 0;                   // or reset it to wrap.

        //if (rtsFlagged) break;              // RTS has been flagged - bail!
    } while (idleIdx != idleStart);     // Exit if we have serviced all devices.

    ThreadContext->idleStart = idleIdx;    // save our position
    return notIdle;
}


inline uint32_t softswitch_onRTS(ThreadCtxt_t* ThreadContext, devInst_t* device)
{
    // Essentially:
    //        Run Device's OnRts
    //         Check pin mask for each pin
    //        If pin does not have pending send, add to buffer


    // Creating multiple bit-fields for RTS depending on a config flag.
    // 32 = 1 bit-field, 64 = 2 bit-fields, etc. etc.
    uint32_t rts = 0; // flags for new ready-to-send pins
    uint32_t pin;

    // Run device's RTS handler.
    device->devType->RTS_Handler(ThreadContext->properties, device, &rts);
    if (rts)
    {
        for (pin = 0; pin < device->numOutputs; pin++)       // Loop through each pin
        {
            if (rts & (0x1 << pin))              // If the pin is marked active,
            {
                // Get a pointer to the active pin
                outPin_t* output_pin = &device->outputPins[pin];

                //If the pin has edges and is not already pending,
                if(output_pin->numEdges && output_pin->sendPending == 0)
                {
                    // Flag that pin is pending
                    output_pin->sendPending = 1;

                    // Add pin to RTS list and update end
                    ThreadContext->rtsBuf[ThreadContext->rtsEnd] = output_pin;

                    ThreadContext->rtsEnd++;
                    if(ThreadContext->rtsEnd == ThreadContext->rtsBuffSize)
                    {
                        ThreadContext->rtsEnd = 0;
                    }
                }
            }
        }
    }
    return rts;
}

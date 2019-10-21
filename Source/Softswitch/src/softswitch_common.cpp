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
    
#ifdef SOFTSWITCH_INSTRUMENTATION
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


/*------------------------------------------------------------------------------
 * softswitch_barrier: Block until told to continue by the mothership
 *----------------------------------------------------------------------------*/
void softswitch_barrier(ThreadCtxt_t* ThreadContext)
{
    // Create RX buffer pointer and get Tinsel Slots
    volatile void *recv_buf = PNULL;
    volatile void *send_buf = tinselSlot(P_MSG_SLOT);     // Send slot
    
    // first phase of barrier: set up a standard message to send to the supervisor
    volatile P_Msg_Hdr_t* hdr = static_cast<volatile P_Msg_Hdr_t*>(send_buf); // Header
    
    
    // Set a Supervisor header with the correct Opcode.
    hdr->swAddr = P_SW_MOTHERSHIP_MASK | P_SW_CNC_MASK;
    hdr->swAddr |= ((P_CNC_BARRIER << P_SW_OPCODE_SHIFT) & P_SW_OPCODE_MASK);
    hdr->pinAddr = tinselId();          // usurp Pin Addr for the source HW addr
    
    // block until we can send it,
    while (!tinselCanSend());
    
    // and then issue the message indicating this thread's startup is complete.
    tinselSetLen(p_hdr_size());
    tinselSend(tinselHostId(), send_buf);
    
    // second phase of barrier: wait for the supervisor's response
    ThreadContext->ctlEnd = 1;
    while (ThreadContext->ctlEnd)
    {
        // by blocking awaiting a receive. 
        while (!tinselCanRecv());
        recv_buf = tinselRecv();
        
        volatile P_Msg_t* rcv_pkt = static_cast<volatile P_Msg_t*>(recv_buf);
        // look for barrier message
        if (((rcv_pkt->header.swAddr & P_SW_OPCODE_MASK) >> P_SW_OPCODE_SHIFT)
                == P_CNC_INIT)
        {
            // *Debug: send packet out to show we have passed the barrier* 
            // softswitch_alive(send_buf);
            // and once it's been received, process it as a startup message
            softswitch_onReceive(ThreadContext, recv_buf);
            ThreadContext->ctlEnd = 0;
        }
        tinselAlloc(recv_buf);
    }                  
}
//------------------------------------------------------------------------------


/*------------------------------------------------------------------------------
 * softswitch_loop: Where the work happens
 *----------------------------------------------------------------------------*/
void softswitch_loop(ThreadCtxt_t* ThreadContext)
{
    // Create RX buffer pointer and get Tinsel Slots
    volatile void *recvBuffer = PNULL;
    volatile void *sendBuffer = tinselSlot(P_MSG_SLOT);     // Send slot
    
    while (!ThreadContext->ctlEnd)
    {
#ifdef SOFTSWITCH_INSTRUMENTATION
        uint32_t cycles = tinselCycleCount();		// cycle counter is per-core
        if((cycles - ThreadContext->lastCycles) > P_INSTR_INTERVAL)
        {
          // Trigger a message to supervisor.
          ThreadContext->pendCycles = 1;
        }
#endif        
        
        // Something to receive
        if (tinselCanRecv())
        {
            recvBuffer=tinselRecv();
            softswitch_onReceive(ThreadContext, recvBuffer); // decode the receive and handle
            tinselAlloc(recvBuffer); // return control of the receive buffer to the hardware
        }

#ifdef SOFTSWITCH_INSTRUMENTATION        
        // Time to send Instrumentation
        else if(ThreadContext->pendCycles && tinselCanSend())
        {
            softswitch_instrumentation(ThreadContext, sendBuffer);
        }
#endif
        
        // Something to send
        else if (ThreadContext->rtsStart != ThreadContext->rtsEnd) //softswitch_IsRTSReady(ThreadContext))
        {
            if (!tinselCanSend())
            {
                // But channel is blocked. Wait until we have something to do.
                tinselWaitUntil(TINSEL_CAN_SEND | TINSEL_CAN_RECV);
            }
            else
            {
                // Let's send something.
                softswitch_onSend(ThreadContext, sendBuffer);
            }
        }
        
        // Nothing to RX, nothing to TX: iterate through all devices until 
        // something happens or all OnComputes have returned 0. 
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
    uint32_t maxIdx = ThreadContext->rtsBufSize;   // # devices we are looping through
    while(ThreadContext->rtsStart != ThreadContext->rtsEnd)
    {
        uint32_t rtsStart = ThreadContext->rtsStart;
        
        ThreadContext->rtsBuf[rtsStart]->idxTgts = 0;
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
        for (uint32_t tgt = 0; tgt < o_pin->numTgts; tgt++)
        {
            outEdge_t* i_tgt = &o_pin->targets[tgt];
            i_tgt->pin = o_pin;
        }
    }
    
    for (uint32_t msg_typ = 0; msg_typ < device->numInputs; msg_typ++)
    {
        inPin_t* i_pin = &device->inputPins[msg_typ];
        
        i_pin->device = device;
        for (uint32_t src = 0; src < i_pin->numSrcs; src++)
        {
            inEdge_t* i_src = &i_pin->sources[src];
            i_src->pin = i_pin;
        }
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
    
    P_Msg_t* msg = static_cast<P_Msg_t*>(const_cast<void*>(send_buf));
    P_Msg_Hdr_t* hdr = &(msg->header);
    P_Instr_Msg_Pyld_t* pyld = reinterpret_cast<P_Instr_Msg_Pyld_t*>(msg->payload);
    
    // Set the Header
    hdr->swAddr = P_SW_MOTHERSHIP_MASK | P_SW_CNC_MASK;
    hdr->swAddr |= ((P_CNC_INSTR << P_SW_OPCODE_SHIFT) & P_SW_OPCODE_MASK);
    hdr->pinAddr = tinselId(); // usurp Pin Addr for the source HW addr
    
    
    // Load the Message
    pyld->threadID = tinselId();
    pyld->cIDX = ThreadContext->cycleIdx++;
    pyld->cycles = cycles - ThreadContext->lastCycles;
    pyld->txCnt = ThreadContext->txCount;
    pyld->supCnt = ThreadContext->superCount;
    pyld->rxCnt = ThreadContext->rxCount;
    pyld->txHanCnt = ThreadContext->txHandlerCount;
    pyld->rxHanCnt = ThreadContext->rxHandlerCount;
    pyld->idleCnt = ThreadContext->idleCount;
    pyld->idleHanCnt = ThreadContext->idleHandlerCount;

#if TinselEnablePerfCount == true
    // Optional Tinsel instrumentation
    uint32_t missCount = tinselMissCount();
    uint32_t hitCount = tinselHitCount();
    uint32_t writebackCount = tinselWritebackCount();
    uint32_t CPUIdleCount = tinselCPUIdleCount();
    
    pyld->missCount = miscount - ThreadContext->lastmissCount;
    pyld->hitCount = hitCount - ThreadContext->lasthitCount;
    pyld->writebackCount = writebackCount - ThreadContext->lastwritebackCount;
    pyld->CPUIdleCount = CPUIdleCount - ThreadContext->lastCPUIdleCount;
#endif 
    
    // Send it
    uint32_t len = p_hdr_size() + p_instrmsg_pyld_size
    tinselSetLen((len - 1) >> TinselLogBytesPerFlit);    // Set the message length
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
    ThreadContext->idleHandlerCount = 0;

#if TinselEnablePerfCount == true   
    ThreadContext->lastmissCount = missCount;
    ThreadContext->lasthitCount = hitCount;
    ThreadContext->lastwritebackCount = writebackCount;
    ThreadContext->lastCPUIdleCount = CPUIdleCount;
#endif 
  
    // Clear the pending message
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
    if(pin->numTgts == 0) return 0;     // Sanity check: make sure the pin has targets
    
    // Pointers of convenience
    devInst_t* device = pin->device;                                // Get the Device
    const outEdge_t* target = &pin->targets[pin->idxTgts];          // Get the target
    volatile char* buf = static_cast<volatile char*>(send_buf);    // Send Buffer
    volatile P_Msg_Hdr_t* hdr = static_cast<volatile P_Msg_Hdr_t*>(send_buf); // Header
    
    size_t hdrSize = p_hdr_size(); //Size of the header.
    
    //--------------------------------------------------------------------------
    // First target, need to run pin's OnSend.
    //--------------------------------------------------------------------------
    if(pin->idxTgts == 0)        
    {
        char* msg = const_cast<char*>(buf)+hdrSize; // Pointer to the message, after headers, etc.
        pin->pinType->Send_Handler(ThreadContext->properties, device, msg);
        ThreadContext->txHandlerCount++;         // Increment SendHandler count
    }
    //--------------------------------------------------------------------------
    
    //--------------------------------------------------------------------------
    // Set the addresses and send the message
    //--------------------------------------------------------------------------
    hdr->swAddr = target->swAddr;
    hdr->pinAddr = target->pinAddr;
    
    uint32_t flt = ((hdrSize + pin->pinType->sz_msg)-1) >> TinselLogBytesPerFlit;
    tinselSetLen((flt < TinselMaxFlitsPerMsg)? flt : (TinselMaxFlitsPerMsg-1));    // Set the message length
    
    if(hdr->swAddr & P_SW_MOTHERSHIP_MASK)
    {   // Message to the Supervisor or External (this goes via the Supervisor)
        tinselSend(tinselHostId(), send_buf);   // Goes to the tinselHost
        ThreadContext->superCount++;         // Increment Supervisor Msg count
    }
    else
    {   // Message to another device.
        tinselSend(target->hwAddr, send_buf);   // Goes to the target's hwAddr
        ThreadContext->txCount++;            // Increment normal Msg count
        
        /*
        //TEMPORARY DEBUGGING BODGE: echo message to supervisor
        volatile uint32_t* last = static_cast<volatile uint32_t*>(send_buf); // TEMPORARY BODGE:
        
        while(!tinselCanSend());
        tinselSetLen(TinselMaxFlitsPerMsg-1);    // Set the message length
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
    
    pin->idxTgts++;     // Increment the target index.
    
    if(pin->idxTgts >= pin->numTgts)  // Reached the end of the send list.
    {
        pin->idxTgts = 0;       // Reset index,
        pin->sendPending = 0;   // Reset pending 

        // Move the circular RTS buffer index
        ThreadContext->rtsStart++;
        if(ThreadContext->rtsStart == ThreadContext->rtsBufSize)
        {
            ThreadContext->rtsStart = 0;
        }
        
        softswitch_onRTS(ThreadContext, device); // Run the Device's RTS handler. This could be conditional.
    }
    
    return pin->idxTgts;
}
//------------------------------------------------------------------------------


/*------------------------------------------------------------------------------
 * softswitch_onReceive: Executed any time a message is received.
 *----------------------------------------------------------------------------*/
void softswitch_onReceive(ThreadCtxt_t* ThreadContext, volatile void* recv_buf)
{
    devInst_t* recvDevBegin;
    devInst_t* recvDevEnd;
    
    ThreadContext->rxCount++;                  // Increment received message count
    
    // Grab the message header 
    volatile P_Msg_Hdr_t* recvHdr = static_cast<volatile P_Msg_Hdr_t*>(recv_buf);
    
    // Decode the header
    uint32_t devAdr = (recvHdr->swAddr & P_SW_DEVICE_MASK) >> P_SW_DEVICE_SHIFT;
    uint8_t opcode = (recvHdr->swAddr & P_SW_OPCODE_MASK) >> P_SW_OPCODE_SHIFT;
    uint8_t pinIdx = (recvHdr->pinAddr & P_HD_TGTPIN_MASK) >> P_HD_TGTPIN_SHIFT;
    uint32_t edgeIdx = (recvHdr->pinAddr & P_HD_DESTEDGEINDEX_MASK) 
                        >> P_HD_DESTEDGEINDEX_SHIFT;
    
    // Stop message ends the simulation & exits main loop at the earliest opportunity
    if((recvHdr->swAddr & P_SW_CNC_MASK) && (opcode == P_CNC_STOP))
    {
        ThreadContext->ctlEnd = 1;
        return;
    }
    
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
    else return;    // Message target is out of range. //TODO: - log/flag
    

    // Loop through each target device (1 unless a broadcast message)
    for (devInst_t* device = recvDevBegin; device != recvDevEnd; device++)
    {
        if(pinIdx >= device->numInputs)
        {   // Sanity check the pin index
            continue;               //TODO: - log/flag
        }
        inPin_t* pin = &device->inputPins[pinIdx];    // Get the pin
        
        ThreadContext->rxHandlerCount++;     // Increment received handler count
        
        
        // Supervisor/CNC/Control Message
        if(recvHdr->swAddr & P_SW_CNC_MASK)
        {
            if(opcode == P_CNC_INIT)
            {
                // **BODGE BODGE BODGE** not so temporary handler for dealing 
                // with __init__. This works ONLY because the __init__ pin on 
                // all devices that have it in existing XML happens to be pin 0 
                // (which means that a Supervisor can guess what the pin number
                // is supposed to be). In any case, this should route it through
                // the __init__ handler. Luckily message types are globally 
                // unique, or likewise this wouldn't work if the device had no
                // __init__ pin. This test should be removed as soon as __init__
                // pins lose any special meaning in existing XML!
                pin->pinType->Recv_handler(ThreadContext->properties, device, 0,
                                            static_cast<const uint8_t*>(
                                            const_cast<const void*>(recv_buf)
                                            )+p_hdr_size());
            }
            else
            {   // Otherwise it triggers OnCtl. Strip volatile at this point. 
                device->devType->OnCtl_Handler(ThreadContext, device, opcode,
                                               static_cast<const uint8_t*>(
                                               const_cast<const void*>(recv_buf)
                                               )+p_hdr_size());
            }
        }
        else
        {   // Handle as a normal packet
            if(edgeIdx >= pin->numSrcs)
            {   // Sanity check the Edge Index
                continue;               //TODO: - log/flag
            }
            pin->pinType->Recv_handler(ThreadContext->properties, device, 
                                        &pin->sources[edgeIdx], 
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
        bool rtsFlagged = false;
        devInst_t* device = &ThreadContext->devInsts[idleIdx];
        
        if (tinselCanRecv())     // Something to RX, bail!
        {
            notIdle = true;       // return 1 as "something" has happened              
            break;
        }
        
        if (device->devType->OnIdle_Handler(ThreadContext->properties, device))
        {
            notIdle = true;                                         // Something interesting happened 
            rtsFlagged |= softswitch_onRTS(ThreadContext, device);  // Call RTS for device
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
    uint32_t rts = 0; //[(P_MAX_OPINS_PER_DEVICE+31)>>5];        // flags for new ready-to-send pins
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
                
                //If the pin has targets and is not already pending,
                if(output_pin->numTgts && output_pin->sendPending == 0)
                {
                    // Flag that pin is pending 
                    output_pin->sendPending = 1;
                    
                    // Add pin to RTS list and update end
                    ThreadContext->rtsBuf[ThreadContext->rtsEnd] = output_pin;
                    
                    ThreadContext->rtsEnd++;
                    if(ThreadContext->rtsEnd == ThreadContext->rtsBufSize)
                    {
                        ThreadContext->rtsEnd = 0;
                    }
                }
            }
        }
    }
    return rts;
}

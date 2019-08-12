#include "softswitch_common.h"
#include "tinsel.h"
#include <cstring>
#include "stdint.h"

void softswitch_init(ThreadCtxt_t* ThreadContext)
{
    for (uint32_t i=1+NUM_SUP_BUFS+MAX_LOG_MSG_BUFS; i < (1<<TinselLogMsgsPerThread); i++) tinselAlloc(tinselSlot(i)); // allocate receive slots.
    for (uint32_t deviceType = 0; deviceType < ThreadContext->numDevTyps; deviceType++) deviceType_init(deviceType, ThreadContext);
    for (uint32_t device = 0; device < ThreadContext->numDevInsts; device++) device_init(&ThreadContext->devInsts[device], ThreadContext);
}

void softswitch_finalize(ThreadCtxt_t* ThreadContext, volatile void** send_buf, volatile void** recv_buf, volatile void** super_buf)
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

    for (uint32_t b = 0; b < NUM_SUP_BUFS; b++) super_buf[b] = PNULL;
    *send_buf = PNULL;
    *recv_buf = PNULL;
    // free mailbox slots
    for (uint32_t i = 0; i < (1<<TinselLogMsgsPerThread); i++) tinselAlloc(tinselSlot(i));
}

void softswitch_barrier(ThreadCtxt_t* ThreadContext, volatile void* send_buf, volatile void* recv_buf)
{
    // first phase of barrier: set up a standard message to send to the supervisor
    set_super_hdr(tinselId() << P_THREAD_OS, P_PKT_MSGTYP_BARRIER, P_SUP_PIN_SYS, p_sup_hdr_size(), 0, static_cast<P_Sup_Hdr_t*>(const_cast<void*>(send_buf)));
    // block until we can send it,
    while (!tinselCanSend());
    tinselSetLen(p_sup_hdr_size());
    // and then issue the message indicating this thread's startup is complete.
    tinselSend(tinselHostId(), send_buf);
    // second phase of barrier: now wait for the supervisor's response
    ThreadContext->ctlEnd = 1;
    while (ThreadContext->ctlEnd)
    {
        // by blocking awaiting a receive. 
        while (!tinselCanRecv());
        recv_buf = tinselRecv();
        P_Msg_t* rcv_pkt = static_cast<P_Msg_t*>(const_cast<void*>(recv_buf));
        // look for barrier message
        if (rcv_pkt->header.messageTag == P_MSG_TAG_INIT)
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

void deviceType_init(uint32_t deviceType_num, ThreadCtxt_t* ThreadContext)
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

void device_init(devInst_t* device, ThreadCtxt_t* ThreadContext)
{
    device->thread = ThreadContext;
    for (uint32_t out = 0; out < device->numOutputs; out++) outPin_init(out, device, ThreadContext); 
    for (uint32_t msg_typ = 0; msg_typ < device->numInputs; msg_typ++) inPin_init(msg_typ, device, ThreadContext);
}

void outPin_init(uint32_t pin, devInst_t* device, ThreadCtxt_t* ThreadContext)
{
    outPin_t* o_pin = &device->outputPins[pin];
    o_pin->device = device;
    for (uint32_t tgt = 0; tgt < o_pin->numTgts; tgt++) outPinTgt_init(tgt, o_pin, ThreadContext);
}

void outPinTgt_init(uint32_t tgt, outPin_t* pin, ThreadCtxt_t* ThreadContext)
{
    outEdge_t* i_tgt = &pin->targets[tgt];
    i_tgt->pin = pin;
}

void inPin_init(uint32_t pin, devInst_t* device, ThreadCtxt_t* ThreadContext)
{
    inPin_t* i_pin = &device->inputPins[pin];
    i_pin->device = device;

    for (uint32_t src = 0; src < i_pin->numSrcs; src++) inPinSrc_init(src, i_pin, ThreadContext);
}

void inPinSrc_init(uint32_t src, inPin_t* pin, ThreadCtxt_t* ThreadContext)
{
    inEdge_t* i_src = &pin->sources[src];
    i_src->pin = pin;
}

uint32_t softswitch_onSend(ThreadCtxt_t* ThreadContext, volatile void* send_buf)
{
    outPin_t* pin = ThreadContext->rtsBuf[ThreadContext->rtsStart];    // Get the next pin to send
    devInst_t* device = pin->device;                                    // Get the Device
    
    //TODO: change this to the standard-size header. 
    // Left as is, this WILL break if a pin has both supervisor and non-supervisor targets.
    uint8_t isSuperMsg = (pin->targets[pin->idxTgts].tgt == DEST_BROADCAST); // Supervisor messages
    size_t hdrSize = isSuperMsg ? p_sup_hdr_size() : p_hdr_size(); // use different headers.
    
    char* buf = static_cast<char*>(const_cast<void*>(send_buf));
    
    if(pin->idxTgts == 0)        // First target, need to run pin's OnSend.
    {
        tinselSetLen((hdrSize + pin->pinType->sz_msg - 1) >> (2+TinselLogWordsPerFlit));    // ??
        //TODO: Fix This
        char* msg = buf+hdrSize;                                                        // Pointer to the message location, after headers, etc.
        pin->pinType->Send_Handler(ThreadContext->properties, device, msg);            // Run the device's OnSend handler
    }
    
    const outEdge_t* target = &pin->targets[pin->idxTgts];
    //==========================================================================
    //TODO: Fix this when headers are fixed as the different treatment here is superfluous.
    if (isSuperMsg)
    {
        set_super_hdr(tinselId() << P_THREAD_OS | ((device->deviceID & P_DEVICE_MASK) << P_DEVICE_OS), 
                        pin->pinType->msgType, target->tgtPin, 
                        pin->pinType->sz_msg+hdrSize, 0, 
                        reinterpret_cast<P_Sup_Hdr_t*>(buf));
        tinselSend(tinselHostId(), send_buf);
    }
    else
    {
        set_msg_hdr(target->tgt, target->tgtEdge, target->tgtPin,
                    pin->pinType->sz_msg, pin->pinType->msgType,
                    reinterpret_cast<P_Msg_Hdr_t*>(buf));
        tinselSend((target->tgt >> P_THREAD_OS), send_buf);
    }
    //==========================================================================
    
    pin->idxTgts++;
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

void softswitch_onReceive(ThreadCtxt_t* ThreadContext, volatile void* recv_buf)
{
    // Decode the message target
    P_Msg_Hdr_t* recvPkt = static_cast<P_Msg_Hdr_t*>(const_cast<void*>(recv_buf));
    devInst_t* recvDevBegin;
    devInst_t* recvDevEnd;
    
    if (recvPkt->destDeviceAddr == DEST_BROADCAST)
    {   // Message is a broadcast to all devices.
        recvDevBegin = ThreadContext->devInsts;
        recvDevEnd = ThreadContext->devInsts + ThreadContext->numDevInsts;
    }
    else if (((recvPkt->destDeviceAddr & P_DEVICE_MASK) >> P_DEVICE_OS) 
                < ThreadContext->numDevInsts)   // TODO: fix when addresses are fixed
    {   // Message is for a single device in range.
        recvDevBegin =  &ThreadContext->devInsts[
                                    (recvPkt->destDeviceAddr & P_DEVICE_MASK)
                                    >> P_DEVICE_OS];
        recvDevEnd = recvDevBegin + 1;
    }
    else return;    // Message target is out of range.
    
    
    // Stop message ends the simulation & exits main loop at the earliest opportunity
    if ((recvPkt->messageTag == P_MSG_TAG_STOP) 
            && (recvPkt->destPin == P_SUP_PIN_SYS_SHORT))
    {
        ThreadContext->ctlEnd = 1;
        return;
    }
    
    // Loop through each target device (1 unless a broadcast message)
    for (devInst_t* device = recvDevBegin; device != recvDevEnd; device++)
    {
        inPin_t* pin = &device->inputPins[recvPkt->destPin];    // Get the pin
        
        // TODO: fix this when we use proper headers
        if (recvPkt->destDeviceAddr & P_SUP_MASK)                                 
        {   // This is a control packet
            if ((recvPkt->messageTag == P_MSG_TAG_INIT) 
                    && (pin->pinType->msgType == recvPkt->messageTag))
            {
                // **BODGE BODGE BODGE** not so temporary handler for dealing with __init__. This works ONLY because
                // the __init__ pin on all devices that have it in existing XML happens to be pin 0 (which means that
                // a Supervisor can guess what the pin number is supposed to be). In any case, this should route it
                // through the __init__ handler. Luckily message types are globally unique, or likewise this wouldn't
                // work if the device had no __init__ pin. This test should be removed as soon as __init__ pins lose
                // any special meaning in existing XML!
                pin->pinType->Recv_handler(ThreadContext->properties, device, 0,
                                            static_cast<const uint8_t*>(
                                            const_cast<const void*>(recv_buf)
                                            )+p_hdr_size());
            }
            else 
            {   // Otherwise it triggers OnCtl
                device->devType->OnCtl_Handler(ThreadContext, device, 
                                                static_cast<const uint8_t*>(
                                                const_cast<const void*>(recv_buf)
                                                )+p_hdr_size());
            }
        }
        else
        {   // Handle as a normal packet
            pin->pinType->Recv_handler(ThreadContext->properties, device, 
                                        &pin->sources[recvPkt->destEdgeIndex], 
                                        static_cast<const uint8_t*>(
                                        const_cast<const void*>(recv_buf)
                                        )+p_hdr_size());
        }    
        softswitch_onRTS(ThreadContext, device);    // Run OnRTS for the device
    }
}

bool softswitch_onIdle(ThreadCtxt_t* ThreadContext)
{
    uint32_t maxIdx = ThreadContext->numDevInsts - 1;   // # devices we are looping through
    //devInst_t* devices = ThreadContext->devInsts;       // Shortcut to the devices array
    
    uint32_t idleStart = ThreadContext->idleStart;      // pickup where we left off
    uint32_t idleIdx = idleStart;
    bool notIdle = false;                           // Return val
    
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
            rtsFlagged = softswitch_onRTS(ThreadContext, device);   // Call RTS for device
        }

        if(idleIdx < maxIdx) ++idleIdx;     // Increment the index,
        else idleIdx = 0;                   // or reset it to wrap.
        
        if (rtsFlagged) break;              // RTS has been flagged - bail!
    } while (idleIdx != idleStart);     // Exit if we have serviced all devices.
    
    ThreadContext->idleStart = idleIdx;    // save our position
    return notIdle;     
}

uint32_t softswitch_onRTS(ThreadCtxt_t* ThreadContext, devInst_t* device)
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
                
                //If the pin is not already pending,
                if(output_pin->sendPending == 0)
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

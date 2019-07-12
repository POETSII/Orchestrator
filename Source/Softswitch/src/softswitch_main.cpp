#include "softswitch_common.h"
#include "tinsel.h"

INLINE bool softswitch_IsRTSReady(ThreadCtxt_t* ThreadContext) 
{
    return *ThreadContext->rtsStart != *ThreadContext->rtsEnd;
}

uint32_t softswitch_onRTS(ThreadCtxt_t* ThreadContext, devInst_t* device)
{
    // Essentially:
    //        Run Device's OnRts
    //         Check pin mask for each pin
    //        If pin does not have pending send, add to buffer
    
    
    // Creating multiple bit-fields for RTS depending on a config flag.
    // 32 = 1 bit-field, 64 = 2 bit-fields, etc. etc.
    uint32_t rts[(P_MAX_OPINS_PER_DEVICE+31)>>5];        // flags for new ready-to-send pins
    uint32_t num_grps = (31+device->numOutputs) >> 5;    // ??
    uint32_t pin_grp, pin, pins_this_grp;
    
    // Run device's RTS handler.
    uint32_t rtsUpdated = device->devType->RTS_Handler(ThreadContext->properties, device, rts)
    if (rtsUpdated)
    {
        for (pin_grp = 0; pin_grp < num_grps; pin_grp++)    // Loop through each pin group?!
        {
            pins_this_grp = (pin_grp == num_grps-1 ? (device->numOutputs - 32*pin_grp) : 32);
            for (pin = 0; pin < pins_this_grp; pin++)       // Loop through each pin in the group
            {
                if (rts[pin_grp] & 0x1 << pin)              // If the pin is marked active,
                {
                    // Get a pointer to the active pin
                    outPin_t* output_pin = &device->outputPins[32*pin_grp + pin];
                    
                    //If the pin is not already pending,
                    if(output_pin->sendPending == 0)
                    {
                        // Flag that pin is pending 
                        output_pin->sendPending = 1;
                        
                        // Add pin to RTS list and update end
                        ThreadContext->rtsBuff[ThreadContext->rtsEnd] = outputPin;
                        if(ThreadContext->rtsEnd < ThreadContext->rtsBufSize) ThreadContext->rtsEnd++;
                        else ThreadContext->rtsEnd = 0;
                    }
                }
            }
        }
    }
    return rtsUpdated;
}

inline bool softswitch_onIdle(ThreadCtxt_t* ThreadContext)
{
    uint32_t maxIdx = *ThreadContext->numDevInsts - 1;  // # devices we are looping through
    devInst_t* devices = ThreadContext->devInsts;       // Shortcut to the devices array
    
    uint32_t idleStart = *ThreadContext->idleStart; // pickup where we left off
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
    
    *ThreadContext->idleStart = idleIdx;    // save our position
    return notIdle;     
}

inline uint32_t softswitch_onSend(ThreadCtxt_t* ThreadContext, volatile char* send_buf)
{
    outPin_t* pin = ThreadContext->rtsBuff[ThreadContext->rtsStart];    // Get the next pin to send
    devInst_t* device = pin->device;                                    // Get the Device
    
    //TODO: change this to the standard-size header. 
    // Left as is, this WILL break if a pin has both supervisor and non-supervisor targets.
    uint8_t isSuperMsg = (pin->targets[pin->idxTgts].tgt == DEST_BROADCAST); // Supervisor messages
    size_t hdrSize = isSuperMsg ? p_sup_hdr_size() : p_hdr_size(); // use different headers.
    
    
    if(pin->idxTgts == 0)        // First target, need to run pin's OnSend.
    {
        tinselSetLen((hdrSize + pin->pinType->sz_msg - 1) >> (2+TinselLogWordsPerFlit));    // ??
        //TODO: Fix This
        volatile void* msg = send_buf+hdrSize;                    // Pointer to the message location, after headers, etc.
        pin->pinType->Send_Handler(ThreadContext->properties, device, msg);            // Run the device's OnSend handler
    }
    
    const outEdge_t* target = &pin->targets[pin->idxTgts];
    //==========================================================================
    //TODO: Fix this when headers are fixed as the different treatment here is superfluous.
    if (isSuperMsg)
    {
        set_super_hdr(tinselId() << P_THREAD_OS | ((cur_device->deviceID & P_DEVICE_MASK) << P_DEVICE_OS), pin->pinType->msgType, target->tgtPin, pin->pinType->sz_msg+hdrSize, 0, static_cast<P_Sup_Hdr_t*>(const_cast<void*>(send_buf)));
        tinselSend(tinselHostId(), send_buf);
    }
    else
    {
        set_msg_hdr(target->tgt, target->tgtEdge, target->tgtPin, pin->pinType->sz_msg, pin->pinType->msgType, static_cast<P_Msg_Hdr_t*>(const_cast<void*>(send_buf)));
        tinselSend((target->tgt >> P_THREAD_OS), send_buf);
    }
    //==========================================================================
    
    pin->idxTgts++;
    if(pin->idxTgts >= pin->numTgts)  // Reached the end of the send list.
    {
        pin->idxTgts = 0;       // Reset index,
        pin->sendPending = 0;   // Reset pending 

        // Move the circular RTS buffer index - make a popRTS method?
        if(ThreadContext->rtsStart < ThreadContext->rtsBufSize) ThreadContext->rtsStart++;
        else ThreadContext->rtsStart = 0;
        
        softswitch_onRTS(ThreadContext, device); // Run the Device's RTS handler. This could be conditional.
    }
    
    return pin->idxTgts;
}

inline void softswitch_onReceive(ThreadCtxt_t* ThreadContext, volatile void* recv_buf)
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
    else if (((recvPkt->destDeviceAddr & P_DEVICE_MASK) >> P_DEVICE_OS) < ThreadContext->numDevInsts)   // TODO: fix when addresses are fixed
    {   // Message is for a single device in range.
        recvDevBegin =  &ThreadContext->devInsts[(recvPkt->destDeviceAddr & P_DEVICE_MASK) >> P_DEVICE_OS];
        recvDevEnd = recvDevBegin + 1;
    }
    else return;    // Message target is out of range.
    
    
    // Stop message ends the simulation & exits main loop at the earliest opportunity
    if ((recvPkt->messageTag == P_MSG_TAG_STOP) && (recvPkt->destPin == P_SUP_PIN_SYS_SHORT))
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
            if ((recvPkt->messageTag == P_MSG_TAG_INIT) && (pin->pinType->msgType == recvPkt->messageTag))
            {
                // **BODGE BODGE BODGE** not so temporary handler for dealing with __init__. This works ONLY because
                // the __init__ pin on all devices that have it in existing XML happens to be pin 0 (which means that
                // a Supervisor can guess what the pin number is supposed to be). In any case, this should route it
                // through the __init__ handler. Luckily message types are globally unique, or likewise this wouldn't
                // work if the device had no __init__ pin. This test should be removed as soon as __init__ pins lose
                // any special meaning in existing XML!
                pin->pinType->Recv_handler(ThreadContext->properties, device, 0, static_cast<const uint8_t*>(const_cast<const void*>(recv_buf))+p_hdr_size());
            }
            else 
            {   // Otherwise it triggers OnCtl
                device->devType->OnCtl_Handler(ThreadContext, device, static_cast<const uint8_t*>(const_cast<const void*>(recv_buf))+p_hdr_size());
            }
        }
        else
        {   // Handle as a normal packet
            pin->pinType->Recv_handler(ThreadContext->properties, device, &pin->sources[recvPkt->destEdgeIndex], static_cast<const uint8_t*>(const_cast<const void*>(recv_buf))+p_hdr_size());
        }    
        softswitch_onRTS(ThreadContext, device);    // Run OnRTS for the device
    }
}



void softswitch_main()
{
    // The thread context will reside at the base of the DRAM heap.
    PThreadContext* ThreadContext = static_cast<PThreadContext*>(tinselHeapBase()); // softswitch_pThreadContexts + tinselID();
    
    // Configure rtsBuf - this sits after ThreadContext in the heap 
    ThreadContext->rtsBuf = static_cast<outPin_t**>(tinselHeapBase() + sizeof(PThreadContext));
    
    // can these slot assignments be done in softswitch_init?
    volatile void *recvBuffer=0;
    volatile char *sendBuffer = static_cast<char*>(tinselSlot(0));   // hardware send buffer is dedicated to the first hw slot
    volatile void *superBuffer[NUM_SUP_BUFS];  // buffers allocated for supervisor messages.
    for (uint32_t sb = 0; sb < NUM_SUP_BUFS; sb++) superBuffer[sb] = tinselSlot(sb+1);
    softswitch_init(ThreadContext);
    // send a message to the local supervisor saying we are ready;
    // then wait for the __init__ message to return by interrogating tinselCanRecv(). 
    softswitch_barrier(ThreadContext, superBuffer[0], recvBuffer);

    // Endless main loop, that is until thread context says to stop.
    while (!ThreadContext->ctlEnd)
    {
        // Something to receive
        if(tinselCanRecv())               
        {
            recvBuffer=tinselRecv();      // Get the message from the HW
            softswitch_onReceive(ThreadContext, recvBuffer); // decode & handle
            tinselAlloc(recvBuffer);      // return control of the buffer to HW
        } 
        
        // Something to send
        else if (softswitch_IsRTSReady(ThreadContext)) 
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
    softswitch_finalize(ThreadContext, &sendBuffer, &recvBuffer, superBuffer); // shut down once the end signal has been received.
}

int main(int argc, char** argv)
{
    softswitch_main();
    return 0;
}

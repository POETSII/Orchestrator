#include "softswitch_common.h"
#include "tinsel.h"
#include <cstring>
#include "stdint.h"

void softswitch_init(ThreadCtxt_t* thr_ctxt)
{
    for (uint32_t i=1+NUM_SUP_BUFS+MAX_LOG_MSG_BUFS; i < (1<<TinselLogMsgsPerThread); i++) tinselAlloc(tinselSlot(i)); // allocate receive slots.
    for (uint32_t deviceType = 0; deviceType < thr_ctxt->numDevTyps; deviceType++) deviceType_init(deviceType, thr_ctxt);
    for (uint32_t device = 0; device < thr_ctxt->numDevInsts; device++) device_init(&thr_ctxt->devInsts[device], thr_ctxt);
}

void softswitch_finalize(ThreadCtxt_t* thr_ctxt, volatile void** send_buf, volatile void** recv_buf, volatile void** super_buf)
{
    // basic shutdown processing: clear buffers
    for (uint32_t device = 0; device < thr_ctxt->numDevInsts; device++)
    {
        for (uint32_t out_pin = 0; out_pin < thr_ctxt->devInsts[device].numOutputs; out_pin++)
        {
            while (softswitch_popMsg(&thr_ctxt->devInsts[device].outputPins[out_pin]));
        }
        while (softswitch_popRTSPin(&thr_ctxt->devInsts[device]));
    }
    while (softswitch_popRTS(thr_ctxt));
    for (uint32_t b = 0; b < NUM_SUP_BUFS; b++) super_buf[b] = 0;
    *send_buf = *recv_buf = 0;
    // free mailbox slots
    for (uint32_t i = 0; i < (1<<TinselLogMsgsPerThread); i++) tinselAlloc(tinselSlot(i));
}

void softswitch_barrier(ThreadCtxt_t* thr_ctxt, volatile void* send_buf, volatile void* recv_buf)
{
    // first phase of barrier: set up a standard message to send to the supervisor
    set_super_hdr(tinselId() << P_THREAD_OS, P_PKT_MSGTYP_BARRIER, P_SUP_PIN_SYS, p_sup_hdr_size(), 0, static_cast<P_Sup_Hdr_t*>(const_cast<void*>(send_buf)));
    // block until we can send it,
    while (!tinselCanSend());
    tinselSetLen(p_sup_hdr_size());
    // and then issue the message indicating this thread's startup is complete.
    tinselSend(tinselHostId(), send_buf);
    // second phase of barrier: now wait for the supervisor's response
    thr_ctxt->ctlEnd = 1;
    while (thr_ctxt->ctlEnd)
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
            softswitch_onReceive(thr_ctxt, recv_buf);
            thr_ctxt->ctlEnd = 0;
        }
        tinselAlloc(recv_buf);
    }                  
}

void deviceType_init(uint32_t deviceType_num, ThreadCtxt_t* thr_ctxt)
{
    devTyp_t* deviceType OS_ATTRIBUTE_UNUSED= &thr_ctxt->devTyps[deviceType_num];
    OS_PRAGMA_UNUSED(deviceType)
    /*
    Handler addresses ought to reside in instruction memory and thus remain valid: to be seen.
    deviceType->RTS_Handler = RTS_Handlers[thr_ctxt->threadID.PThread][deviceType_num]; // RTS_Handlers is a table that must be built by the Orchestrator.
    deviceType->OnIdle_Handler = OnIdle_Handlers[thr_ctxt->threadID.PThread][deviceType_num];
    deviceType->OnCtl_Handler = OnCtl_Handlers[thr_ctxt->threadID.PThread][deviceType_num]; // OnCtl_Handler should amongst other things handle monitoring.
    */
    //deviceType->outputTypes = offset_ptr<out_pintyp_t*>(thr_ctxt, deviceType->outputTypes);
    //deviceType->inputTypes = offset_ptr<in_pintyp_t*>(thr_ctxt, deviceType->inputTypes);
    // for (uint32_t o = 0; o < deviceType->numOutputTypes; o++) outputPinType_init(o, deviceType_num, thr_ctxt);
    // for (uint32_t i = 0; i < deviceType->numInputTypes; i++) inputPinType_init(i, deviceType_num, thr_ctxt);
}

void device_init(devInst_t* device, ThreadCtxt_t* thr_ctxt)
{
    device->thread = thr_ctxt;
    for (uint32_t out = 0; out < device->numOutputs; out++) outPin_init(out, device, thr_ctxt); 
    for (uint32_t msg_typ = 0; msg_typ < device->numInputs; msg_typ++) inPin_init(msg_typ, device, thr_ctxt);
}

void outPin_init(uint32_t pin, devInst_t* device, ThreadCtxt_t* thr_ctxt)
{
    outPin_t* o_pin = &device->outputPins[pin];
    o_pin->device = device;
    for (uint32_t tgt = 0; tgt < o_pin->numTgts; tgt++) outPinTgt_init(tgt, o_pin, thr_ctxt);
    // initialise the message queue circular buffers. Buffer size 0 implies no initialisation.
    for (uint32_t buf = 0; buf < P_MSG_Q_MAXCOUNT; buf++)
    { 
        if (buf+1 != P_MSG_Q_MAXCOUNT) o_pin->msg_q_buf[buf].next = &o_pin->msg_q_buf[buf+1];
        else o_pin->msg_q_buf[P_MSG_Q_MAXCOUNT-1].next = &o_pin->msg_q_buf[0];
        
        if (buf) o_pin->msg_q_buf[buf].prev = &o_pin->msg_q_buf[buf-1];
        else o_pin->msg_q_buf[0].prev = &o_pin->msg_q_buf[P_MSG_Q_MAXCOUNT-1];
    }
}

void outPinTgt_init(uint32_t tgt, outPin_t* pin, ThreadCtxt_t* thr_ctxt)
{
    outEdge_t* i_tgt = &pin->targets[tgt];
    i_tgt->pin = pin;
}

void inPin_init(uint32_t pin, devInst_t* device, ThreadCtxt_t* thr_ctxt)
{
    inPin_t* i_pin = &device->inputPins[pin];
    i_pin->device = device;

    for (uint32_t src = 0; src < i_pin->numSrcs; src++) inPinSrc_init(src, i_pin, thr_ctxt);
}

void inPinSrc_init(uint32_t src, inPin_t* pin, ThreadCtxt_t* thr_ctxt)
{
    inEdge_t* i_src = &pin->sources[src];
    i_src->pin = pin;
}

int softswitch_onSend(ThreadCtxt_t* thr_ctxt, volatile void* send_buf)
{
    devInst_t* cur_device = thr_ctxt->RTSHead;
    outPin_t* cur_pin = cur_device->RTSPinHead;
    uint32_t buffered = 0; // additional argument for OnSend - could also use the msgType field
    uint8_t isSuperMsg = (cur_pin->targets[cur_device->currTgt].tgt == DEST_BROADCAST); // Supervisor messages
    size_t hdrSize = isSuperMsg ? p_sup_hdr_size() : p_hdr_size(); // use different headers
    // set up OnSend with the first address   
    if (cur_device->currTgt == 0)
    {
        tinselSetLen((hdrSize + cur_pin->pinType->sz_msg - 1) >> (2+TinselLogWordsPerFlit));
        // if we are buffering, is there a message in the buffer?
        if (cur_pin->msg_q_head)
        {
            // if so, move its contents into the hardware mailbox
            P_Msg_Q_t* msg = softswitch_popMsg(cur_pin);
            // offsetting the copy location by the header length to space for header to be inserted.
            memcpy(static_cast<char*>(const_cast<void*>(send_buf))+hdrSize, &msg->msg, cur_pin->pinType->sz_msg);
            buffered = 1;
        }
        // then run the application's OnSend (which, if we are buffering, may alter the buffer again)
        uint32_t RTS_updated OS_ATTRIBUTE_UNUSED= cur_pin->pinType->Send_Handler(thr_ctxt->properties, cur_device, static_cast<char*>(const_cast<void*>(send_buf))+hdrSize, buffered);
        OS_PRAGMA_UNUSED(RTS_updated)
    }
    // send the message (to as many destinations as possible before the network blocks).
    while (tinselCanSend() && cur_device->currTgt < cur_pin->numTgts)
    {
        const outEdge_t* target = &cur_pin->targets[cur_device->currTgt++];
        if (isSuperMsg)
        {
            set_super_hdr(tinselId() << P_THREAD_OS | ((cur_device->deviceID & P_DEVICE_MASK) << P_DEVICE_OS), cur_pin->pinType->msgType, target->tgtPin, cur_pin->pinType->sz_msg+hdrSize, 0, static_cast<P_Sup_Hdr_t*>(const_cast<void*>(send_buf)));
            tinselSend(tinselHostId(), send_buf);
            thr_ctxt->superCount++;
        }
        else
        {
            set_msg_hdr(target->tgt, target->tgtEdge, target->tgtPin, cur_pin->pinType->sz_msg, cur_pin->pinType->msgType, static_cast<P_Msg_Hdr_t*>(const_cast<void*>(send_buf)));
            tinselSend((target->tgt >> P_THREAD_OS), send_buf);
            thr_ctxt->sentCount++;
        }
    }
    // then update the RTS list as necessary
    softswitch_onRTS(thr_ctxt, cur_device);
    return cur_device->currTgt;
}

void softswitch_onReceive(ThreadCtxt_t* thr_ctxt, volatile void* recv_buf)
{
    // first need to do some basic decode of the packet:
    P_Msg_Hdr_t* recv_pkt = static_cast<P_Msg_Hdr_t*>(const_cast<void*>(recv_buf));
    devInst_t* recv_device_begin = thr_ctxt->devInsts;
    devInst_t* recv_device_end = thr_ctxt->devInsts+thr_ctxt->numDevInsts;
    // device in range? A Supervisor packet may target the entire group by sending a broadcast address.    
    if (recv_pkt->destDeviceAddr != DEST_BROADCAST)
    {
        if (((recv_pkt->destDeviceAddr & P_DEVICE_MASK) >> P_DEVICE_OS) < thr_ctxt->numDevInsts)
           recv_device_end = (recv_device_begin =  &thr_ctxt->devInsts[(recv_pkt->destDeviceAddr & P_DEVICE_MASK) >> P_DEVICE_OS]) + 1;
        else return; // exit and dump packet if the device is out of range.
    }
    uint32_t RTS_updated __attribute__((unused));
    // stop message ends the simulation and exits the update loop at the earliest possible opportunity
    if ((recv_pkt->messageTag == P_MSG_TAG_STOP) && (recv_pkt->destPin == P_SUP_PIN_SYS_SHORT))
    {
        thr_ctxt->ctlEnd = 1;
        return;
    }
    
    // go through the devices (usually only 1)
    for (devInst_t* recv_device = recv_device_begin; recv_device != recv_device_end; recv_device++)
    {
        // which pin will receive the message?
        inPin_t* recv_pin = &recv_device->inputPins[recv_pkt->destPin];
        // source was a supervisor? Run OnCtl. We assume here full context (thread, device) should be passed.
        if (recv_pkt->destDeviceAddr & P_SUP_MASK)
        {
            // **BODGE BODGE BODGE** *Very* temporary handler for dealing with __init__. This works ONLY because
            // the __init__ pin on all devices that have it in existing XML happens to be pin 0 (which means that
            // a Supervisor can guess what the pin number is supposed to be). In any case, this should route it
            // through the __init__ handler. Luckily message types are globally unique, or likewise this wouldn't
            // work if the device had no __init__ pin. This test should be removed as soon as __init__ pins lose
            // any special meaning in existing XML!
            if ((recv_pkt->messageTag == P_MSG_TAG_INIT) && (recv_pin->pinType->msgType == recv_pkt->messageTag))
                RTS_updated = recv_pin->pinType->Recv_handler(thr_ctxt->properties, recv_device, 0, static_cast<const uint8_t*>(const_cast<const void*>(recv_buf))+p_hdr_size());
            else RTS_updated = recv_device->devType->OnCtl_Handler(thr_ctxt, recv_device, static_cast<const uint8_t*>(const_cast<const void*>(recv_buf))+p_hdr_size());
        }
            // otherwise handle as a normal device through the appropriate receive handler.
            else RTS_updated = recv_pin->pinType->Recv_handler(thr_ctxt->properties, recv_device, &recv_pin->sources[recv_pkt->destEdgeIndex], static_cast<const uint8_t*>(const_cast<const void*>(recv_buf))+p_hdr_size());
        // finally, run the RTS handler for the device.
        softswitch_onRTS(thr_ctxt, recv_device);
    }
}

bool softswitch_onIdle(ThreadCtxt_t* thr_ctxt)
{
    uint32_t cur_device = thr_ctxt->nextOnIdle & ~P_ONIDLE_CHANGE;
    if (cur_device >= thr_ctxt->numDevInsts) return false;
    uint32_t last_device = cur_device+IDLE_SWEEP_CHUNK_SIZE;
    if (last_device >= (thr_ctxt->numDevInsts)) last_device = IDLE_SWEEP_CHUNK_SIZE - thr_ctxt->numDevInsts + last_device;
    while (cur_device != last_device)
    {
        devInst_t* device = &thr_ctxt->devInsts[cur_device];
        // each device's OnIdle handler and if something interesting happens update the flag and run the RTS handler.
        if (device->devType->OnIdle_Handler(thr_ctxt->properties, device))
        {
            if (++cur_device >= thr_ctxt->numDevInsts) thr_ctxt->nextOnIdle = P_ONIDLE_CHANGE;
            else thr_ctxt->nextOnIdle = P_ONIDLE_CHANGE | cur_device;
            softswitch_onRTS(thr_ctxt, device);
            return true;
        }
        if (++cur_device >= thr_ctxt->numDevInsts)
        {
            if (thr_ctxt->nextOnIdle & P_ONIDLE_CHANGE)
            {
               // something 'interesting' happened in OnIdle. So wrap around to the top of the device list and continue looping
               thr_ctxt->nextOnIdle &= ~P_ONIDLE_CHANGE;
               cur_device = 0;
            }
            else
            {
               thr_ctxt->nextOnIdle = thr_ctxt->numDevInsts;
               return false; // got through the entire device list without incident. Exit and wait for I/O.
            }
        }
    }
    thr_ctxt->nextOnIdle = last_device | (thr_ctxt->nextOnIdle & P_ONIDLE_CHANGE);
    return true; // got through this chunk of the device list. Allow I/O the chance to register. 
}

void softswitch_onRTS(ThreadCtxt_t* thr_ctxt, devInst_t* device)
{
    uint32_t rts[(P_MAX_OPINS_PER_DEVICE+31)>>5]; // flags for new ready-to-send pins
    void* rts_buf[P_MAX_OPINS_PER_DEVICE] = {}; // send buffers for the RTS handler, using default zero-initialisation
    uint32_t num_grps = (31+device->numOutputs) >> 5;
    uint32_t pin_grp, pin, pins_this_grp;
    // first need to deal with the active sending device. If it's finished sending
    // a message, we remove it from the queue before handling new insertions.
    outPin_t* cur_pin = device->RTSPinHead;
    if (device == thr_ctxt->RTSHead)
    {
        if (device->currTgt >= cur_pin->numTgts)
        {
            // all targets for the current message have been sent to. Go to next target group. 
            device->currTgt = 0;
            // this function does not affect the internal queue pointers for its pin yet
            // this way, new pins that want to send will always get priority.
            softswitch_popRTSPin(device);
            softswitch_popRTS(thr_ctxt);
        }
    }
    // set up the active pin flags and buffers.
    for (pin_grp = 0; pin_grp < num_grps; pin_grp++)
    {
        rts[pin_grp] = 0;
        pins_this_grp = (pin_grp == num_grps-1 ? (device->numOutputs - 32*pin_grp) : 32);
        for (pin = 0; pin < pins_this_grp; pin++)
        {
            // any pin that still has message buffers available can provide one to the RTS handler.
            if (P_Msg_Q_t* pin_nextMsg = softswitch_nextMsg(&device->outputPins[32*pin_grp + pin]))
            rts_buf[32*pin_grp + pin] = &pin_nextMsg->msg;      
        }       
    }
    // run the application's RTS handler
    if (device->devType->RTS_Handler(thr_ctxt->properties, device, rts, rts_buf))
    {
        // and look for newly active pins.
        for (pin_grp = 0; pin_grp < num_grps; pin_grp++)
        {
            pins_this_grp = (pin_grp == num_grps-1 ? (device->numOutputs - 32*pin_grp) : 32);
            for (pin = 0; pin < pins_this_grp; pin++)
            {
                // is the pin now active? 
                if (rts[pin_grp] & 0x1 << pin)
                {
                    // yes. Push the various queues. 
                    outPin_t* output_pin = &device->outputPins[32*pin_grp + pin];
                    if (rts_buf[32*pin_grp + pin]) // pin has an available message buffer?
                    {
                        // append to the pin's message queue.
                        // if we can't append, the message will be silently dropped. This
                        // *could* be fixed with a while(!softswitch_nextMsg) loop and update
                        // of the buffer until it could be buffered, but this could be complicated
                        // and may introduce the possibility of deadlock.
                        softswitch_pushMsg(output_pin, softswitch_nextMsg(output_pin));
                    }
                    // pin not already active. Insert into the queue.
                    if (!(output_pin->RTSPinPrev || output_pin->RTSPinNext)) softswitch_pushRTSPin(device, output_pin);
                }
            }
        }
    }
    // rotate the currently active pin round to the back of the device's list if it still has messages to send
    if (cur_pin && (cur_pin != device->RTSPinHead) && cur_pin->msg_q_head) softswitch_pushRTSPin(device, cur_pin);
    // device not already active. Insert into the queue, or rotate the device round to the back of the thread's RTS list if it's still RTS.
    if (!(device->RTSPrev || device->RTSNext || (device == thr_ctxt->RTSHead)) && device->RTSPinHead) softswitch_pushRTS(thr_ctxt, device); 
}

void softswitch_pushRTS(ThreadCtxt_t* thr_ctxt, devInst_t* new_device)
{
    new_device->RTSNext = 0;
    new_device->RTSPrev = thr_ctxt->RTSTail;
    // need to test head because it will have to be set independently if it is null.
    if (!thr_ctxt->RTSHead) thr_ctxt->RTSHead = new_device;
    // only if head is nonzero will tail have a meaningful value to set.
    else thr_ctxt->RTSTail->RTSNext = new_device;
    thr_ctxt->RTSTail = new_device;
}

devInst_t* softswitch_popRTS(PThreadContext* thr_ctxt)
{
    devInst_t* popped = thr_ctxt->RTSHead;
    // single = in the if-conditional is correct here: we are setting and then testing for 0 having set.
    if ((thr_ctxt->RTSHead = popped->RTSNext)) thr_ctxt->RTSHead->RTSPrev = 0;
    else thr_ctxt->RTSTail = 0; // may not be necessary since softswitch_pushRTS always overwrites RTSTail and queue empty checks look at RTSHead
    popped->RTSNext = popped->RTSPrev = 0; // zero out the popped device's linked-list pointers to remove it entirely from the list.
    return popped;
}

void softswitch_pushRTSPin(devInst_t* device, outPin_t* new_pin)
{
    new_pin->RTSPinNext = 0;
    new_pin->RTSPinPrev = device->RTSPinTail;
    // need to test head because it will have to be set independently if it is null.
    if (!device->RTSPinHead) device->RTSPinHead = new_pin;
    // only if head is nonzero will tail have a meaningful value to set.
    else device->RTSPinTail->RTSPinNext = new_pin;
    device->RTSPinTail = new_pin;
}
       
outPin_t* softswitch_popRTSPin(devInst_t* device)
{
    outPin_t* popped = device->RTSPinHead;
    // single = in the if-conditional is correct here: we are setting and then testing for 0 having set.
    if ((device->RTSPinHead = popped->RTSPinNext)) device->RTSPinHead->RTSPinPrev = 0;
    else device->RTSPinTail = 0;
    popped->RTSPinNext = popped->RTSPinPrev = 0;
    return popped;
}

/* these 3 operations work on a doubly-linked circular buffer. Buffer members
   are kept in order so we can always insert a new buffer into the active list
   by simply appending the next available one. If the next one is also
   the head of the active list, the buffer is exhausted.
 */
void softswitch_pushMsg(outPin_t* pin, P_Msg_Q_t* msg)
{
    pin->msg_q_tail = msg; // can always insert; no need for pointer juggling. 
    if (!pin->msg_q_head) pin->msg_q_head = msg; // if this is the first member it should be the head as well.
}

P_Msg_Q_t* softswitch_popMsg(outPin_t* pin)
{
    P_Msg_Q_t* popped = pin->msg_q_head;
    // if this is the last active member then zero the list.
    if (pin->msg_q_head == pin->msg_q_tail) pin->msg_q_head = pin->msg_q_tail = 0;
    else pin->msg_q_head = popped->next;
    return popped;
}

P_Msg_Q_t* softswitch_nextMsg(outPin_t* pin)
{
    // no buffer available. 
    if (!pin->msg_q_buf) return 0;
    // queue empty - can use the first available message.
    if (!pin->msg_q_head) return &pin->msg_q_buf[0];
    // buffer exhausted - no available messages. 
    if (pin->msg_q_tail->next == pin->msg_q_head) return 0;
    else return pin->msg_q_tail->next; // otherwise give us a message.
}

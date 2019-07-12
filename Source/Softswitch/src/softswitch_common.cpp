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

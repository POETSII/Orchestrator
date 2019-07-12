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

void softswitch_finalize(ThreadCtxt_t* ThreadContext, volatile char** send_buf, volatile void** recv_buf, volatile void** super_buf)
{
    // Loop through everything in the RTS buffer and clear it
    uint32_t maxIdx = ThreadContext->rtsBufSize - 1;   // # devices we are looping through
    while(ThreadContext->rtsStart != ThreadContext->rtsEnd)
    {
        uint32_t rtsStart = ThreadContext->rtsStart;
        
        ThreadContext->rtsBuf[rtsStart]->idxTgts = 0;
        ThreadContext->rtsBuf[rtsStart]->sendPending = 0;
        ThreadContext->rtsBuf[rtsStart] = PNULL;
        
        if(ThreadContext->rtsStart < maxIdx) ThreadContext->rtsStart++;
        else ThreadContext->rtsStart = 0;
    }

    for (uint32_t b = 0; b < NUM_SUP_BUFS; b++) super_buf[b] = PNULL;
    *send_buf = PNULL;
    *recv_buf = PNULL;
    // free mailbox slots
    for (uint32_t i = 0; i < (1<<TinselLogMsgsPerThread); i++) tinselAlloc(tinselSlot(i));
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
    for (uint32_t out = 0; out < device->numOutputs; out++) outPin_init(out, device); 
    for (uint32_t msg_typ = 0; msg_typ < device->numInputs; msg_typ++) inPin_init(msg_typ, device, thr_ctxt);
}

void outPin_init(uint32_t pin, devInst_t* device)
{
    outPin_t* o_pin = &device->outputPins[pin];
    
    for (uint32_t tgt = 0; tgt < o_pin->numTgts; tgt++) 
    {
        outPinTgt_init(tgt, o_pin);
    }
}

void outPinTgt_init(uint32_t tgt, outPin_t* pin)
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

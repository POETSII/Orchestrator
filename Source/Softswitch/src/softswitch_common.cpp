#include "softswitch_common.h"
#include "tinsel.h"
#include <cstring>
#include "stdint.h"
#include <algorithm>
using namespace std;

InputPinLookup::InputPinLookup(const devInst_t* device) : target_device(device)
{
}

inPinSrc_t* InputPinLookup::find_inputPin(uint32_t msgType, uint32_t srcID)
{
   // slightly bodgey way of getting quick lookups
  inPinMsg_t msgKeyType;
  msgKeyType.device = target_device;
  msgKeyType.msgType = msgType;
  inPinSrc_t srcKeyType;
  srcKeyType.src = srcID;
  pair<inPinMsg_t*, inPinMsg_t*> matching_msgs;
  // pair<inPinSrc_t*, inPinSrc_t*> matching_srcs;
  matching_msgs = equal_range(target_device->inputPins, (target_device->inputPins+target_device->numInputs), msgKeyType, msg_key_compare);
  return equal_range(matching_msgs.first->sources, (matching_msgs.first->sources+matching_msgs.first->numSrcs), srcKeyType, src_key_compare).first;
}

template<class T> T offset_ptr(ThreadCtxt_t* base, T offset)
{
  // dangerous but necessary - we have no idea how large data structures are and the best that can be done in the loaded context
  // is to give them as a pointer referenced to a reliable base address. There is little we can count on here.
  uintptr_t offset_addr = reinterpret_cast<uintptr_t>(offset);
  uintptr_t virtual_addr = reinterpret_cast<uintptr_t>(base->virtualAddr);
  uintptr_t base_addr = reinterpret_cast<uintptr_t>(base);
  uintptr_t physical_offset;
  if (virtual_addr > offset_addr)
  {
     physical_offset = virtual_addr - offset_addr;
     if (physical_offset > base_addr) return 0; // memory base address so low the datastructures won't fit in front of it.
     else return reinterpret_cast<T>(base_addr-physical_offset);
  }
  else
  {
     physical_offset = offset_addr-virtual_addr;
     if ((UINTPTR_MAX-base_addr) < physical_offset) return 0; // offset is above the available memory space.
     else return reinterpret_cast<T>(base_addr+physical_offset);
  }
}

void softswitch_init(ThreadCtxt_t* thr_ctxt)
{
     for (uint32_t i=1; i < (1<<TinselLogMsgsPerThread); i++) tinselAlloc(tinselSlot(i)); // allocate receive slots.
     // we are going to pack the structures together as tightly as we can, obeying any alignment restrictions
     thr_ctxt->devTyps = offset_ptr<devTyp_t*>(thr_ctxt, thr_ctxt->devTyps);
     thr_ctxt->devInsts = offset_ptr<devInst_t*>(thr_ctxt, thr_ctxt->devInsts);
     thr_ctxt->properties = offset_ptr<const void*>(thr_ctxt, thr_ctxt->properties);
     for (uint32_t deviceType = 0; deviceType < thr_ctxt->numDevTyps; deviceType++) deviceType_init(deviceType, thr_ctxt);
     for (uint32_t device = 0; device < thr_ctxt->numDevInsts; device++) device_init(&thr_ctxt->devInsts[device], thr_ctxt);
}

void softswitch_finalize(ThreadCtxt_t* thr_ctxt, volatile void** send_buf, volatile void** recv_buf)
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
     *send_buf = *recv_buf = 0;
     // free mailbox slots
     for (uint32_t i = 0; i < (1<<TinselLogMsgsPerThread); i++) tinselAlloc(tinselSlot(i));
}

void softswitch_barrier(ThreadCtxt_t* thr_ctxt, volatile void* send_buf, volatile void* recv_buf)
{
     // first phase of barrier: set up a standard message to send to the supervisor
     uint32_t this_thread = tinselId();
     uint32_t supervisor =  (this_thread & (P_BOX_MASK | P_BOARD_MASK)) | P_SUP_MASK;
     // block until we can send it,
     while (!tinselCanSend());
     encode_pkt_hdr(this_thread, supervisor, P_PKT_MSGTYP_BARRIER, const_cast<void*>(send_buf));
     tinselSetLen(P_PKT_HDR_SIZE);
     // and then issue the message indicating this thread's startup is complete.
     tinselSend(supervisor, send_buf);
     // second phase of barrier: now wait for the supervisor's response
     thr_ctxt->ctlEnd = 1;
     while (thr_ctxt->ctlEnd)
     {
           // by blocking awaiting a receive 
           while (!tinselCanRecv());
	   recv_buf = tinselRecv();
	   Ppkt_hdr_t* rcv_pkt = static_cast<Ppkt_hdr_t*>(const_cast<void*>(recv_buf));
	   // ignore any traffic from non-supervisor devices,
	   if (rcv_pkt->src & P_SUP_MASK)
	   {
	      // and once it's been received, process it as a startup message
	      softswitch_onReceive(thr_ctxt, recv_buf);
	      thr_ctxt->ctlEnd = 0;
	   }
	   tinselAlloc(recv_buf);
     }				  
}

void deviceType_init(uint32_t deviceType_num, ThreadCtxt_t* thr_ctxt)
{
     devTyp_t* deviceType = &thr_ctxt->devTyps[deviceType_num];
     /*
     Handler addresses ought to reside in instruction memory and thus remain valid: to be seen.
     deviceType->RTS_Handler = RTS_Handlers[thr_ctxt->threadID.PThread][deviceType_num]; // RTS_Handlers is a table that must be built by the Orchestrator.
     deviceType->OnIdle_Handler = OnIdle_Handlers[thr_ctxt->threadID.PThread][deviceType_num];
     deviceType->OnCtl_Handler = OnCtl_Handlers[thr_ctxt->threadID.PThread][deviceType_num]; // OnCtl_Handler should amongst other things handle monitoring.
     */
     deviceType->outputTypes = offset_ptr<out_pintyp_t*>(thr_ctxt, deviceType->outputTypes);
     deviceType->inputTypes = offset_ptr<in_pintyp_t*>(thr_ctxt, deviceType->inputTypes);
     // for (uint32_t o = 0; o < deviceType->numOutputTypes; o++) outputPinType_init(o, deviceType_num, thr_ctxt);
     // for (uint32_t i = 0; i < deviceType->numInputTypes; i++) inputPinType_init(i, deviceType_num, thr_ctxt);
}

void device_init(devInst_t* device, ThreadCtxt_t* thr_ctxt)
{
     device->thread = thr_ctxt;
     device->devType = offset_ptr<const devTyp_t*>(thr_ctxt, device->devType);
     device->properties = offset_ptr<const void*>(thr_ctxt, device->properties);
     device->state = offset_ptr<void*>(thr_ctxt, device->state);
     device->outputPins = offset_ptr<outPin_t*>(thr_ctxt, device->outputPins);
     device->inputPins = offset_ptr<inPinMsg_t*>(thr_ctxt, device->inputPins);
     for (uint32_t out = 0; out < device->numOutputs; out++) outPinTgt_init(out, device, thr_ctxt); 
     for (uint32_t msg_typ = 0; msg_typ < device->numInputs; msg_typ++) inPin_init(msg_typ, device, thr_ctxt);
}

void outPinTgt_init(uint32_t pin, devInst_t* device, ThreadCtxt_t* thr_ctxt)
{
     outPin_t* o_pin = &device->outputPins[pin];
     o_pin->device = device;
     o_pin->pinType = offset_ptr<out_pintyp_t*>(thr_ctxt, o_pin->pinType);
     o_pin->targets = offset_ptr<const uint32_t*>(thr_ctxt, o_pin->targets);
     // initialise the message queue circular buffers. Buffer size 0 implies no initialisation.
     for (int buf = 0; buf < P_MSG_Q_MAXCOUNT; buf++)
     { 
         if (buf+1 != P_MSG_Q_MAXCOUNT) o_pin->msg_q_buf[buf].next = &o_pin->msg_q_buf[buf+1];
         else o_pin->msg_q_buf[P_MSG_Q_MAXCOUNT-1].next = &o_pin->msg_q_buf[0];
	 if (buf) o_pin->msg_q_buf[buf].prev = &o_pin->msg_q_buf[buf-1];
	 else o_pin->msg_q_buf[0].prev = &o_pin->msg_q_buf[P_MSG_Q_MAXCOUNT-1];
     }
}

void inPin_init(uint32_t pin, devInst_t* device, ThreadCtxt_t* thr_ctxt)
{
     inPinMsg_t* i_pin = &device->inputPins[pin];
     i_pin->device = offset_ptr<const devInst_t*>(thr_ctxt, i_pin->device);
     i_pin->sources = offset_ptr<inPinSrc_t*>(thr_ctxt, i_pin->sources);
     for (int src = 0; src < i_pin->numSrcs; src++) inPinSrc_init(src, i_pin, thr_ctxt);
}

void inPinSrc_init(uint32_t src, inPinMsg_t* pin, ThreadCtxt_t* thr_ctxt)
{
     inPinSrc_t* i_src = &pin->sources[src];
     i_src->pinType = offset_ptr<const in_pintyp_t*>(thr_ctxt, i_src->pinType);
     i_src->properties = offset_ptr<const void*>(thr_ctxt, i_src->properties);
     i_src->state = offset_ptr<void*>(thr_ctxt, i_src->state);
}

int softswitch_onSend(ThreadCtxt_t* thr_ctxt, volatile void* send_buf)
{
    devInst_t* cur_device = thr_ctxt->RTSHead;
    outPin_t* cur_pin = cur_device->RTSPinHead;
    uint32_t buffered = 0; // additional argument for OnSend - could also use the msgType field
    // set up OnSend with the first address   
    if (cur_device->currTgt == 0)
    {
       encode_pkt_hdr(tinselId() | cur_device->deviceID, 0, cur_pin->pinType->msgType, const_cast<void*>(send_buf));
       tinselSetLen(P_PKT_HDR_SIZE + cur_pin->pinType->sz_msg);
       // if we are buffering, is there a message in the buffer?
       if (cur_pin->msg_q_head)
       {
	  // if so, move its contents into the hardware mailbox
	  Pmsg_t* msg = softswitch_popMsg(cur_pin);
	  memcpy(static_cast<char*>(const_cast<void*>(send_buf))+P_PKT_HDR_SIZE, &msg->msg, cur_pin->pinType->sz_msg);
	  buffered = 1;
       }
       // then run the application's OnSend (which, if we are buffering, may alter the buffer again)
       uint32_t RTS_updated = cur_pin->pinType->Send_Handler(thr_ctxt->properties, cur_device, const_cast<void*>(send_buf), buffered);
    }
    // send the message (to as many destinations as possible before the network blocks).
    while (tinselCanSend() && cur_device->currTgt < cur_pin->numTgts)
          tinselSend(cur_pin->targets[cur_device->currTgt++], send_buf);
    // then update the RTS list as necessary
    softswitch_onRTS(thr_ctxt, cur_device);
    return cur_device->currTgt;
}

void softswitch_onReceive(ThreadCtxt_t* thr_ctxt, volatile void* recv_buf)
{
     // first need to do some basic decode of the packet:
     Ppkt_hdr_t* rcv_pkt = static_cast<Ppkt_hdr_t*>(const_cast<void*>(recv_buf));
     devInst_t* recv_device_begin = thr_ctxt->devInsts;
     devInst_t* recv_device_end = thr_ctxt->devInsts+thr_ctxt->numDevInsts;
     // device in range? A Supervisor packet may target the entire group by sending a device number above the max.
     if (((rcv_pkt->hdw & P_DEVICE_MASK) >> P_DEVICE_OS) < thr_ctxt->numDevInsts)
        recv_device_end = (recv_device_begin =  &thr_ctxt->devInsts[(rcv_pkt->hdw & P_DEVICE_MASK) >> P_DEVICE_OS]) + 1;
     else if (!(rcv_pkt->hdw & P_SUP_MASK)) return; // exit and dump packet if the device is out of range.
     uint32_t msg_type, RTS_updated;
     // what message type does it contain?
     memcpy(&msg_type, &rcv_pkt->housekeeping[P_PKT_MSGTYP_OS], sizeof(uint32_t));
     // go through the devices (usually only 1)
     for (devInst_t* recv_device = recv_device_begin; recv_device != recv_device_end; recv_device++)
     {
        // which pin will receive the message?
        inPinSrc_t* recv_pin = find_inputPin(recv_device, msg_type, rcv_pkt->src);
	// source was a supervisor? Run OnCtl. We assume here full context (thread, device) should be passed.
	if (rcv_pkt->hdw & P_SUP_MASK) RTS_updated = recv_device->devType->OnCtl_Handler(thr_ctxt, recv_device, const_cast<const void*>(recv_buf)+sizeof(Ppkt_hdr_t));
        // otherwise handle as a normal device through the appropriate receive handler.
        else RTS_updated = recv_pin->pinType->Recv_handler(thr_ctxt->properties, recv_device, recv_pin, const_cast<const void*>(recv_buf)+sizeof(Ppkt_hdr_t));
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
  uint32_t rts[P_MAX_OPINS_PER_DEVICE>>5];
  void* rts_buf[P_MAX_OPINS_PER_DEVICE];
  uint32_t num_grps = device->numOutputs >> 5;
  uint32_t pin_grp, pin, pins_this_grp;
  // first need to deal with the active sending device. If it's finished sending
  // a message, we remove it from the queue before handling new insertions.
  outPin_t* cur_pin = 0;
  if (device == thr_ctxt->RTSHead)
  {
     cur_pin = device->RTSPinHead;
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
	  rts_buf[32*pin_grp + pin] = &device->outputPins[32*pin_grp + pin].msg_q_tail->msg;
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
	        Pmsg_t* msg_buf;
	        // try to get a buffer for the message and append if we succeed.
	        if (msg_buf = softswitch_nextMsg(output_pin)) softswitch_pushMsg(output_pin, msg_buf);
	        // pin not already active. Insert into the queue.
	        if (!(output_pin->RTSPinPrev || output_pin->RTSPinNext)) softswitch_pushRTSPin(device, output_pin);
	     }
         }
     }
  }
  // rotate the currently active pin round to the back of the device's list if it still has messages to send
  if (cur_pin && cur_pin->msg_q_head) softswitch_pushRTSPin(device, cur_pin);
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
      if (thr_ctxt->RTSHead = popped->RTSNext) thr_ctxt->RTSHead->RTSPrev = 0;
      popped->RTSNext = 0; // zero out the popped device's linked-list pointer to remove it entirely from the list.
      return popped;
}

void softswitch_pushRTSPin(devInst_t* device, outPin_t* new_pin)
{
     new_pin->RTSPinNext = 0;
     new_pin->RTSPinPrev = device->RTSPinTail;
     // need to test head because it will have to be set independently if it is null.
     if (!device->RTSPinTail) device->RTSPinHead = new_pin;
     // only if head is nonzero will tail have a meaningful value to set.
     else device->RTSPinTail->RTSPinNext = new_pin;
     device->RTSPinTail = new_pin;
}
       
outPin_t* softswitch_popRTSPin(devInst_t* device)
{
      outPin_t* popped = device->RTSPinHead;
      // single = in the if-conditional is correct here: we are setting and then testing for 0 having set.
      if (device->RTSPinHead = popped->RTSPinNext) device->RTSPinHead->RTSPinPrev = 0;
      return popped;
}

/* these 3 operations work on a doubly-linked circular buffer. Buffer members
   are kept in order so we can always insert a new buffer into the active list
   by simply appending the next available one. If the next one is also
   the head of the active list, the buffer is exhausted.
 */
void softswitch_pushMsg(outPin_t* pin, Pmsg_t* msg)
{
     pin->msg_q_tail = msg; // can always insert; no need for pointer juggling. 
     if (!pin->msg_q_head) pin->msg_q_head = msg; // if this is the first member it should be the head as well.
}

Pmsg_t* softswitch_popMsg(outPin_t* pin)
{
     Pmsg_t* popped = pin->msg_q_head;
     // if this is the last active member then zero the list.
     if (pin->msg_q_head == pin->msg_q_tail) pin->msg_q_head = pin->msg_q_tail = 0;
     else pin->msg_q_head = popped->next;
     return popped;
}

Pmsg_t* softswitch_nextMsg(outPin_t* pin)
{
     // no buffer available. 
     if (!pin->msg_q_buf) return 0;
     // queue empty - can use the first available message.
     if (!pin->msg_q_head) return &pin->msg_q_buf[0];
     // buffer exhausted - no available messages. 
     if (pin->msg_q_tail->next == pin->msg_q_head) return 0;
     else return pin->msg_q_tail->next; // otherwise give us a message.
}
	 
uint32_t encode_address(const address_t* addr)
{
      // some basic bounds checking: return 0 if the address is out of range. 
      if ((addr->PBox > (P_BOX_MASK >> P_BOX_OS)) || (addr->PBoard > (P_BOARD_MASK >> P_BOARD_OS)) || (addr->PCore > (P_CORE_MASK >> P_CORE_OS)) || (addr->PThread > (P_THREAD_MASK >> P_THREAD_OS)) || (addr->PDevice > (P_DEVICE_MASK >> P_DEVICE_OS))) return 0;
      // otherwise concatenate subfields.
      return (addr->PBox << P_BOX_OS) | (addr->PBoard << P_BOARD_OS) | (addr->PCore << P_CORE_OS) | (addr->PThread << P_THREAD_OS) | (addr->PDevice << P_DEVICE_OS) | (addr->Supervisor ? P_SUP_MASK : 0);
}
 
address_t decode_address(uint32_t hw_addr)
{
      address_t decoded;
      decoded.PBox = (hw_addr & P_BOX_MASK) >> P_BOX_OS;
      decoded.PBoard = (hw_addr & P_BOARD_MASK) >> P_BOARD_OS;
      decoded.PCore = (hw_addr & P_CORE_MASK) >> P_CORE_OS;
      decoded.PThread = (hw_addr & P_THREAD_MASK) >> P_THREAD_OS;
      decoded.PDevice = (hw_addr & P_DEVICE_MASK) >> P_DEVICE_OS;
      decoded.Supervisor = (hw_addr & P_SUP_MASK); 
      return decoded;
}

void encode_pkt_hdr(uint32_t src, uint32_t dst, uint32_t msgtyp, void* buf)
{
     Ppkt_hdr_t* pkt = static_cast<Ppkt_hdr_t*>(buf);
     pkt->hdw = dst;
     pkt->src = src;
     memcpy(&pkt->housekeeping[P_PKT_MSGTYP_OS], &msgtyp, sizeof(uint32_t));
}

void encode_pkt_dst(uint32_t dst, void* buf)
{
     Ppkt_hdr_t* pkt = static_cast<Ppkt_hdr_t*>(buf);
     pkt->hdw = dst;
}

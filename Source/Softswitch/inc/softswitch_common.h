#ifndef _SOFTSWITCH_COMMON_H_
#define _SOFTSWITCH_COMMON_H_

#include <cstdint>
#include <string>
#include <map>
#include <vector>

#define P_BOX_OS 26
#define P_BOARD_OS 22
#define P_CORE_OS 16
#define P_THREAD_OS 11
#define P_SUP_OS 10
#define P_DEVICE_OS 0
#define P_BOX_MASK 0xFC000000
#define P_BOARD_MASK 0x3C00000
#define P_CORE_MASK 0x3F0000
#define P_THREAD_MASK 0xF800
#define P_SUP_MASK 0x400
#define P_DEVICE_MASK 0x2FF
#define P_PKT_HDR_SIZE 24
#define P_PKT_SIZE 128
#define P_PYLD_SIZE P_PKT_SIZE-P_PKT_HDR_SIZE
#define P_PKT_CONTROL_OS 0
#define P_PKT_LEN_OS 1
#define P_PKT_PRIORITY_OS 2
#define P_PKT_HOP_OS 3
#define P_PKT_SPLIT_OS 5
#define P_PKT_SEND_PORT_OS 7
#define P_PKT_RECV_PORT_OS 8
#define P_PKT_MSGTYP_OS 10
#define P_PKT_MSGTYP_BARRIER 0x10000
#define P_MAX_OPINS_PER_DEVICE 32
#define P_MSG_Q_MAXCOUNT 1
#define P_ONIDLE_CHANGE 0x80000000
#define IDLE_SWEEP_CHUNK_SIZE 128

struct PDeviceInstance;
struct PThreadContext;

struct address_t
{
  uint32_t PBox;
  uint32_t PBoard;
  uint32_t PCore;
  uint32_t PThread;
  uint32_t PDevice;
  bool     Supervisor;
};

typedef struct Ppkt_hdr
{
  uint32_t hdw;
  uint32_t bch;
  uint32_t src;
  char     housekeeping[14];
} Ppkt_hdr_t;

typedef struct P_Pkt
{
  Ppkt_hdr_t hdr;
  char payload[P_PYLD_SIZE];
} P_Pkt_t;

typedef struct Pmsg
{
  P_Pkt_t msg;
  Pmsg* prev;
  Pmsg* next;
} Pmsg_t;

typedef uint32_t (*RTS_handler_t)
(const void* graphProps,
 void*       device,
 uint32_t*   readyToSend,
 void**      msg_buf
);

typedef uint32_t (*OnIdle_handler_t)
(const void* graphProps,
 void*       device
);

typedef uint32_t (*OnCtl_handler_t)
(const void* graphProps,
 void*       device,
 const void* msg
);

typedef uint32_t (*Recv_handler_t)
(const void* graphProps,
 void*       device,
 void*       edge,
 const void* msg
);

typedef uint32_t (*Send_handler_t)
(const void* graphProps,
 void*       device,
 void*       msg,
 uint32_t    buffered
);

typedef struct PInputType
{
  Recv_handler_t Recv_handler;
  uint32_t       sz_msg;
  uint32_t       msgType;
  uint32_t       sz_props;
  uint32_t       sz_state;
} in_pintyp_t;

typedef struct POutputType
{
  Send_handler_t Send_Handler;
  uint32_t       sz_msg;
  uint32_t       msgType;
} out_pintyp_t;

typedef struct PDeviceType
{
  RTS_handler_t     RTS_Handler;
  OnIdle_handler_t  OnIdle_Handler;
  OnCtl_handler_t   OnCtl_Handler;
  uint32_t          sz_props;
  uint32_t          sz_state;
  uint32_t          numInputTypes;
  in_pintyp_t*      inputTypes;
  uint32_t          numOutputTypes;
  out_pintyp_t*     outputTypes;
} devTyp_t;

typedef struct POutputPin
{
  PDeviceInstance*  device;
  out_pintyp_t*     pinType;
  Pmsg_t            msg_q_buf[P_MSG_Q_MAXCOUNT];
  Pmsg_t*           msg_q_head;
  Pmsg_t*           msg_q_tail;
  uint32_t          numTgts;
  const uint32_t*   targets;
  POutputPin*       RTSPinPrev;
  POutputPin*       RTSPinNext;
} outPin_t;

/* this maps input pins by source. This enables us to look up the correct
   pin to direct an incident packet to based upon its source information.
*/ 
typedef struct PInPinSrc
{
  uint32_t           tgt;
  uint32_t           src;
  const in_pintyp_t* pinType;
  const void*        properties;
  void*              state;
} inPinSrc_t;

// this maps groups of input pins by message type

typedef struct PInPinMsg
{
  const PDeviceInstance*   device;
  uint32_t                 msgType;
  uint32_t                 numSrcs;
  inPinSrc_t*              sources;
} inPinMsg_t;
  
typedef struct PDeviceInstance
{
  PThreadContext*                     thread;
  const devTyp_t*                     devType;
  uint32_t                            deviceID;
  uint32_t                            numInputs;
    /* this maps input pins by message type. A given message type may be
     associated with several actual pins. A nested map is used in this
     case so that the lookup may be 2-stage indexed. We might also use
     a multimap and do a linear search on the inner elements (potentially
     slower). Note that the number of elements in the inTypeSources
     array need not be specified, because this can be looked up from the
     vtable structure (vtable->numInputTypes)
 */
  inPinMsg_t*                         inputPins;
  uint32_t                            numOutputs;
  outPin_t*                           outputPins;
  const void*                         properties;
  void*                               state;
  PDeviceInstance*                    RTSPrev;
  PDeviceInstance*                    RTSNext;
  outPin_t*                           RTSPinHead;
  outPin_t*                           RTSPinTail;
  uint32_t                            currTgt; // device to send to for current RTS pin
} devInst_t;

typedef struct PThreadContext
{
  PThreadContext*    virtualAddr; // used to calculate offsets at initialisation time
  uint32_t           numDevTyps;
  devTyp_t*          devTyps;
  uint32_t           numDevInsts;
  devInst_t*         devInsts;
  const void*        properties;
  devInst_t*         RTSHead;
  devInst_t*         RTSTail;
  uint32_t           nextOnIdle;
  uint32_t           receiveHasPriority;
  uint32_t           ctlEnd;
} ThreadCtxt_t;

class InputPinLookup
{
  public:
  
  InputPinLookup(const devInst_t* device);
  inPinSrc_t* find_inputPin(uint32_t msgType, uint32_t srcID);

  private:
  const devInst_t* target_device;
};

// these functions would be more cleanly done as methods of a class PThread.

/* softswitch_init should:
   1) set internal variables to their initial state (which state should
      simply be in DRAM at a fixed location)
   2) configure the internal nameserver tables by querying the main NameServer.
   3) populate the context structure
   5) place any initial messages in the RTS queues.
   6) go into barrier and await other cores in the application.
*/
template<class T> T offset_ptr(ThreadCtxt_t* base, T offset);
void softswitch_init(ThreadCtxt_t* thr_ctxt);
void softswitch_finalize(ThreadCtxt_t* thr_ctxt, volatile void** send_buf, volatile void** recv_buf);
void softswitch_barrier(ThreadCtxt_t* thr_ctxt, volatile void* send_buf, volatile void* recv_buf);
void deviceType_init(uint32_t deviceType_num, ThreadCtxt_t* thr_ctxt);
// handlers should reside in instruction memory and thus shouldn't need setup.
// inline void outputPinType_init(uint32_t pin, uint32_t dev_typ, ThreadCtxt_t* thr_ctxt) {thr_ctxt->devTyps[dev_typ].outputTypes[pin].Send_Handler = Send_Handlers[thr_ctxt->threadID.PThread][dev_typ][pin];};
// inline void inputPinType_init(uint32_t pin, uint32_t dev_typ, ThreadCtxt_t* thr_ctxt) {thr_ctxt->devTyps[dev_typ].inputTypes[pin].Recv_Handler = Recv_Handlers[thr_ctxt->threadID.PThread][dev_typ][pin];};
void device_init(devInst_t* device, ThreadCtxt_t* thr_ctxt);
void outPinTgt_init(uint32_t pin, devInst_t* device, ThreadCtxt_t* thr_ctxt);
void inPin_init(uint32_t pin, devInst_t* device, ThreadCtxt_t* thr_ctxt);
void inPinSrc_init(uint32_t src, inPinMsg_t* pin, ThreadCtxt_t* thr_ctxt);

// wrappers for the basic handlers.
// the onSend handler needs to be responsible for updating the send address and managing the RTS list.
int softswitch_onSend(ThreadCtxt_t* thr_ctxt, volatile void* send_buf);
void softswitch_onReceive(ThreadCtxt_t* thr_ctxt, volatile void* recv_buf);
bool softswitch_onIdle(ThreadCtxt_t* thr_ctxt);
void softswitch_onRTS(ThreadCtxt_t* thr_ctxt, devInst_t* device);

// utility functions to manage ready-to-send queue 
inline bool softswitch_IsRTSReady(ThreadCtxt_t* thr_ctxt) {return thr_ctxt->RTSHead != 0;};
void softswitch_pushRTS(ThreadCtxt_t* thr_ctxt, devInst_t* new_device);
devInst_t* softswitch_popRTS(ThreadCtxt_t* thr_ctxt);
void softswitch_pushRTSPin(devInst_t* device, outPin_t* new_pin);
outPin_t* softswitch_popRTSPin(devInst_t* device);
void softswitch_pushMsg(outPin_t* pin, Pmsg_t* msg);
Pmsg_t* softswitch_popMsg(outPin_t* pin);
Pmsg_t* softswitch_nextMsg(outPin_t* pin);

// packet encode/decode
uint32_t encode_address(const address_t* addr);
address_t decode_address(uint32_t hw_addr);
void encode_pkt_hdr(uint32_t src, uint32_t dst, uint32_t msgtyp, void* buf);
void encode_pkt_dst(uint32_t dst, void* buf);


// input pin lookup table search
inline inPinSrc_t* find_inputPin(devInst_t* device, uint32_t msgType, uint32_t srcID) {return InputPinLookup(device).find_inputPin(msgType, srcID);};
inline bool msg_key_compare(const inPinMsg_t msgPins0, const inPinMsg_t msgPins1) {return (msgPins0.msgType < msgPins1.msgType);};
inline bool src_key_compare(const inPinSrc_t srcPin0, const inPinSrc_t srcPin1) {return (srcPin0.src < srcPin1.src);};

// workaround bodge for some unfinished business in the XML handler fragments. Should be fixed in the XML.
inline uint32_t handler_exit(uint32_t code) {return code;};

#endif

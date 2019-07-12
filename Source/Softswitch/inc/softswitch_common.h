#ifndef _SOFTSWITCH_COMMON_H_
#define _SOFTSWITCH_COMMON_H_

#include "softswitch.h"

#include "OSFixes.hpp"

#define P_MAX_OPINS_PER_DEVICE 32                   // The maximum number of output pins a device can have

#define P_MSG_Q_MAXCOUNT 1
#define P_ONIDLE_CHANGE 0x80000000
#define IDLE_SWEEP_CHUNK_SIZE 128

struct PDeviceInstance;
struct PThreadContext;
struct POutputPin;
struct PInputPin;

typedef struct p_message_q
{
    P_Msg_t msg;
    p_message_q* prev;
    p_message_q* next;
} P_Msg_Q_t;

typedef uint32_t (*RTS_handler_t)
(   const void* graphProps,
    void*       device,
    uint32_t*   readyToSend
);

typedef uint32_t (*OnIdle_handler_t)
(   const void* graphProps,
    void*       device
);

typedef uint32_t (*OnCtl_handler_t)
(   const void* graphProps,
    void*       device,
    const void* msg
);

typedef uint32_t (*Recv_handler_t)
(   const void* graphProps,
    void*       device,
    void*       edge,
    const void* msg
);

typedef uint32_t (*Send_handler_t)
(   const void* graphProps,
    void*       device,
    void*       msg
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

// this maps output edges by target (device, pin, edge_index). While
// this is in fact the index as seen by the target device, from the
// POV of the source device this may simply be seen as an opaque
// 72-bit edge identifier.

typedef struct POutputEdge
{
    POutputPin*       pin;        // back pointer to pin
    uint32_t          tgt;        // destination device
    uint32_t          tgtPin;     // destination pin
    uint32_t          tgtEdge;    // destination edge index
} outEdge_t;

typedef struct POutputPin
{
    PDeviceInstance*  device;
    out_pintyp_t*     pinType;    
    uint32_t          numTgts;
    outEdge_t*        targets;
    uint32_t          idxTgts;              // Where we are up to with sending this pin
    uint32_t          sendPending;          // Flag that the pin wants to send
} outPin_t;

// this maps input edges by pin.

typedef struct PInputEdge
{
    const PInputPin*  pin;        // back pointer to pin
    uint32_t          tgt;        // destination device (for convenience)
    uint32_t          src;        // source device (for convenience)
    const void*       properties; // globally available
    void*             state;
} inEdge_t;

// this maps input pins by device

typedef struct PInputPin
{
    const PDeviceInstance*   device;
    in_pintyp_t*             pinType;
    uint32_t                 numSrcs;
    inEdge_t*                sources;
} inPin_t;
  
typedef struct PDeviceInstance
{
    PThreadContext*                     thread;
    const devTyp_t*                     devType;
    uint32_t                            deviceID;
    uint32_t                            numInputs;
    inPin_t*                            inputPins;
    uint32_t                            numOutputs;
    outPin_t*                           outputPins;
    const void*                         properties;
    void*                               state;
} devInst_t;

typedef struct PThreadContext
{
    PThreadContext*    virtualAddr; // used to calculate offsets at initialisation time
    uint32_t           numDevTyps;
    devTyp_t*          devTyps;
    uint32_t           numDevInsts;
    devInst_t*         devInsts;
    const void*        properties;
    uint32_t           rtsBufSize;      // The size of the RTS buffer
    outPin_t**         rtsBuf;          // Pointer to the RTS buffer array
    uint32_t           rtsStart;        // index of the first pending RTS
    uint32_t           rtsEnd;          // index of the last pending RTS.   
    uint32_t           idleStart;       // index of where to start OnIdle from
    uint32_t           ctlEnd;
} ThreadCtxt_t;

// these functions would be more cleanly done as methods of a class PThread.

/* softswitch_init should:
   1) set internal variables to their initial state (which state should
      simply be in DRAM at a fixed location)
   2) configure the internal nameserver tables by querying the main NameServer.
   3) populate the context structure
   5) place any initial messages in the RTS queues.
   6) go into barrier and await other cores in the application.
*/
// template<class T> T offset_ptr(ThreadCtxt_t* base, T offset);
void softswitch_init(ThreadCtxt_t* thr_ctxt);
void softswitch_finalize(ThreadCtxt_t* thr_ctxt, volatile void** send_buf, volatile void** recv_buf, volatile void** super_buf);
// void softswitch_alive(volatile void* send_buf); // debug: send alive message to host
void softswitch_barrier(ThreadCtxt_t* thr_ctxt, volatile void* send_buf, volatile void* recv_buf);
void deviceType_init(uint32_t deviceType_num, ThreadCtxt_t* thr_ctxt);
// handlers should reside in instruction memory and thus shouldn't need setup.
// inline void outputPinType_init(uint32_t pin, uint32_t dev_typ, ThreadCtxt_t* thr_ctxt) {thr_ctxt->devTyps[dev_typ].outputTypes[pin].Send_Handler = Send_Handlers[thr_ctxt->threadID.PThread][dev_typ][pin];};
// inline void inputPinType_init(uint32_t pin, uint32_t dev_typ, ThreadCtxt_t* thr_ctxt) {thr_ctxt->devTyps[dev_typ].inputTypes[pin].Recv_Handler = Recv_Handlers[thr_ctxt->threadID.PThread][dev_typ][pin];};
void device_init(devInst_t* device, ThreadCtxt_t* thr_ctxt);
void outPin_init(uint32_t pin, devInst_t* device, ThreadCtxt_t* thr_ctxt);
void outPinTgt_init(uint32_t tgt, outPin_t* pin, ThreadCtxt_t* thr_ctxt);
void inPin_init(uint32_t pin, devInst_t* device, ThreadCtxt_t* thr_ctxt);
void inPinSrc_init(uint32_t src, inPin_t* pin, ThreadCtxt_t* thr_ctxt);

// workaround bodge for some unfinished business in the XML handler fragments. Should be fixed in the XML.
inline uint32_t handler_exit(uint32_t code) {return code;};

#endif //_SOFTSWITCH_COMMON_H_

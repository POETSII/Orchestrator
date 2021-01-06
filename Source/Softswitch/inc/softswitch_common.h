#ifndef _SOFTSWITCH_COMMON_H_
#define _SOFTSWITCH_COMMON_H_

#include "softswitch.h"
#include "OSFixes.hpp"

struct PDeviceInstance;
struct PThreadContext;
struct POutputPin;
struct PInputPin;

typedef uint32_t (*RTS_handler_t)
(   const void* __GraphProps,
    void*       __Device,
    uint32_t*   readyToSend
);

typedef uint32_t (*OnInit_handler_t)
(   const void* graphProps,
    void*       __Device
);

typedef uint32_t (*OnHWIdle_handler_t)
(   const void* __GraphProps,
    void*       __Device
);

typedef uint32_t (*OnIdle_handler_t)
(   const void* __GraphProps,
    void*       __Device
);

typedef uint32_t (*OnCtl_handler_t)
(   const void* __GraphProps,
    void*       __Device,
    uint8_t     opcode,
    const void* pkt
);

typedef uint32_t (*Recv_handler_t)
(   const void* __GraphProps,
    void*       __Device,
    void*       __Edge,
    const void* pkt
);

typedef uint32_t (*Send_handler_t)
(   const void* __GraphProps,
    void*       __Device,
    void*       pkt
);

typedef struct PInputType
{
    Recv_handler_t Recv_handler;        // Function pointer to the input type’s receive handler
    uint32_t       sz_pkt;              // Size in bytes of the packet
    uint32_t       sz_props;            // Size of the edge's properties
    uint32_t       sz_state;            // Size of the edge's state
} in_pintyp_t;

typedef struct POutputType
{
    Send_handler_t Send_Handler;        // Function pointer to the input type’s send handler
    uint32_t       sz_pkt;              // Size in bytes of the packet
} out_pintyp_t;

typedef struct PDeviceType
{
    RTS_handler_t     RTS_Handler;      // F pointer to devt’s RTS handler
    OnInit_handler_t  OnInit_Handler;   // F pointer to devt’s OnInit handler
    OnIdle_handler_t  OnIdle_Handler;   // F pointer to devt’s OnDeIdle handler
    OnHWIdle_handler_t  OnHWIdle_Handler;   // F pointer to devt’s OnHWIdle handler
    OnCtl_handler_t   OnCtl_Handler;    // F pointer to devt’s  OnCtl handler
    uint32_t          sz_props;         // Size in bytes of the device type’s properties
    uint32_t          sz_state;         // Size in bytes of the device type’s state
    uint32_t          numInputTypes;    // Number of input pin types the device type has
    in_pintyp_t*      inputTypes;       // Array of input pin types. Devices pins point to this
    uint32_t          numOutputTypes;   // Number of output pin types the device type has
    out_pintyp_t*     outputTypes;      // Array of output pin types. Devices pins point to this
} devTyp_t;

// this maps output edges by target (device, pin, edge_index). While
// this is in fact the index as seen by the target device, from the
// POV of the source device this may simply be seen as an opaque
// 72-bit edge identifier.

typedef struct POutputEdge
{
    POutputPin*       pin;                  // Back pointer to pin the pin instance
    uint32_t          hwAddr;               // Target Hardware Address
    uint32_t          swAddr;               // Target Software Address
    uint32_t          pinAddr;              // Target Pin Address
} outEdge_t;

typedef struct POutputPin
{
    PDeviceInstance*  device;               // Back pointer to the device instance
    out_pintyp_t*     pinType;              // Pointer to the pin type
    uint32_t          numEdges;             // Number of edges the pin has
    outEdge_t*        outEdges;             // Pointer to the array of output edges
    uint32_t          idxEdges;             // Index of the next edge to send on
    uint32_t          sendPending;          // Flag indicating the pin wants to send
} outPin_t;

// this maps input edges by pin.

typedef struct PInputEdge
{
    const PInputPin*  pin;              // Back pointer to pin the pin instance
    const void*       properties;       // Pointer to the edge properties
    void*             state;            // Pointer to the edge state
} inEdge_t;

// this maps input pins by device

typedef struct PInputPin
{
    const PDeviceInstance*   device;    // Back pointer to the device instance
    in_pintyp_t*             pinType;   // Pointer to the pin type
    uint32_t                 numEdges;  // Number of sources
    inEdge_t*                inEdges;     // Pointer to array of input edges
} inPin_t;

typedef struct PDeviceInstance
{
    PThreadContext*     thread;          // Back pointer to the ThreadContext
    const devTyp_t*     devType;         // Pointer to the Device Instance
    uint32_t            deviceID;        // Thread-unique device ID
    uint32_t            numInputs;       // Number of inputs the device has
    inPin_t*            inputPins;       // Pointer to the inputPin array
    uint32_t            numOutputs;      // Number of outputs the device has
    outPin_t*           outputPins;      // Pointer to the outputPin array
    const void*         properties;      // Pointer to the device's properties
    void*               state;           // Pointer to the device's state
} devInst_t;

typedef struct PThreadContext
{
    uint32_t            numDevTyps;
    devTyp_t*           devTyps;
    uint32_t            numDevInsts;
    devInst_t*          devInsts;
    const void*         properties;
    uint32_t            rtsBuffSize;      // The size of the RTS buffer
    outPin_t**          rtsBuf;          // Pointer to the RTS buffer array
    uint32_t            rtsStart;        // index of the first pending RTS
    uint32_t            rtsEnd;          // index of the last pending RTS
    uint32_t            idleStart;       // index of where to start OnIdle from
    uint32_t            ctlEnd;

    // Instrumentation
    uint32_t            lastCycles;         // cached last cycle count
    uint32_t            pendCycles;         // Is there an instrumentation update pending? 2=yes, 1=claimed, 0=no
    uint32_t            txCount;            // Number of actual packets sent
    uint32_t            superCount;         // Number of supervisor packets sent
    uint32_t            rxCount;            // Number of actual packets received
    uint32_t            txHandlerCount;     // Number of On Send handler called
    uint32_t            rxHandlerCount;     // Number of On Receive handler called
    uint32_t            idleCount;          // number of times  Idle branch
    uint32_t            idleHandlerCount;   // Number of OnIdle handlers called.
    uint32_t            blockCount;         // Number of times we have been blocked by being unable to send
    uint32_t            cycleIdx;           // Update index

#if TinselEnablePerfCount == true
    // Optional Tinsel Instrumentation
    uint32_t            lastmissCount;         // Cache miss count
    uint32_t            lasthitCount;          // Cache hit count
    uint32_t            lastwritebackCount;    // Cache writeback count
    uint32_t            lastCPUIdleCount;      // CPU idle-cycle count (lower 32 bits)
#endif
} ThreadCtxt_t;


void softswitch_loop(ThreadCtxt_t* ThreadContext);

void softswitch_init(ThreadCtxt_t* ThreadContext);
void softswitch_finalise(ThreadCtxt_t* ThreadContext);

void softswitch_barrier(ThreadCtxt_t* ThreadContext);


void deviceType_init(uint32_t deviceType_num, ThreadCtxt_t* thr_ctxt);
void device_init(devInst_t* device, ThreadCtxt_t* thr_ctxt);

void softswitch_instrumentation(ThreadCtxt_t* thr_ctxt, volatile void* send_buf);
uint32_t softswitch_onSend(ThreadCtxt_t* thr_ctxt, volatile void* send_buf);
void softswitch_onReceive(ThreadCtxt_t* ThreadContext, volatile void* recv_buf);
bool softswitch_onIdle(ThreadCtxt_t* thr_ctxt);
uint32_t softswitch_onRTS(ThreadCtxt_t* thr_ctxt, devInst_t* device);


// utility functions to manage ready-to-send queue
inline bool softswitch_IsRTSReady(ThreadCtxt_t* ThreadContext) {return (ThreadContext->rtsStart != ThreadContext->rtsEnd);};

// workaround bodge for some unfinished business in the XML handler fragments. Should be fixed in the XML.
inline uint32_t handler_exit(uint32_t code) {return code;};

#endif //_SOFTSWITCH_COMMON_H_

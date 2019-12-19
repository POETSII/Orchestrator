#include "softswitch_common.h"
#include "tinsel.h"
#include <cstring>
#include "stdint.h"

#define DEBUG_INIT_ENTERED 0xB0
#define DEBUG_INIT_TINSEL_RECV_IN 0xB1
#define DEBUG_INIT_TINSEL_RECV_OUT 0xB2
#define DEBUG_INIT_TINSEL_DEVTS_IN 0xB3
#define DEBUG_INIT_TINSEL_DEVTS_OUT 0xB4
#define DEBUG_INIT_TINSEL_DEVS_IN 0xB5
#define DEBUG_INIT_TINSEL_DEV_DEBUG_NUMS 0xB6
#define DEBUG_INIT_TINSEL_DEV_OUTPUT_PINS_BEGIN 0xB7
#define DEBUG_INIT_TINSEL_DEV_OUTPUT_PIN 0xB8
#define DEBUG_INIT_TINSEL_DEV_INPUT_PINS_BEGIN 0xB9
#define DEBUG_INIT_TINSEL_DEV_INPUT_PIN_1 0xBA
#define DEBUG_INIT_TINSEL_DEV_INPUT_PIN_2 0xBB
#define DEBUG_INIT_TINSEL_DEV_INPUT_PIN_3 0xBC
#define DEBUG_INIT_TINSEL_DEV_INPUT_PIN_4 0xBD
#define DEBUG_INIT_TINSEL_DEVS_OUT 0xBE
#define DEBUG_INIT_WE_OUT_YO 0xBF

#define DEBUG_INIT_TINSEL_DEV_GMB_0 0xD0
#define DEBUG_INIT_TINSEL_DEV_GMB_1 0xD1
#define DEBUG_INIT_TINSEL_DEV_GMB_2 0xD2
#define DEBUG_INIT_TINSEL_DEV_GMB_3 0xD3
#define DEBUG_INIT_TINSEL_DEV_GMB_4 0xD4

#define DEBUG_BARRIER_ENTERED 0xC0
#define DEBUG_BARRIER_SENDING_MESSAGE 0xC1
#define DEBUG_BARRIER_SENT_MESSAGE 0xC2
#define DEBUG_BARRIER_RECEIVED_A_MESSAGE_OF_SOME_KIND 0xC3
#define DEBUG_BARRIER_ITS_A_VALID_PACKET 0xC4
#define DEBUG_BARRIER_ITS_NOT_A_VALID_PACKET 0xC5
#define DEBUG_BARRIER_WE_OUT_YO 0xCF

#define DEBUG_RECV_OOR_START 0xE0
#define DEBUG_RECV_OOR_END 0xE1

typedef struct device_properties_wharrgarbl
{
float ignored;
uint16_t ignoredagain;
uint32_t x;
uint32_t y;
} device_properties_wharrgarbl_t;

void softswitch_init(ThreadCtxt_t* ThreadContext)
{
// MLV: We in
    while(!tinselUartTryPut(DEBUG_INIT_ENTERED));

    union {
        uint32_t something;
        uint8_t asBytes[4];
    } uartStuff;
    uartStuff.something = ThreadContext->numDevInsts;
    for (int uartIndex = 0; uartIndex <= 3; uartIndex++)
    {
        while (!tinselUartTryPut(uartStuff.asBytes[3 - uartIndex]));
    }

    // Allocate Tinsel receive slots.
    while(!tinselUartTryPut(DEBUG_INIT_TINSEL_RECV_IN));
    for (uint32_t i=P_RXSLOT_START; i < (1<<TinselLogMsgsPerThread); i++)
    {
        tinselAlloc(tinselSlot(i)); // allocate receive slots.
    }
    while(!tinselUartTryPut(DEBUG_INIT_TINSEL_RECV_OUT));

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
    while(!tinselUartTryPut(DEBUG_INIT_TINSEL_DEVTS_IN));
    for (uint32_t deviceType = 0;
            deviceType < ThreadContext->numDevTyps;
            deviceType++)
    {
        deviceType_init(deviceType, ThreadContext);
    }
    while(!tinselUartTryPut(DEBUG_INIT_TINSEL_DEVTS_OUT));

    // Initialise devices
    while(!tinselUartTryPut(DEBUG_INIT_TINSEL_DEVS_IN));
    uint32_t deviceX, deviceY;
deviceX = static_cast<device_properties_wharrgarbl_t*>(const_cast<void*>(ThreadContext->devInsts[0].properties))->x;
deviceY = static_cast<device_properties_wharrgarbl_t*>(const_cast<void*>(ThreadContext->devInsts[0].properties))->y;

    uartStuff.something = deviceX;
    for (int uartIndex = 0; uartIndex <= 3; uartIndex++)
    {
        while (!tinselUartTryPut(uartStuff.asBytes[3 - uartIndex]));
    }
    uartStuff.something = deviceY;
    for (int uartIndex = 0; uartIndex <= 3; uartIndex++)
    {
        while (!tinselUartTryPut(uartStuff.asBytes[3 - uartIndex]));
    }
    uartStuff.something = ThreadContext->rtsBufSize;
    for (int uartIndex = 0; uartIndex <= 3; uartIndex++)
    {
        while (!tinselUartTryPut(uartStuff.asBytes[3 - uartIndex]));
    }


    for (uint32_t device = 0; device < ThreadContext->numDevInsts; device++)
    {
        device_init(&ThreadContext->devInsts[device], ThreadContext);
    }
    while(!tinselUartTryPut(DEBUG_INIT_TINSEL_DEVS_OUT));

    // MLV: We out
    while(!tinselUartTryPut(DEBUG_INIT_WE_OUT_YO));
}


/*------------------------------------------------------------------------------
 * softswitch_barrier: Block until told to continue by the mothership
 *----------------------------------------------------------------------------*/
void softswitch_barrier(ThreadCtxt_t* ThreadContext)
{
    while(!tinselUartTryPut(DEBUG_BARRIER_ENTERED));

    // Create RX buffer pointer and get Tinsel Slots
    volatile void *recv_buf = PNULL;
    volatile void *send_buf = tinselSlot(P_MSG_SLOT);     // Send slot

    // first phase of barrier: set up a standard message to send to the supervisor
    volatile P_Msg_Hdr_t* hdr = static_cast<volatile P_Msg_Hdr_t*>(send_buf); // Header

    // block until we can send it,
    while (!tinselCanSend());

    // Set a Supervisor header with the correct Opcode.
    hdr->swAddr = P_SW_MOTHERSHIP_MASK | P_SW_CNC_MASK;
    hdr->swAddr |= ((P_CNC_BARRIER << P_SW_OPCODE_SHIFT) & P_SW_OPCODE_MASK);
    hdr->pinAddr = tinselId();          // usurp Pin Addr for the source HW addr

    // MLV: Debugging - send some information over the UART link to the
    // supervisor.
    union {
        uint32_t myAddr;
        uint8_t asBytes[4];
    } uartStuffBig;

    // MLV: The host ID
    uartStuffBig.myAddr = tinselId();
    for (int uartIndex = 0; uartIndex <= 3; uartIndex++)
    {
        while (!tinselUartTryPut(uartStuffBig.asBytes[3 - uartIndex]));
    }

    // MLV: Informing supervisor that we're about to send our message.
    while(!tinselUartTryPut(DEBUG_BARRIER_SENDING_MESSAGE));

    // and then issue the message indicating this thread's startup is complete.
    tinselSetLen(p_hdr_size());
    tinselSend(tinselHostId(), send_buf);

    // MLV: Informing supervisor that we're waiting for their response.
    while(!tinselUartTryPut(DEBUG_BARRIER_SENT_MESSAGE));

    // second phase of barrier: wait for the supervisor's response
    ThreadContext->ctlEnd = 1;
    while (ThreadContext->ctlEnd)
    {
        // by blocking awaiting a receive.
        while (!tinselCanRecv());
        recv_buf = tinselRecv();

        // MLV: Informing supervisor that we received a message of some kind.
        while(!tinselUartTryPut(DEBUG_BARRIER_RECEIVED_A_MESSAGE_OF_SOME_KIND));

        volatile P_Msg_t* rcv_pkt = static_cast<volatile P_Msg_t*>(recv_buf);
        // look for barrier message
        if (((rcv_pkt->header.swAddr & P_SW_OPCODE_MASK) >> P_SW_OPCODE_SHIFT)
                == P_CNC_INIT)
        {
            // *Debug: send packet out to show we have passed the barrier*
            // softswitch_alive(send_buf);
            // and once it's been received, process it as a startup message
            while(!tinselUartTryPut(DEBUG_BARRIER_ITS_A_VALID_PACKET));
            softswitch_onReceive(ThreadContext, recv_buf);
            ThreadContext->ctlEnd = 0;
        }

        // MLV: Bad message!
        else
        {
            while(!tinselUartTryPut(DEBUG_BARRIER_ITS_NOT_A_VALID_PACKET));
        }
        tinselAlloc(recv_buf);
    }

    // MLV: Leaving softswitch_barrier
    while(!tinselUartTryPut(DEBUG_BARRIER_WE_OUT_YO));
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
    volatile void *superBuffer = tinselSlot(P_SUPMSG_SLOT); // Supervisor send slot

#ifdef SOFTSWITCH_INSTRUMENTATION
    uint32_t cycles = tinselCycleCount();		// cycle counter is per-core
    ThreadContext->lastCycles = cycles;         // save initial cycle count
 #if TinselEnablePerfCount == true
    // Save initial extended instrumentation counts.
    ThreadContext->lastmissCount = tinselMissCount();
    ThreadContext->lasthitCount = tinselHitCount();
    ThreadContext->lastwritebackCount = tinselWritebackCount();
    ThreadContext->lastCPUIdleCount = tinselCPUIdleCount();
 #endif
#endif

    while (!ThreadContext->ctlEnd)
    {
#ifdef SOFTSWITCH_INSTRUMENTATION
        cycles = tinselCycleCount();
        if((cycles - ThreadContext->lastCycles) > P_INSTR_INTERVAL)
        {   // Trigger a message to supervisor.
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
            softswitch_instrumentation(ThreadContext, superBuffer);
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
        //softswitch_onIdle(ThreadContext);

        // Skip the block for now so that we can do SW Idle detection stuff
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

    /* More verbose debugging only on core 8 of board 0. */
    uint8_t isDebug = 0;
    if (tinselId() == 0x80) isDebug = 1;

    uint32_t numOutputs;
    uint32_t numInputs;
    union {
        uint32_t big;
        uint8_t asBytes[4];
    } uartStuff;

    if (isDebug == 1)
    {
        while(!tinselUartTryPut(DEBUG_INIT_TINSEL_DEV_DEBUG_NUMS));

        uartStuff.big = device->numOutputs;
        for (int uartIndex = 0; uartIndex <= 3; uartIndex++)
        {
            while (!tinselUartTryPut(uartStuff.asBytes[3 - uartIndex]));
        }

        uartStuff.big = device->numInputs;
        for (int uartIndex = 0; uartIndex <= 3; uartIndex++)
        {
            while (!tinselUartTryPut(uartStuff.asBytes[3 - uartIndex]));
        }

        // numOutputs = 5;
        // numInputs = 5;
    }
    numOutputs = device->numOutputs;
    numInputs = device->numInputs;

    while(!tinselUartTryPut(DEBUG_INIT_TINSEL_DEV_OUTPUT_PINS_BEGIN));
    for (uint32_t out = 0; out < numOutputs; out++)
    {
        if (isDebug == 1) while(!tinselUartTryPut(DEBUG_INIT_TINSEL_DEV_OUTPUT_PIN));
        outPin_t* o_pin = &device->outputPins[out];
        if (isDebug == 1) while(!tinselUartTryPut(DEBUG_INIT_TINSEL_DEV_GMB_0));

        o_pin->device = device;
        if (isDebug == 1)
        {
            while(!tinselUartTryPut(DEBUG_INIT_TINSEL_DEV_GMB_1));
            uartStuff.big = o_pin->numTgts;
            for (int uartIndex = 0; uartIndex <= 3; uartIndex++)
            {
                while (!tinselUartTryPut(uartStuff.asBytes[3 - uartIndex]));
            }
        }

        for (uint32_t tgt = 0; tgt < o_pin->numTgts; tgt++)
        {
            //if (isDebug == 1) while(!tinselUartTryPut(DEBUG_INIT_TINSEL_DEV_GMB_2));
            outEdge_t* i_tgt = &o_pin->targets[tgt];
            //if (isDebug == 1) while(!tinselUartTryPut(DEBUG_INIT_TINSEL_DEV_GMB_3));

            i_tgt->pin = o_pin;
            //if (isDebug == 1) while(!tinselUartTryPut(DEBUG_INIT_TINSEL_DEV_GMB_4));

        }
    }

    while(!tinselUartTryPut(DEBUG_INIT_TINSEL_DEV_INPUT_PINS_BEGIN));
    for (uint32_t msg_typ = 0; msg_typ < numInputs; msg_typ++)
    {
        if (isDebug == 1) while(!tinselUartTryPut(DEBUG_INIT_TINSEL_DEV_INPUT_PIN_1));
        inPin_t* i_pin = &device->inputPins[msg_typ];
        if (isDebug == 1) while(!tinselUartTryPut(DEBUG_INIT_TINSEL_DEV_INPUT_PIN_2));

        i_pin->device = device;
        if (isDebug == 1) while(!tinselUartTryPut(DEBUG_INIT_TINSEL_DEV_INPUT_PIN_3));
        for (uint32_t src = 0; src < i_pin->numSrcs; src++)
        {
            if (isDebug == 1) while(!tinselUartTryPut(DEBUG_INIT_TINSEL_DEV_INPUT_PIN_4));
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

    pyld->missCount = missCount - ThreadContext->lastmissCount;
    pyld->hitCount = hitCount - ThreadContext->lasthitCount;
    pyld->writebackCount = writebackCount - ThreadContext->lastwritebackCount;
    pyld->CPUIdleCount = CPUIdleCount - ThreadContext->lastCPUIdleCount;
#endif

    // Send it
    uint32_t len = p_hdr_size() + p_instrmsg_pyld_size;
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

    // Pointers of convenience
    devInst_t* device = pin->device;                                // Get the Device
    const outEdge_t* target = &pin->targets[pin->idxTgts];          // Get the target
    volatile char* buf = static_cast<volatile char*>(send_buf);    // Send Buffer
    volatile P_Msg_Hdr_t* hdr = static_cast<volatile P_Msg_Hdr_t*>(send_buf); // Header

    size_t hdrSize = p_hdr_size(); //Size of the header.

    if(pin->numTgts > 0)     // Sanity check: make sure the pin has targets
    {
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
    }

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
    else
    {
        handler_log(5, "dIDX OOR %d", devAdr);
        return;    // Message target is out of range. //TODO: - log/flag
    }


    if((recvHdr->swAddr & P_SW_CNC_MASK) && (opcode > P_CNC_MAX_USER))
    {   // Check for reserved Opcodes
        switch(opcode)
        {
            case P_CNC_STOP:    // Stop message ends the simulation
                ThreadContext->ctlEnd = 1;
                break;
#ifdef SOFTSWITCH_INSTRUMENTATION
            case P_CNC_INSTR:   // Request Instrumentation from the Softswitch
                ThreadContext->pendCycles = 1;
                break;
#endif
            case P_CNC_INIT:
                // **BODGE BODGE BODGE** not so temporary handler for dealing
                // with __init__. This works ONLY because the __init__ pin on
                // all devices that have it in existing XML happens to be pin 0
                // (which means that a Supervisor can guess what the pin number
                // is supposed to be). In any case, this should route it through
                // the __init__ handler. Luckily message types are globally
                // unique, or likewise this wouldn't work if the device had no
                // __init__ pin. This test should be removed as soon as __init__
                // pins lose any special meaning in existing XML!
                for (devInst_t* device = recvDevBegin; device != recvDevEnd; device++)
                {
                    inPin_t* pin = &device->inputPins[pinIdx];    // Get the pin
                    pin->pinType->Recv_handler(ThreadContext->properties, device, 0,
                                                static_cast<const uint8_t*>(
                                                const_cast<const void*>(recv_buf)
                                                )+p_hdr_size());
                }
                break;
            default:            // Unused reserved Opcode - log it.
                                handler_log(5, "BAD-OP %d", opcode);

        }
        return;
    }

    // Loop through each target device (1 unless a broadcast message)
    for (devInst_t* device = recvDevBegin; device != recvDevEnd; device++)
    {
        if(pinIdx >= device->numInputs)
        {   // Sanity check the pin index
            handler_log(5, "pIDX OOR %d %d", device, pinIdx);
            continue;               //TODO: - log/flag
        }
        inPin_t* pin = &device->inputPins[pinIdx];    // Get the pin

        ThreadContext->rxHandlerCount++;     // Increment received handler count


        // Supervisor/CNC/Control Message
        if(recvHdr->swAddr & P_SW_CNC_MASK)
        {  // Trigger OnCtl. Strip volatile at this point.
            device->devType->OnCtl_Handler(ThreadContext, device, opcode,
                                               static_cast<const uint8_t*>(
                                               const_cast<const void*>(recv_buf)
                                               )+p_hdr_size());
        }
        else
        {   // Handle as a normal packet
            if(edgeIdx >= pin->numSrcs)
            {   // Sanity check the Edge Index
                while(!tinselUartTryPut(DEBUG_RECV_OOR_START));
                union {
                    uint32_t something;
                    uint8_t asBytes[4];
                } uartStuff;
                uartStuff.something = edgeIdx;
                for (int uartIndex = 0; uartIndex <= 3; uartIndex++)
                {
                    while (!tinselUartTryPut(uartStuff.asBytes[3 - uartIndex]));
                }
                uartStuff.something = pin->numSrcs;
                for (int uartIndex = 0; uartIndex <= 3; uartIndex++)
                {
                    while (!tinselUartTryPut(uartStuff.asBytes[3 - uartIndex]));
                }
                while(!tinselUartTryPut(DEBUG_RECV_OOR_END));

                handler_log(5, "eIDX OOR %d %d %d", device, pinIdx, edgeIdx);
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
        //bool rtsFlagged = false;
        devInst_t* device = &ThreadContext->devInsts[idleIdx];

        if (tinselCanRecv())     // Something to RX, bail!
        {
            notIdle = true;       // return 1 as "something" has happened
            break;
        }

        if (device->devType->OnIdle_Handler(ThreadContext->properties, device))
        {
            notIdle = true;                                         // Something interesting happened
            //rtsFlagged |= softswitch_onRTS(ThreadContext, device);  // Call RTS for device
            softswitch_onRTS(ThreadContext, device);  // Call RTS for device
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
    uint32_t rts = 0; // flags for new ready-to-send pins
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

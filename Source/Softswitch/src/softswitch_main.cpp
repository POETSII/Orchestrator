#include "softswitch_common.h"
#include "tinsel.h"

void softswitch_main()
{
    // The thread context will reside at the base of the DRAM memory area.
    PThreadContext* ThreadContext = static_cast<PThreadContext*>(tinselHeapBase());
    
    // Configure rtsBuf - this sits after ThreadContext in the heap.
    // Cast as a char* to mute a warning about using void* in pointer arithmetic
    outPin_t* rtsBuf[ThreadContext->rtsBufSize+1];
    ThreadContext->rtsBuf = rtsBuf;
    
    // Create RX buffer pointer and Allocate Tinsel Slots
    volatile void *recvBuffer = PNULL;
    volatile void *sendBuffer = tinselSlot(P_MSG_SLOT);     // Send slot
    
    softswitch_init(ThreadContext);
    // send a message to the local supervisor saying we are ready;
    // then wait for the __init__ message to return by interrogating tinselCanRecv(). 
    softswitch_barrier(ThreadContext, sendBuffer, recvBuffer);
    
    // Endless main loop, that is until thread context says to stop.
    while (!ThreadContext->ctlEnd)
    {
        uint32_t cycles = tinselCycleCount();		// cycle counter is per-core
        if(!(ThreadContext->pendCycles) 
            && ((cycles - ThreadContext->lastCycles) > 250000000)) //~1s at 250 MHz
        {
          // Trigger a message to supervisor.
          ThreadContext->pendCycles = 2;
        }
        
        // Something to receive
        if (tinselCanRecv())
        {
            recvBuffer=tinselRecv();
            softswitch_onReceive(ThreadContext, recvBuffer); // decode the receive and handle
            tinselAlloc(recvBuffer); // return control of the receive buffer to the hardware
        }
        
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
        else if(!softswitch_onIdle(ThreadContext)) 
        {                                           
            tinselWaitUntil(TINSEL_CAN_RECV);
        }
    }
    softswitch_finalize(ThreadContext, &sendBuffer, &recvBuffer); // shut down once the end signal has been received.
}

int main(int argc, char** argv)
{
    softswitch_main();
    return 0;
}

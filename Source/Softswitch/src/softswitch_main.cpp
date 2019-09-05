#include "softswitch_common.h"
#include "tinsel.h"

void softswitch_main()
{
    // The thread context will reside at the base of the DRAM memory area.
    PThreadContext* ThreadContext = static_cast<PThreadContext*>(tinselHeapBase()); // softswitch_pThreadContexts + tinselID();
    // can these slot assignments be done in softswitch_init?
    volatile void *recvBuffer=0;
    volatile void *sendBuffer=tinselSlot(0);   // hardware send buffer is dedicated to the first tinsel slot
    volatile void *superBuffer[NUM_SUP_BUFS];  // buffers allocated for supervisor messages.
    for (uint32_t sb = 0; sb < NUM_SUP_BUFS; sb++) superBuffer[sb] = tinselSlot(sb+1);
    softswitch_init(ThreadContext);
    // send a message to the local supervisor saying we are ready;
    // then wait for the __init__ message to return by interrogating tinselCanRecv(). 
    softswitch_barrier(ThreadContext, superBuffer[0], recvBuffer);
    //softswitch_alive(superBuffer[0]); // *debug: say that we have passed initialisation*
    /* endless main loop. It would be nice to have some stop variable here
     * that can be set by a command. The problem is, it's not technically
     * safe to stop until all messages that should have been expected are
     * received. But a receiver cannot know which messages to expect, and
     * the Supervisor can only know that all messages have been sent. There
     * seems to be no easy way to know if all messages have been received. 
     * Best that can be done is a command from the Supervisor that sets an
     * end flag.
     */
    while (!ThreadContext->ctlEnd)
    {
        uint32_t cycles = tinselCycleCount();		// cycle counter is per-core
        if(!(ThreadContext->pendCycles) && ((cycles - ThreadContext->lastCycles) > 250000000)) //~1s at 250 MHz
        {
          // Trigger a message to supervisor.
          ThreadContext->pendCycles = 2;
        }
        
        // softswitch_alive(superBuffer[0]); // *debug: send periodic message to host*
        // handle the receive case first as the highest priority.
        if ((ThreadContext->receiveHasPriority || !softswitch_IsRTSReady(ThreadContext)) && tinselCanRecv())
        {
            // softswitch_alive(superBuffer[0]); // *debug: send periodic message to host*
            recvBuffer=tinselRecv();
            softswitch_onReceive(ThreadContext, recvBuffer); // decode the receive and handle
            tinselAlloc(recvBuffer); // return control of the receive buffer to the hardware
            ThreadContext->rxCount++;
        }
        // softswitch_IsRTSReady would be more cleanly done as a method of
        // a class PThreadContext.
        else if (softswitch_IsRTSReady(ThreadContext))
        {
            // something to send, but channel is blocked. Do whatever idle
            // processing can be achieved; if literally nothing can be done
            // wait until there is something to do.
            if (!tinselCanSend())
            {
                if (!softswitch_onIdle(ThreadContext)) tinselWaitUntil(TINSEL_CAN_SEND | TINSEL_CAN_RECV);
            }
            // otherwise deal with messages to send.
            else
            {
                // softswitch_alive(superBuffer[0]); // *debug: send periodic message to host*
                softswitch_onSend(ThreadContext, sendBuffer);
            }
        }
        else if (!softswitch_onIdle(ThreadContext)) tinselWaitUntil(TINSEL_CAN_RECV); // thread is idle.
    }
    softswitch_finalize(ThreadContext, &sendBuffer, &recvBuffer, superBuffer); // shut down once the end signal has been received.
}

int main(int argc, char** argv)
{
    softswitch_main();
    return 0;
}

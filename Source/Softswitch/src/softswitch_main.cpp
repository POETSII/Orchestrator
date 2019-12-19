#include "softswitch_common.h"
#include "tinsel.h"

#define DEBUG_MAIN_ENTERED 0xA0
#define DEBUG_MAIN_INIT_IN 0xA1
#define DEBUG_MAIN_INIT_OUT 0xA2
#define DEBUG_MAIN_BARRIER_IN 0xA3
#define DEBUG_MAIN_BARRIER_OUT 0xA4

void softswitch_main()
{
    // MLV: We in
    while(!tinselUartTryPut(DEBUG_MAIN_ENTERED));

    // The thread context will reside at the base of the DRAM memory area.
    PThreadContext* ThreadContext = static_cast<PThreadContext*>(tinselHeapBase());

    // Configure rtsBuf
    outPin_t* rtsBuf[ThreadContext->rtsBufSize+1];
    ThreadContext->rtsBuf = rtsBuf;

    // Softswitch Initialisation
    // MLV: We in
    while(!tinselUartTryPut(DEBUG_MAIN_INIT_IN));
    softswitch_init(ThreadContext);
    while(!tinselUartTryPut(DEBUG_MAIN_INIT_OUT));

    // send a message to the local supervisor saying we are ready;
    // then wait for the __init__ message to return by interrogating tinselCanRecv().
    while(!tinselUartTryPut(DEBUG_MAIN_BARRIER_IN));
    softswitch_barrier(ThreadContext);
    while(!tinselUartTryPut(DEBUG_MAIN_BARRIER_OUT));

    // Endless main loop, that is until thread context says to stop.
    softswitch_loop(ThreadContext);

    softswitch_finalise(ThreadContext); // shut down once the end signal has been received.
}

int main(int argc, char** argv)
{
    softswitch_main();
    return 0;
}

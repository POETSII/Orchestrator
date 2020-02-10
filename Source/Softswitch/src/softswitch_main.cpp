#include "softswitch_common.h"
#include "tinsel.h"

void softswitch_main()
{
    // The thread context will reside at the base of the DRAM memory area.
    PThreadContext* ThreadContext = static_cast<PThreadContext*>(tinselHeapBase());
    
    // Configure rtsBuf 
    outPin_t* rtsBuf[ThreadContext->rtsBuffSize+1];
    ThreadContext->rtsBuf = rtsBuf;
    
    // Softswitch Initialisation
    softswitch_init(ThreadContext);
    
    // send a message to the local supervisor saying we are ready;
    // then wait for the __init__ message to return by interrogating tinselCanRecv(). 
    softswitch_barrier(ThreadContext);
    
    // Endless main loop, that is until thread context says to stop.
    softswitch_loop(ThreadContext);
    
    softswitch_finalise(ThreadContext); // shut down once the end signal has been received.
}

int main(int argc, char** argv)
{
    softswitch_main();
    return 0;
}

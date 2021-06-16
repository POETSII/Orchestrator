#include "softswitch_common.h"
#include "tinsel.h"


#ifndef BACKEND_INIT
#define BACKEND_INIT(a)
#endif

#ifndef BACKEND_DEINIT
#define BACKEND_DEINIT()
#endif

#include "softswitch_common.cpp"    // Include the CPP for translation unit massaging

inline void softswitch_main()
{
    // The thread context will reside at the base of the DRAM memory area.
    PThreadContext* ThreadContext = static_cast<PThreadContext*>(tinselHeapBase());

    // Configure rtsBuf
    outPin_t* rtsBuf[ThreadContext->rtsBuffSize+1];
    ThreadContext->rtsBuf = rtsBuf;

#ifdef BUFFERING_SOFTSWITCH
    // configure the packet buffer if we are in buffering mode
    P_Pkt_pyld_t pktBuf[ThreadContext->rtsBuffSize+1];
    ThreadContext->pktBuf = pktBuf;
#endif

    // Softswitch Initialisation
    softswitch_init(ThreadContext);

    // send a packet to the local supervisor saying we are ready;
    // then wait for the __init__ packet to return by interrogating tinselCanRecv().
    softswitch_barrier(ThreadContext);

    // Endless main loop, that is until thread context says to stop.
    softswitch_loop(ThreadContext);

    softswitch_finalise(ThreadContext); // shut down once the end signal has been received.
}

int main(int argc, char** argv)
{
    BACKEND_INIT(argv)     // Backend initialisation. May not be defined.
    
    softswitch_main();
    
    BACKEND_DEINIT()       // Backend deinitialisation. May not be defined.
    return 0;
}

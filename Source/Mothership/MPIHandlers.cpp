/* This source file defines the private Mothership methods for handling
 * incoming MPI messages. The functionality of these handlers is described in
 * the Mothership documentation. They all take an input message (and a
 * communicator index, which we don't use), and returns */

#include "Mothership.h"

unsigned Mothership::HandleExit(PMsg_p* message, unsigned commIndex)
{
    threading.set_quit();
    return 1;  /* CommonBase's Decode reads this, returning from MPISpinner. */
}

unsigned Mothership::HandleSystKill(PMsg_p* message, unsigned commIndex)
{
    return 1;
}

/* And here are some stubs. */

unsigned Mothership::HandleAppSpec(PMsg_p* message, unsigned commIndex)
{
    printf("AppSpec message received!\n");
    return 0;
}
unsigned Mothership::HandleAppDist(PMsg_p* message, unsigned commIndex)
{
    printf("AppDist message received!\n"); return 0;
}
unsigned Mothership::HandleAppSupd(PMsg_p* message, unsigned commIndex)
{
    printf("AppSupd message received!\n"); return 0;
}
unsigned Mothership::HandleCmndRecl(PMsg_p* message, unsigned commIndex)
{
    printf("CmndRecl message received!\n"); return 0;
}
unsigned Mothership::HandleCmndInit(PMsg_p* message, unsigned commIndex)
{
    printf("CmndInit message received!\n"); return 0;
}
unsigned Mothership::HandleCmndRun(PMsg_p* message, unsigned commIndex)
{
    printf("CmndRun message received!\n"); return 0;
}
unsigned Mothership::HandleCmndStop(PMsg_p* message, unsigned commIndex)
{
    printf("CmndStop message received!\n"); return 0;
}
unsigned Mothership::HandleBendCnc(PMsg_p* message, unsigned commIndex)
{
    printf("BendCnc message received!\n"); return 0;
}
unsigned Mothership::HandleBendSupr(PMsg_p* message, unsigned commIndex)
{
    printf("BendSupr message received!\n"); return 0;
}
unsigned Mothership::HandlePkts(PMsg_p* message, unsigned commIndex)
{
    printf("Pkts message received!\n"); return 0;
}
unsigned Mothership::HandleDump(PMsg_p* message, unsigned commIndex)
{
    printf("Dump message received!\n"); return 0;
}

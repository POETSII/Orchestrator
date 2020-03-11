/* This source file defines the private Mothership methods for handling
 * incoming MPI messages. The functionality of these handlers is described in
 * the Mothership documentation. They all take an input message (and a
 * communicator index, which we don't use), and returns */

#include "Mothership.h"

unsigned HandleExit(PMsg_p* message, unsigned commIndex)
{
    threading.set_quit();
    return 1;  /* CommonBase's Decode reads this, returning from MPISpinner. */
}

unsigned HandleSystKill(PMsg_p* message, unsigned commIndex)
{
    return 1;
}

/* And here are some stubs. */

unsigned HandleAppSpec(PMsg_p* message, unsigned commIndex)
{
    printf("AppSpec message received!\n");
}
unsigned HandleAppDist(PMsg_p* message, unsigned commIndex)
{
    printf("AppDist message received!\n");
}
unsigned HandleAppSupd(PMsg_p* message, unsigned commIndex)
{
    printf("AppSupd message received!\n");
}
unsigned HandleCmndRecl(PMsg_p* message, unsigned commIndex)
{
    printf("CmndRecl message received!\n");
}
unsigned HandleCmndInit(PMsg_p* message, unsigned commIndex)
{
    printf("CmndInit message received!\n");
}
unsigned HandleCmndRun(PMsg_p* message, unsigned commIndex)
{
    printf("CmndRun message received!\n");
}
unsigned HandleCmndStop(PMsg_p* message, unsigned commIndex)
{
    printf("CmndStop message received!\n");
}
unsigned HandleBendCnc(PMsg_p* message, unsigned commIndex)
{
    printf("BendCnc message received!\n");
}
unsigned HandleBendSupr(PMsg_p* message, unsigned commIndex)
{
    printf("BendSupr message received!\n");
}
unsigned HandlePkts(PMsg_p* message, unsigned commIndex)
{
    printf("Pkts message received!\n");
}
unsigned HandleDump(PMsg_p* message, unsigned commIndex)
{
    printf("Dump message received!\n");
}

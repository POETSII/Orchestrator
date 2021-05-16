//------------------------------------------------------------------------------

#include "SupT_t.h"
#include "CFrag.h"
#include <stdint.h>

//==============================================================================

SupT_t::SupT_t(GraphT_t * _p,string _s):DevT_t(_p,_s)
{
devTyp  = 'S';                         // Supervisor type (override base class)
pOnPkt  = 0;                           // Packet handler
pOnRTCL = 0;                           // RTCL handler
pOnStop = 0;                           // Stop handler
pOnCTL  = 0;                           // Control handler
}

//------------------------------------------------------------------------------

SupT_t::SupT_t(DevT_t * pD)
{
devTyp  = 'S';                         // Supervisor type (override base class)
pOnPkt  = 0;                           // Packet handler
pOnRTCL = 0;                           // RTCL handler
pOnStop = 0;                           // Stop handler
pOnCTL  = 0;                           // Control handler

PinTI_v = pD->PinTI_v;                 // Input pin types
PinTO_v = pD->PinTO_v;                 // Output pin types
pPinTSI = pD->pPinTSI;                 // (Single) input pin from supervisor
pPinTSO = pD->pPinTSO;                 // (Single) output pin to supervisor
pShCd   = pD->pShCd;                   // Shared code
pOnDeId = pD->pOnDeId;                 // On device idle handler
pOnHWId = pD->pOnHWId;                 // On hardware idle handler
pOnRTS  = pD->pOnRTS;                  // On RTS handler
pOnInit = pD->pOnInit;                 // On init handler
pPropsD = pD->pPropsD;                 // Properties code
pStateD = pD->pStateD;                 // State code
devTyp  = pD->devTyp;                  // D(evice)/X(ternal)/S(uper)/U(ndef)
par     = pD->par;                     // Owning graph
Meta_v  = pD->Meta_v;                  // MetaData vector
}

//------------------------------------------------------------------------------

SupT_t::~SupT_t()
{
if (pOnPkt  !=0) delete pOnPkt;
if (pOnRTCL !=0) delete pOnRTCL;
if (pOnStop !=0) delete pOnStop;
if (pOnCTL  !=0) delete pOnCTL;
}

//------------------------------------------------------------------------------

void SupT_t::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n%sSupT_t++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sSupervisor handler code:\n",os);
fprintf(fp,"%sOnPkt          %#018lx\n",os,(uint64_t)pOnPkt);
if (pOnPkt!=0) pOnPkt->Dump(off+2,fp);
fprintf(fp,"%sOnRTCL         %#018lx\n",os,(uint64_t)pOnRTCL);
if (pOnRTCL!=0) pOnRTCL->Dump(off+2,fp);
fprintf(fp,"%sOnStop         %#018lx\n",os,(uint64_t)pOnRTCL);
if (pOnStop!=0) pOnStop->Dump(off+2,fp);
fprintf(fp,"%sOnCTL          %#018lx\n",os,(uint64_t)pOnCTL);
if (pOnCTL !=0) pOnCTL->Dump(off+2,fp);
DevT_t::Dump(off+2,fp);
fprintf(fp,"%sSupT_t--------------------------------------------------\n\n",os);
fflush(fp);
}

//==============================================================================

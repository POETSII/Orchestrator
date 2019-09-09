//------------------------------------------------------------------------------

#include "CmRTCL_t.h"
#include "PMsg_p.hpp"
#include "Pglobals.h"
#include "Root.h"

//==============================================================================

CmRTCL_t::CmRTCL_t(Root * p):par(p)
{

}

//------------------------------------------------------------------------------

CmRTCL_t::~CmRTCL_t()
{
}

//------------------------------------------------------------------------------

void CmRTCL_t::Dump(FILE * fp)
{
fprintf(fp,"CmRTCL_t++++++++++++\n");

fprintf(fp,"CmRTCL_t------------\n\n");
fflush(fp);

}

//------------------------------------------------------------------------------

unsigned CmRTCL_t::operator()(Cli * pC)
{
//printf("CmRTCL_t splitter for ....\n");
if (pC==0) return 0;                   // Paranoia
PMsg_p Msg;
unsigned cIdx = 0;
for (; cIdx < par->Comms.size(); cIdx++) {
  if (par->pPmap[cIdx]->U.RTCL != Q::NAP) {
    Msg.comm = par->Comms[cIdx];
    break;
  }
}
if (cIdx >= par->Comms.size()) {
   par->Post(161);
   return 0;                           // No RTCL, but should probably return normally
}
Msg.Put(1,&(pC->Orig));                // Command line
Msg.Src(par->Urank);                   // Me
Msg.Key(Q::RTCL);                      // Key
Msg.Send(par->pPmap[cIdx]->U.RTCL);    // Away we go
return 0;                              // Legitimate command exit
}

//==============================================================================


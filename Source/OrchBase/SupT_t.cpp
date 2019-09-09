//------------------------------------------------------------------------------

#include "SupT_t.h"
//#include "GraphT_t.h"
//#include "macros.h"
//#include "PinT_t.h"

//==============================================================================

SupT_t::SupT_t(GraphT_t * _p,string _s):DevT_t(_p,_s)
{
devTyp  = 'S';                         // Supervisor type
pOnPkt  = 0;
pOnRTCL = 0;
pOnStop = 0;
}

//------------------------------------------------------------------------------

SupT_t::~SupT_t()
{
if(pOnPkt  !=0) delete pOnPkt;
if(pOnRTCL !=0) delete pOnRTCL;
if(pOnStop !=0) delete pOnStop;
}

//------------------------------------------------------------------------------

void SupT_t::Dump(FILE * fp)
{
fprintf(fp,"SupT_t++++++++++++++++++++++++++++++++++++\n");
DevT_t::Dump(fp);
fprintf(fp,"Supervisor handler code:\n");
fprintf(fp,"OnPkt          %#08p\n",pOnPkt);
if (pOnPkt!=0) pOnPkt->Dump(fp);
fprintf(fp,"OnRTCL         %#08p\n",pOnRTCL);
if (pOnRTCL!=0) pOnRTCL->Dump(fp);
fprintf(fp,"OnStop         %#08p\n",pOnRTCL);
if (pOnStop!=0) pOnStop->Dump(fp);
fprintf(fp,"SupT_t------------------------------------\n");
fflush(fp);
}

//==============================================================================




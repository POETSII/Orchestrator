//------------------------------------------------------------------------------

#include "PinT_t.h"
#include "MsgT_t.h"

//==============================================================================

PinT_t::PinT_t(DevT_t * _p,string _s):par(_p)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
pPropsD = 0;
//pPropsI = 0;
pStateD = 0;
//pStateI = 0;
pHandl  = 0;
pMsg    = 0;
//PinPropsSize = 0;
//PinStateSize = 0;
}

//------------------------------------------------------------------------------

PinT_t::~PinT_t()
// Delete skeleton classes only
{
if(pPropsD!=0) delete pPropsD;
//if(pPropsI!=0) delete pPropsI;
if(pStateD!=0) delete pStateD;
//if(pStateI!=0) delete pStateI;
if(pHandl!=0)  delete pHandl;
}

//------------------------------------------------------------------------------

void PinT_t::Dump(FILE * fp)
{
fprintf(fp,"PinT_t++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      %#018lx,%#018lx\n",
        (uint64_t) this, (uint64_t) par);
NameBase::Dump(fp);
if(pPropsD!=0) pPropsD->Dump(fp);
else fprintf(fp,"No pin property definition\n");
//if(pPropsI!=0) pPropsI->Dump(fp);
//else fprintf(fp,"No pin property initialiser\n");
if(pStateD!=0) pStateD->Dump(fp);
else fprintf(fp,"No pin state definition\n");
//if(pStateI!=0) pStateI->Dump(fp);
//else fprintf(fp,"No pin state initialiser\n");
if(pHandl!=0)  pHandl->Dump(fp);
else fprintf(fp,"No pin event handler");
if(pMsg!=0)    pMsg->Dump(fp);
else fprintf(fp,"No pin message type\n");
fprintf(fp,"PinT_t------------------------------------\n");
fflush(fp);
}

//==============================================================================




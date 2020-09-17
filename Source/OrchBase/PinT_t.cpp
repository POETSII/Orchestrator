//------------------------------------------------------------------------------

#include "PinT_t.h"
#include "MsgT_t.h"
#include "DevT_t.h"
#include "CFrag.h"

//==============================================================================

PinT_t::PinT_t(DevT_t * _p,string _s):par(_p)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
pPropsD = 0;
pStateD = 0;
pHandl  = 0;
pMsg    = 0;
}

//------------------------------------------------------------------------------

PinT_t::~PinT_t()
// Delete skeleton classes only
{
if(pPropsD!=0) delete pPropsD;
if(pStateD!=0) delete pStateD;
if(pHandl!=0)  delete pHandl;
}

//------------------------------------------------------------------------------

void PinT_t::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n%sPinT_t++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sNameBase       %s\n",os,FullName().c_str());
fprintf(fp,"%sMe,Parent      %#018lx,%#018lx\n",
           os,(uint64_t)this,(uint64_t)par);
if(pPropsD!=0) pPropsD->Dump(off+2,fp);
else           fprintf(fp,"%sNo pin property definition\n",os);
if(pStateD!=0) pStateD->Dump(off+2,fp);
else           fprintf(fp,"%sNo pin state definition\n",os);
if(pHandl!=0)  pHandl->Dump(off+2,fp);
else           fprintf(fp,"%sNo pin event handler\n",os);
fprintf(fp,"%sPin packet type name %s\n",os,tyId.c_str());
if(pMsg!=0)    pMsg->Dump(off+2,fp);
else           fprintf(fp,"%sNo pin packet type defined\n",os);
NameBase::Dump(off+2,fp);
DefRef::Dump(off+2,fp);
fprintf(fp,"%sPinT_t--------------------------------------------------\n\n",os);
fflush(fp);
}

//==============================================================================




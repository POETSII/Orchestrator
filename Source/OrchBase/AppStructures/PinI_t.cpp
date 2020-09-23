//------------------------------------------------------------------------------

#include "PinI_t.h"
#include "CFrag.h"
#include "GraphI_t.h"
#include "PinT_t.h"

//==============================================================================

PinI_t::PinI_t(GraphI_t * _p,string _s):par(_p),pT(0),pPropsI(0),pStateI(0)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
}

//------------------------------------------------------------------------------

PinI_t::~PinI_t()
{
if (pPropsI!=0) delete pPropsI;
if (pStateI!=0) delete pStateI;
}

//------------------------------------------------------------------------------

void PinI_t::AddKey(unsigned k)
{
Key_v.push_back(k);
}

//------------------------------------------------------------------------------

void PinI_t::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n%sPinI_t +++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sNameBase                  %s\n",os,FullName().c_str());
fprintf(fp,"%sMe,Parent                 %#018lx,%#018lx\n",
            os,(uint64_t)this,(uint64_t)par);
if (par!=0) fprintf(fp,"%s...%s\n",os,par->FullName().c_str());
fprintf(fp,"%sType cross-link :  %#018lx\n",os,(uint64_t)pT);
if (pT!=0) fprintf(fp,"%s... -> %s\n",os,pT->FullName().c_str());
fprintf(fp,"%sProperties initialiser    %#018lx\n",os,(uint64_t)pPropsI);
if (pPropsI!=0) pPropsI->Dump(off+2,fp);
fprintf(fp,"%sState initialiser         %#018lx\n",os,(uint64_t)pStateI);
if (pStateI!=0) pStateI->Dump(off+2,fp);
fprintf(fp,"%sKey vector has %lu entries\n",os,Key_v.size());
WALKVECTOR(unsigned,Key_v,i) fprintf(fp,"%s%05u\n",os,*i);
NameBase::Dump(off+2,fp);
DumpChan::Dump(off+2,fp);
fprintf(fp,"%sPinI_t -------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void PinI_t::PinDat_cb(PinI_t * const & p)
// Debug callback for pin data
{
if (p!=0) fprintf(dfp,"pin(D): %s",p->Name().c_str());
else fprintf(dfp,"pin(D): ***");
fflush(dfp);
}

//------------------------------------------------------------------------------

void PinI_t::PinKey_cb(unsigned const & u)
// Debug callback for pin key
{
fprintf(dfp,"pin(K): %03u",u);
fflush(dfp);
}

//==============================================================================




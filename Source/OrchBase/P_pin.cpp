//------------------------------------------------------------------------------

#include "P_pin.h"

//==============================================================================

P_pin::P_pin(D_graph * _p,string _s):par(_p),pP_pintyp(0),pPropsI(0),pStateI(0)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
}

//------------------------------------------------------------------------------

P_pin::~P_pin()
{

}

//------------------------------------------------------------------------------

void P_pin::Dump(FILE * fp)
{
fprintf(fp,"P_pin+++++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      %#018lx,%#018lx\n",
        (uint64_t) this, (uint64_t) par);
fprintf(fp,"NameBase       %s\n",FullName().c_str());
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
fprintf(fp,"Pin type       %#018lx\n", (uint64_t) pP_pintyp);
if (pP_pintyp!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
fprintf(fp,"Properties initialiser    %#018lx\n", (uint64_t) pPropsI);
if (pPropsI!=0) pPropsI->Dump(fp);
fprintf(fp,"State initialiser         %#018lx\n", (uint64_t) pStateI);
if (pStateI!=0) pStateI->Dump(fp);
NameBase::Dump(fp);
DumpChan::Dump(fp);
fprintf(fp,"P_pin---------------------------------------\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void P_pin::PinDat_cb(P_pin * const & p)
// Debug callback for pin data
{
if (p!=0) fprintf(dfp,"pin(D): %s",p->Name().c_str());
else fprintf(dfp,"pin(D): ***");
fflush(dfp);
}

//------------------------------------------------------------------------------

void P_pin::PinKey_cb(unsigned const & u)
// Debug callback for pin key
{
fprintf(dfp,"pin(K): %03u",u);
fflush(dfp);
}

//==============================================================================




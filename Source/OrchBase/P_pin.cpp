//------------------------------------------------------------------------------

#include "P_pin.h"
#include "stdint.h"
// #include "P_datavalue.h"

//==============================================================================

P_pin::P_pin(D_graph * _p,string _s):par(_p),pP_pintyp(0),pPropsI(0),pStateI(0)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
// pProps = 0; // temporary until P_datavalue is debugged
// pState = 0;
}

//------------------------------------------------------------------------------

P_pin::~P_pin()
{
// if (pProps!=0) delete pProps;
// if (pState!=0) delete pState;
if (pPropsI != 0) delete pPropsI;
if (pStateI != 0) delete pStateI;
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
// fprintf(fp,"Properties %#018lx\n", (uint64_t) pProps);
// if (pProps!=0) pProps->Dump(fp);
// fprintf(fp,"State %#018lx\n", (uint64_t) pState);
// if (pState!=0) pState->Dump(fp);
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




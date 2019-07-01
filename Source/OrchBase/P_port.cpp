//------------------------------------------------------------------------------

#include "P_port.h"
#include "stdint.h"

//==============================================================================

P_port::P_port(string _s)
{
Name(_s);
}

//------------------------------------------------------------------------------

P_port::~P_port()
{

}

//------------------------------------------------------------------------------

void P_port::Dump(FILE * fp)
{
fprintf(fp,"P_port++++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me             %#018lx\n", (uint64_t) this);
NameBase::Dump(fp);
DumpChan::Dump(fp);
fprintf(fp,"P_port--------------------------------------\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void P_port::PrtDat_cb(P_port * const & p)
// Debug callback for pin data
{
if (p!=0) fprintf(dfp,"prt(D): %s",p->Name().c_str());
else fprintf(dfp,"prt(D): ***");
fflush(dfp);
}

//------------------------------------------------------------------------------

void P_port::PrtKey_cb(unsigned const & u)
// Debug callback for pin key
{
fprintf(dfp,"prt(K): %03u",u);
fflush(dfp);
}

//==============================================================================




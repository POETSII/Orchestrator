//------------------------------------------------------------------------------

#include "P_link.h"

//==============================================================================

P_link::P_link(string _s)
{
Name(_s);                              // Save name
}

//------------------------------------------------------------------------------

P_link::~P_link()
{

}

//------------------------------------------------------------------------------

void P_link::Dump(FILE * fp)
{
fprintf(fp,"P_link++++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me             %#018lx\n", (uint64_t) this);
NameBase::Dump(fp);
DumpChan::Dump(fp);
fprintf(fp,"P_link--------------------------------------\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void P_link::LnkDat_cb(P_link * const & p)
// Debug callback for arc data
{
if (p!=0) fprintf(dfp,"lnk(D): %s",p->Name().c_str());
else fprintf(dfp,"lnk(D): ***");
fflush(dfp);
}

//------------------------------------------------------------------------------

void P_link::LnkKey_cb(unsigned const & u)
// Debug callback for arc key
{
fprintf(dfp,"lnk(K): %03u",u);
fflush(dfp);
}

//==============================================================================




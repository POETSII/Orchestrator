/* Defines P_link behaviour (see the accompanying header for further
   information). */

#include "P_link.h"

//==============================================================================

P_link::P_link(string _s)
{
Name(_s);                              // Save name
}

//------------------------------------------------------------------------------

/* Constructs a P_link, and names it. */
P_link::P_link(float weight):weight(weight)
{
    AutoName();
}

//------------------------------------------------------------------------------
/* Constructs a P_link, names it, and registers a namebase parent. */
P_link::P_link(float weight, NameBase* parent):weight(weight)
{
    Npar(parent);
    AutoName();
}

//------------------------------------------------------------------------------

P_link::~P_link()
{
WALKVECTOR(Meta_t *,Meta_v,i) delete *i;
}

//------------------------------------------------------------------------------

void P_link::Dump(FILE* file)
{
    /* Pretty breaker */
    std::string prefix = dformat("P_link %s", FullName().c_str());
    DumpUtils::open_breaker(file, prefix);
    fprintf(file, "Edge weight: %f\n", weight);
    NameBase::Dump(file);
    DumpUtils::close_breaker(file, prefix);
    fflush(file);
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




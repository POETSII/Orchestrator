/* Defines P_link behaviour (see the accompanying header for further
   information). */

#include "P_link.h"

/* Constructs a P_link, and names it. */
P_link::P_link(float weight):weight(weight)
{
    AutoName();
}

/* Constructs a P_link, names it, and registers a namebase parent. */
P_link::P_link(float weight, NameBase* parent):weight(weight)
{
    Npar(parent);
    AutoName();
}

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

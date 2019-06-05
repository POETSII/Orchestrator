/* Defines P_port behaviour (see the accompanying header for further
   information). */

#include "P_port.h"

P_port::P_port(NameBase* parent)
{
    Npar(parent);
    AutoName();
}

void P_port::Dump(FILE* file)
{
    std::string prefix = dformat("P_port %s", FullName().c_str());
    DumpUtils::open_breaker(file, prefix);
    NameBase::Dump(file);
    DumpUtils::close_breaker(file, prefix);
    fflush(file);
}

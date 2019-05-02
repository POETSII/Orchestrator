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
    /* Pretty breaker */
    std::string fullName = FullName();  /* Name of this from namebase. */
    std::string nameWithPrefix = dformat("P_port %s ", fullName.c_str());
    std::string breakerTail;
    if (nameWithPrefix.size() >= MAXIMUM_BREAKER_LENGTH)
    {
        breakerTail.assign("+");
    }
    else
    {
        breakerTail.assign(MAXIMUM_BREAKER_LENGTH - nameWithPrefix.size() - 1,
                           '+');
    }
    fprintf(file, "%s%s\n", nameWithPrefix.c_str(), breakerTail.c_str());

    NameBase::Dump(file);

    /* Close breaker and flush the dump. */
    std::replace(breakerTail.begin(), breakerTail.end(), '+', '-');
    fprintf(file, "%s%s\n", nameWithPrefix.c_str(), breakerTail.c_str());
    fflush(file);
}

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
    std::string fullName = FullName();  /* Name of this from namebase. */
    std::string nameWithPrefix = dformat("P_link %s ", fullName.c_str());
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
    fprintf(file, "Edge weight: %f\n", weight);

    NameBase::Dump(file);

    /* Close breaker and flush the dump. */
    std::replace(breakerTail.begin(), breakerTail.end(), '+', '-');
    fprintf(file, "%s%s\n", nameWithPrefix.c_str(), breakerTail.c_str());
    fflush(file);
}

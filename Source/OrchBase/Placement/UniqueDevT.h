#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_UNIQUEDEVT_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_UNIQUEDEVT_H

#include "DevT_t.h"
#include "GraphI_t.h"

/* This type represents an application-device type that is unique within a
 * graph instance. This stops devices from different graph instances, that
 * happen to have the same type, from being placed on the same core (pair). */
struct UniqueDevT
{
    GraphI_t* gi;
    DevT_t* pT;
    bool operator!=(const UniqueDevT& alt) const
        {return !(gi == alt.gi && pT == alt.pT);}
    bool operator<(const UniqueDevT& alt) const
    {
        if (gi == alt.gi) return pT < alt.pT;
        else return gi < alt.gi;
    }
};

#endif

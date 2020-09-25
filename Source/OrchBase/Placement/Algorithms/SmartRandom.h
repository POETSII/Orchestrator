#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_SMARTRANDOM_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_SMARTRANDOM_H

/* Describes the random, hard constraint-aware placement algorithm. */

#include <math.h>

#include "Placer.h"

class SmartRandom: public Algorithm
{
public:
    /* Holds, for each device type, which cores are valid for placement of
     * devices of that type. */
    std::map<DevT_t*, std::set<P_core*> > validCoresForDeviceType;

    SmartRandom(Placer* placer);
    float do_it(GraphI_t* gi);
};

#endif

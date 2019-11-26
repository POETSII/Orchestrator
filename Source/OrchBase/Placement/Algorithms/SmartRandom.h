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
    std::map<P_devtyp*, std::set<P_core*>> validCoresForDeviceType;

    /* Driven by the placer, shouldn't change (so we precompute it for
     * speed). */
    unsigned devicesPerThreadSoftMax;

    SmartRandom(Placer* placer);
    float do_it(P_task* task);
};

#endif

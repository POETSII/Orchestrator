#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_BUCKETFILLING_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_BUCKETFILLING_H

/* Describes the bucket filling placement algorithm. */

class HardwareIterator;

#include "Algorithm.h"
#include "HardwareIterator.h"

class BucketFilling: public Algorithm
{
public:
    BucketFilling(Placer* placer);
    float do_it(GraphI_t* gi);
    bool is_core_empty(P_core* core);
    void poke_iterator(HardwareIterator& hardwareIt);
};

#endif

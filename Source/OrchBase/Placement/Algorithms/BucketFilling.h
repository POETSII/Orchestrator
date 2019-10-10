#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_BUCKETFILLING_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_BUCKETFILLING_H

/* Describes the bucket filling placement algorithm. */

class Algorithm;
class HardwareIterator;

#include "Algorithm.h"
#include "HardwareIterator.h"
#include "Placer.h"

class BucketFilling: public Algorithm
{
public:
    BucketFilling(Placer* placer):Algorithm(placer){}
    float do_it(P_task* task);
    void Dump(FILE* = stdout);
    void poke_iterators(std::vector<HardwareIterator>* iterators);
};

#endif

#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_BUCKETFILLING_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_BUCKETFILLING_H

/* Describes the bucket filling placement algorithm. */

class Algorithm;
class Placer;

#include "Algorithm.h"
#include "Placer.h"

class BucketFilling: public Algorithm
{
public:
    float do_it(P_task*, Placer*);
    void Dump(FILE* = stdout);
};

#endif

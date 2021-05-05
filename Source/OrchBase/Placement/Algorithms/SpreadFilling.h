#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_SPREADFILLING_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_SPREADFILLING_H

/* Describes the spread filling placement algorithm. */

class HardwareIterator;

#include "Algorithm.h"
#include "HardwareIterator.h"
#include "Placer.h"

class SpreadFilling: public Algorithm
{
public:
    SpreadFilling(Placer* placer);
    float do_it(GraphI_t* gi);
};

#endif

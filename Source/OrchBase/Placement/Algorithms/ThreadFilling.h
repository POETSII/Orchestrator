#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_THREADFILLING_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_THREADFILLING_H

/* Describes the thread-filling (formerly bucket-filling) placement
 * algorithm. */

class HardwareIterator;

#include "Algorithm.h"
#include "HardwareIterator.h"

class ThreadFilling: public Algorithm
{
public:
    ThreadFilling(Placer* placer);
    float do_it(GraphI_t* gi);
    bool is_core_empty(P_core* core);
    void poke_iterator(HardwareIterator& hardwareIt);
};

#endif

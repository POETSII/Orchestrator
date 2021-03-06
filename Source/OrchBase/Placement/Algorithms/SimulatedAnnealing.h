#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_SIMULATEDANNEALING_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_SIMULATEDANNEALING_H

/* Describes the simulated annealing placement algorithm. */

#include <math.h>

class HardwareIterator;

#include "FileOpenException.h"
#include "HardwareIterator.h"
#include "Placer.h"
#include "ThreadFilling.h"

/* Naive end point. */
#define ITERATION_MAX_DEFAULT unsigned(1e8)

/* Exponential decay with half life of a third of the number of iterations. */
#define DISORDER_DECAY

class SimulatedAnnealing: public Algorithm
{
public:
    SimulatedAnnealing(Placer* placer, bool disorder=true);

    bool disorder;
    unsigned iteration;
    unsigned maxIteration;  /* Set in do_it */

    /* Holds, for each device type, which cores are valid for placement of
     * devices of that type. */
    std::map<UniqueDevT, std::set<P_core*> > validCoresForDeviceType;

    /* Driven by the placer, shouldn't change (so we precompute it for
     * speed). */
    unsigned devicesPerThreadSoftMax;
    unsigned threadsPerCoreSoftMax;

    SimulatedAnnealing(Placer* placer);
    float acceptance_probability(float fitnessBefore, float fitnessAfter);
    inline float compute_disorder();
    float do_it(GraphI_t* gi);
    inline bool is_finished();
    void select(GraphI_t* gi, DevI_t** device, P_thread** thread,
                DevI_t** swapDevice);
};

#endif

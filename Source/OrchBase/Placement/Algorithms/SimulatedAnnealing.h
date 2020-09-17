#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_SIMULATEDANNEALING_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_SIMULATEDANNEALING_H

/* Describes the simulated annealing placement algorithm. */

#include <math.h>

class HardwareIterator;

#include "BucketFilling.h"
#include "FileOpenException.h"
#include "HardwareIterator.h"
#include "Placer.h"

/* Naive end point. */
#define ITERATION_MAX 100000000

/* Exponential decay with half life of a third of the number of iterations. */
#define DISORDER_DECAY log(0.5) / (ITERATION_MAX / 3.0)

class SimulatedAnnealing: public Algorithm
{
public:
    SimulatedAnnealing(Placer* placer, bool disorder=true);

    unsigned iteration;
    bool disorder;

    /* Holds, for each device type, which cores are valid for placement of
     * devices of that type. */
    std::map<P_devtyp*, std::set<P_core*>> validCoresForDeviceType;

    /* Driven by the placer, shouldn't change (so we precompute it for
     * speed). */
    unsigned devicesPerThreadSoftMax;
    unsigned threadsPerCoreSoftMax;

    SimulatedAnnealing(Placer* placer);
    float acceptance_probability(float fitnessBefore, float fitnessAfter);
    inline float compute_disorder();
    float do_it(P_task* task);
    inline bool is_finished();
    void select(P_task* task, P_device** device, P_thread** thread,
                P_device** swapDevice);
};

#endif

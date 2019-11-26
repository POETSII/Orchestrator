#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_SIMULATEDANNEALING_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_SIMULATEDANNEALING_H

/* Describes the simulated annealing placement algorithm. */

#include <math.h>

class HardwareIterator;

#include "BucketFilling.h"
#include "HardwareIterator.h"
#include "Placer.h"

/* Naive end point. */
#define ITERATION_MAX 300000

/* Exponential decay (half life of 5000 iterations:
 * DISORDER_DECAY = - ln(0.5) / 5000. */
#define DISORDER_DECAY 0.0001386  /* More or less. */

class SimulatedAnnealing: public Algorithm
{
public:
    unsigned iteration;

    /* Holds, for each device type, which cores are valid for placement of
     * devices of that type. */
    std::map<P_devtyp*, std::set<P_core*>> validCoresForDeviceType;

    /* Driven by the placer, shouldn't change (so we precompute it for
     * speed). */
    unsigned devicesPerThreadSoftMax;

    SimulatedAnnealing(Placer* placer);
    float acceptance_probability(float fitnessBefore, float fitnessAfter);
    inline float compute_disorder();
    void define_valid_cores_map(P_task* task);
    float do_it(P_task* task);
    inline bool is_finished();
    void select(P_task* task, P_device** device, P_thread** thread,
                P_device** swapDevice);
};

#endif

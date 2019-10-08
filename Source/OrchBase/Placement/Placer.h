#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_PLACER_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_PLACER_H

/* Describes the encapsulated placement logic, which maps a POETS application
 * (loaded from an XML, presumably) to the POETS hardware stack (specifically,
 * a POETS Engine).
 *
 * The Placer is a self-contained description of how devices (P_device) are
 * placed onto threads (P_thread).
 *
 * See the placement documentation for further information. */

#include <list>
#include <map>
#include <vector>

/* Algorithms! */
#include "BucketFilling.h"

/* Constraints! */
#include "MaxDevicesPerThread.h"

/* Exceptions! */
#include "AlreadyPlacedException.h"
#include "InvalidAlgorithmDescriptorException.h"
#include "NoSpaceToPlaceException.h"

/* And everything else. */
#include "DumpUtils.h"
#include "HardwareModel.h"
#include "OSFixes.hpp"
#include "P_device.h"
#include "P_task.h"

class Placer: public DumpChan
{
public:
    Placer(P_engine* engine);
    ~Placer();
    P_engine* engine;

    /* Placement information for the entire system is held in these maps. */
    std::map<P_device*, P_thread*> deviceToThread;
    std::map<P_thread*, std::list<P_device*>> threadToDevices;

    /* Constraint management. */
    std::list<Constraint*> constraints;

    /* Information on tasks that have been placed. */
    std::map<P_task*, Algorithm*> placedTasks;

    /* Fitness evaluation. */
    float compute_fitness(P_task* task);

    /* Diagnostics */
    void Dump(FILE* = stdout);

    /* Doing the dirty */
    float place(P_task* task, std::string algorithmDescription);
    void unplace(P_task* task);

    /* Low-level placement operation, to be used only be algorithms */
    void link(P_thread* thread, P_device* device);

    /* Constraint query */
    unsigned constrained_max_devices_per_thread(P_task* task);

private:
    Algorithm* algorithm_from_string(std::string);

    /* Integrity */
    bool check_all_devices_mapped(P_task* task,
                                  std::vector<P_device*>* unmapped);
};

#endif

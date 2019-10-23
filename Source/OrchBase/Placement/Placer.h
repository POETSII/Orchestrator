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

#include <ctime>
#include <fstream>
#include <list>
#include <map>
#include <vector>

class Algorithm;
struct Result;

/* Algorithms! */
#include "Algorithm.h"
#include "BucketFilling.h"

/* Constraints! */
#include "MaxDevicesPerThread.h"

/* Exceptions! */
#include "AlreadyPlacedException.h"
#include "BadIntegrityException.h"
#include "InvalidAlgorithmDescriptorException.h"
#include "NoEngineException.h"
#include "NoSpaceToPlaceException.h"
#include "NoTaskToDumpException.h"

/* And everything else. */
#include "DumpUtils.h"
#include "HardwareModel.h"
#include "OSFixes.hpp"
#include "P_device.h"
#include "P_devtyp.h"
#include "P_task.h"

class Placer
{
public:
    Placer();
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

    /* Since each core pair must hold only devices of one type, and since a
     * device type is bound to a certain task, it is often useful to identify
     * the cores (and threads) that contain devices owned by a certain task. */
    std::map<P_task*, std::set<P_core*>> taskToCores;

    /* Dealing with hard constraints. */
    bool are_all_hard_constraints_satisfied(P_task* task,
                                            std::vector<Constraint*>* broken);

    /* Fitness evaluation. */
    float compute_fitness(P_task* task);

    /* Diagnostics (note the lowercase D). This is not a dumpchan method -
     * we're doing something fundamentally different here. */
    void dump(P_task* task);

    /* Convenient way to get all boxes mapped with a given task in the
     * engine. */
    void get_boxes_for_task(P_task* task, std::set<P_box*>* boxes);

    /* Low-level placement operation, to be used only be algorithms */
    void link(P_thread* thread, P_device* device);

    /* Doing the dirty */
    float place(P_task* task, std::string algorithmDescription);
    void unplace(P_task* task);

    /* Constraint query */
    unsigned constrained_max_devices_per_thread(P_task* task);

private:
    Algorithm* algorithm_from_string(std::string);
    void update_task_to_cores_map(P_task* task);

    /* Integrity */
    bool check_all_devices_mapped(P_task* task,
                                  std::vector<P_device*>* unmapped);
    // <!> Could have a method that verifies that no core pairs have more than
    // one device type placed upon them.
    // <!> Add are_all_hard_constraints_satisfied to the integrity check.

    void populate_result_structures(Result* result, P_task* task, float score);

    /* Fine-grained diagnostics. */
    void dump_costs(P_task* task, const char* path);
    void dump_diagnostics(P_task* task, const char* path);
    void dump_map(P_task* task, const char* path);
};

#endif

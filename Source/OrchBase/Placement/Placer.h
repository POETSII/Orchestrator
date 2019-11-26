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

class CostCache;
struct Result;

/* Algorithms! */
#include "Algorithm.h"
#include "SimulatedAnnealing.h"
#include "SmartRandom.h"

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
#include "CostCache.h"
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
    CostCache* cache;

    /* Placement information for the entire system is held in these maps. */
    std::map<P_device*, P_thread*> deviceToThread;
    std::map<P_thread*, std::list<P_device*>> threadToDevices;

    /* Constraint management. */
    std::list<Constraint*> constraints;

    /* Edge costs. */
    std::map<P_task*, std::map<std::pair<P_device*, P_device*>, float>> \
        taskEdgeCosts;

    /* Information on tasks that have been placed. */
    std::map<P_task*, Algorithm*> placedTasks;

    /* Since each core pair must hold only devices of one type, and since a
     * device type is bound to a certain task, it is often useful to identify
     * the cores (and threads) that contain devices owned by a certain task. */
    std::map<P_task*, std::set<P_core*>> taskToCores;

    /* The reverse of the node map in a task graph - given a device, what is
     * the key in the graph? */
    std::map<P_device*, unsigned> deviceToGraphKey;

    /* Check integrity of a placed task. */
    void check_integrity(P_task* task, std::string algorithmDescription);
    bool are_all_core_pairs_device_locked(P_task* task,
        std::map<std::pair<P_core*, P_core*>,
                 std::set<P_devtyp*>>* badCoresToDeviceTypes);
    bool are_all_devices_mapped(P_task* task,
                                std::vector<P_device*>* unmapped);
    bool are_all_hard_constraints_satisfied(P_task* task,
        std::vector<Constraint*>* broken,
        std::vector<P_device*> devices = std::vector<P_device*>());

    /* Fitness evaluation. */
    float compute_fitness(P_task* task);
    float compute_fitness(P_task* task, P_device* device);
    void populate_edge_weight(P_task* task, P_device* from, P_device* to);
    void populate_edge_weights(P_task* task);
    void populate_edge_weights(P_task* task, P_device* device);

    void populate_result_structures(Result* result, P_task* task, float score);

    /* Diagnostics (note the lowercase D). This is not a dumpchan method -
     * we're doing something fundamentally different here. */
    void dump(P_task* task);

    /* Convenient way to get all boxes mapped with a given task in the
     * engine. */
    void get_boxes_for_task(P_task* task, std::set<P_box*>* boxes);

    /* Convenient way to get all edges (as device-device pairs) that involve a
     * given device. */
    void get_edges_for_device(P_task* task, P_device* device,
        std::vector<std::pair<P_device*, P_device*>>* devicePairs);

    /* Convenient way to get core occupation. */
    void define_valid_cores_map(P_task* task,
        std::map<P_devtyp*, std::set<P_core*>>* validCoresForDeviceType);

    /* Low-level placement operation, to be used only be algorithms */
    void link(P_thread* thread, P_device* device);

    /* Doing the dirty */
    float place(P_task* task, std::string algorithmDescription);
    void unplace(P_task* task, bool andConstraints=true);

    /* Constraint query */
    unsigned constrained_max_devices_per_thread(P_task* task);

    /* Updates software addresses for placed devices. */
    void update_software_addresses(P_task* task);

    /* Timing for algorithms */
    std::string timestamp();

private:
    Algorithm* algorithm_from_string(std::string);
    void update_task_to_cores_map(P_task* task);

    void populate_device_to_graph_key_map(P_task* task);

    /* Fine-grained diagnostics. */
    void dump_costs(P_task* task, const char* path);
    void dump_diagnostics(P_task* task, const char* path);
    void dump_map(P_task* task, const char* path);
};

#endif

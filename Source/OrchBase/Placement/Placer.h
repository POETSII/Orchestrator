#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_PLACER_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_PLACER_H

/* Describes the encapsulated placement logic, which maps a POETS graph
 * instance (loaded from an XML, presumably) to the POETS hardware stack
 * (specifically, a POETS Engine).
 *
 * The Placer is a self-contained description of how devices (DevI_t) are
 * placed onto threads (P_thread).
 *
 * See the placement documentation for further information. */

#include <climits>
#include <ctime>
#include <fstream>
#include <list>
#include <map>
#include <vector>

class CostCache;
struct Result;

/* Algorithms! */
#include "Algorithm.h"
#include "PlacementLoader.h"
#include "SimulatedAnnealing.h"
#include "SmartRandom.h"
#include "SpreadFilling.h"

/* Constraints! */
#include "MaxDevicesPerThread.h"
#include "MaxThreadsPerCore.h"

/* Exceptions! */
#include "AlreadyPlacedException.h"
#include "BadIntegrityException.h"
#include "InvalidAlgorithmDescriptorException.h"
#include "NoEngineException.h"
#include "NoSpaceToPlaceException.h"
#include "NoGIToDumpException.h"

/* And everything else. */
#include "Apps_t.h"
#include "CostCache.h"
#include "DumpUtils.h"
#include "GraphI_t.h"
#include "HardwareModel.h"
#include "OSFixes.hpp"
#include "PlaceArgs.h"
#include "UniqueDevT.h"

#define THREAD_LOADING_SCALING_FACTOR 0.01

class Placer
{
public:
    Placer();
    Placer(P_engine* engine);
    ~Placer();
    P_engine* engine;
    CostCache* cache;

    /* Placement information for the entire system is held in these maps. */
    std::map<DevI_t*, P_thread*> deviceToThread;
    std::map<P_thread*, std::list<DevI_t*> > threadToDevices;

    /* Constraint management. */
    std::list<Constraint*> constraints;

    /* Edge costs. */
    std::map<GraphI_t*, std::map<std::pair<DevI_t*, DevI_t*>, float> > \
        giEdgeCosts;

    /* Information on graph instances that have been placed. */
    std::map<GraphI_t*, Algorithm*> placedGraphs;

    /* Since each core pair must hold only devices of one type from one graph
     * instance, it is often useful to identify the cores (and threads) that
     * contain devices owned by a certain graph instance. */
    std::map<GraphI_t*, std::set<P_core*> > giToCores;

    /* The reverse of the node map in a graph instance - given a device, what
     * is the key in the graph? */
    std::map<DevI_t*, unsigned> deviceToGraphKey;

    /* Path to write output files to (e.g. dumps, results). At its best with a
     * trailing slash (yay C++98) */
    std::string outFilePath;

    /* Staged arguments for the next algorithm. */
    PlaceArgs args;

    /* Identify whether an input string is an algorithm or an argument. Also
     * see the private algorithm_from_string method. */
    bool algorithm_or_argument(std::string unknown, bool& isAlgorithm);

    /* Check integrity of a placed graph instance. */
    void check_integrity(GraphI_t* gi, Algorithm* algorithm);
    bool are_all_core_pairs_device_locked(GraphI_t* gi,
        std::map<std::pair<P_core*, P_core*>,
                 std::set<UniqueDevT> >* badCoresToDeviceTypes);
    bool are_all_devices_mapped(GraphI_t* gi, std::vector<DevI_t*>* unmapped);
    bool are_all_hard_constraints_satisfied(GraphI_t* gi,
        std::vector<Constraint*>* broken,
        std::vector<DevI_t*> devices = std::vector<DevI_t*>());

    /* Fitness evaluation. */
    float compute_fitness(GraphI_t* gi);
    float compute_fitness(GraphI_t* gi, DevI_t* device);
    void populate_edge_weight(GraphI_t* gi, DevI_t* from, DevI_t* to);
    void populate_edge_weights(GraphI_t* gi);
    void populate_edge_weights(GraphI_t* gi, DevI_t* device);

    void populate_result_structures(Result* result, GraphI_t* gi, float score);

    /* Diagnostics (note the lowercase D). This is not a dumpchan method -
     * we're doing something fundamentally different here. */
    void dump(GraphI_t* gi);

    /* Convenient way to get all boxes mapped with a given graph instance in
     * the engine. */
    void get_boxes_for_gi(GraphI_t* gi, std::set<P_box*>* boxes);

    /* Convenient way to get all cores mapped within a given graph instance
     * that exist on a certain box. */
    void get_cores_for_gi_in_box(GraphI_t* gi, P_box* box,
                                 std::set<P_core*>& cores);

    /* Convenient way to get all edges (as device-device pairs) that involve a
     * given device. */
    void get_edges_for_device(GraphI_t* gi, DevI_t* device,
        std::vector<std::pair<DevI_t*, DevI_t*> >* devicePairs);

    /* Convenient way to get core occupation. */
    void define_valid_cores_map(GraphI_t* gi,
        std::map<UniqueDevT, std::set<P_core*> >* validCoresForDeviceType);

    /* Redistribution of devices within a graph instance at the core level. */
    void redistribute_devices_in_gi(GraphI_t* gi);
    void redistribute_devices_in_core(P_core* core, GraphI_t* gi=PNULL);

    /* Low-level placement operation, to be used only be algorithms */
    void link(P_thread* thread, DevI_t* device);

    /* Doing the dirty */
    float place(GraphI_t* gi, std::string algorithmDescription);
    float place(GraphI_t* gi, Algorithm* algorithm);
    float place_load(GraphI_t* gi, std::string path);
    void unplace(GraphI_t* gi, bool andConstraints=true);

    /* Constraint query */
    unsigned constrained_max_devices_per_thread(GraphI_t* gi);
    unsigned constrained_max_threads_per_core(GraphI_t* gi);

    /* Updates software addresses for placed devices. */
    void update_software_addresses(GraphI_t* gi);

    /* Timing for algorithms */
    std::string timestamp();

private:
    Algorithm* algorithm_from_string(std::string);  /* Also see the public
                                                     * algorithm_or_argument
                                                     * method. */
    void update_gi_to_cores_map(GraphI_t* gi);

    void populate_device_to_graph_key_map(GraphI_t* gi);

    /* Fine-grained diagnostics. */
    void dump_costs(GraphI_t* gi, const char* path);
    void dump_diagnostics(GraphI_t* gi, const char* path);
    void dump_edge_loading(const char* path);
    void dump_map(GraphI_t* gi, const char* path);
    void dump_map_reverse(GraphI_t* gi, const char* path);
    void dump_node_loading(const char* path);
};

#endif

#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_COSTCACHE_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_COSTCACHE_H

/* Holds a cached matrix of non-neighbour mailbox-mailbox edge costs. The cache
 * is computed using the Floyd-Warshall algorithm (time O(nodes^3) and
 * O(nodes^2) memory), and may be used by placement algorithms. This cache also
 * stores the computed path (extra time O(edge*node) and O(node^2) memory).
 * For convenience, this class also exposes methods to compute thread-thread
 * costs using the costs defined by the engine.
 *
 * See the placement documentation for further information. */

#include "DumpUtils.h"
#include "HardwareModel.h"

/* To construct the cache, we need to create a large graph of mailboxes from
 * the engine's board graph, and each board's mailbox graph. Pins aren't
 * explicitly used, but edges are floats (which represent costs). */
typedef pdigraph<unsigned, P_mailbox*,
                 unsigned, float,
                 unsigned, unsigned> CombinedGraph;

class CostCache: public DumpChan
{
public:
    std::map<P_mailbox*, std::map<P_mailbox*, float>> costs;
    std::map<P_mailbox*, std::map<P_mailbox*, P_mailbox*>> pathNext;
    P_engine* engine;
    CostCache(P_engine* engine);
    void build_cache();
    float compute_cost(P_thread*, P_thread*);
    void get_path(P_mailbox*, P_mailbox*, std::vector<P_mailbox*>*);
    void populate_combined_graph(CombinedGraph*, std::ofstream* = PNULL);
    void Dump(FILE* = stdout);
};

#endif

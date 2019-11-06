/* Defines cost calculation behaviour (see the accompanying header for further
 * information). */

#include "CostCache.h"

CostCache::CostCache(P_engine* engine):engine(engine){build_cache();}

/* Populates the 'costs' and 'pathNext' member by querying the cost model
 * stored in the engine. */
void CostCache::build_cache()
{
    /* Make a big graph! It's thrown away when we're done. */
    CombinedGraph combinedGraph;
    populate_combined_graph(&combinedGraph);

    /* Optionally, we can dump bits of the graph as we go, but the file is
     * redundant with the cache dump. It's pretty useful for debugging
     * though. -- Past Mark
    std::ofstream ofs;
    ofs.open("graph_detail.txt", std::ofstream::out | std::ofstream::app);
    populate_combined_graph(&combinedGraph, &ofs);
    ofs.close();
    */

    /* Initialise members (we may be called twice). All path lengths are
     * initialised to a big float, and all 'next-nodes' are initialised to a
     * special 'uninitialised' value. */
    HardwareIterator outerIt = HardwareIterator(engine);
    HardwareIterator middleIt = HardwareIterator(engine);

    P_mailbox* outer = PNULL;
    P_mailbox* middle = PNULL;

    while (!(outerIt.has_wrapped()))
    {
        outer = outerIt.get_mailbox();
        while (!(middleIt.has_wrapped()))
        {
            middle = middleIt.get_mailbox();

            /* Here's the initialisation. */
            costs[outer][middle] = HUGE_VAL;
            pathNext[outer][middle] = PNULL;

            middleIt.next_mailbox();
        }

        outerIt.next_mailbox();
    }

    /* Floyd-Warshall (with path extension) starts here. Look at the psuedocode
     * in the documentation (it'll make a lot more sense than comments here
     * would). */

    /* Initial values. These arcs get walked twice, which is a shame, but
     * doesn't really affect the speed of the algorithm (it's limited by
     * nodes^3). */
    WALKPDIGRAPHARCS(unsigned, P_mailbox*, unsigned, float, unsigned, unsigned,
                     combinedGraph, edgeIt)
    {
        P_mailbox* fromMailbox = *(edgeIt->second.fr_n->second);
        P_mailbox* toMailbox = *(edgeIt->second.to_n->second);
        costs[fromMailbox][toMailbox] = edgeIt->second.data;
        pathNext[fromMailbox][toMailbox] = toMailbox;
    }

    /* Trivial values */
    WALKPDIGRAPHNODES(unsigned, P_mailbox*, unsigned, float, unsigned,
                      unsigned, combinedGraph, nodeIt)
    {
        P_mailbox* loopMailbox = nodeIt->second.data;
        costs[loopMailbox][loopMailbox] = 0;
        pathNext[loopMailbox][loopMailbox] = loopMailbox;
    }

    /* As I'm sure you're aware because you've read the documentation, we
     * parallelise the middle (O(node^2)) loop, such that each thread iterates
     * over all mailboxes in a board. Before we start iterating however, we can
     * compute the mailbox ranges that these threads will occupy, because they
     * won't change as a function of the position of the outer loop. We do so
     * now.
     *
     * The result of this loop is that threadRanges is populated, for one entry
     * per thread, with a range of mailboxes for each thread to consider.
     *
     * Alternatively, if we're duressed into running in serial, we simply spawn
     * a single thread spanning the entire engine. */
    std::vector<std::pair<P_mailbox*, P_mailbox*>> threadRanges;
    P_mailbox* first;
    P_mailbox* last;
    if (SERIAL_FLOYD_WARSHALL)
    {
        /* Look how easy pdigraph is to use! :p */
        first = engine->G.index_n.begin()->second.data->
            G.index_n.begin()->second.data;
        last = engine->G.index_n.rbegin()->second.data->
            G.index_n.rbegin()->second.data;
        threadRanges.push_back(std::make_pair(first, last));
    }
    else
    {
        WALKPDIGRAPHNODES(AddressComponent, P_board*,
                          unsigned, P_link*,
                          unsigned, P_port*, engine->G, boardNode)
        {
            first = boardNode->second.data->G.index_n.begin()->second.data;
            last = boardNode->second.data->G.index_n.rbegin()->second.data;
            threadRanges.push_back(std::make_pair(first, last));
        }
    }

    /* We'll need an object to hold all of the pthread_ts we're going to start
     * creating... as well as their arguments. */
    std::vector<pthread_t> threads(threadRanges.size(), 0);

    /* O(node^3) loop of death. */
    std::vector<FWThreadArg*> threadArgs;
    outerIt.reset_all_iterators();
    middleIt.reset_all_iterators();
    while (!(outerIt.has_wrapped()))
    {
        printf("On mailbox %s.\n", outerIt.get_mailbox()->FullName().c_str());

        /* Here's the parallel O(node^2) loop - spawn one thread to deal with
         * all mailboxes per board. */
        for (unsigned threadIndex = 0; threadIndex < threads.size();
             threadIndex++)
        {
            /* Kick off a thread. */
            threadArgs.push_back(new FWThreadArg);
            threadArgs.back()->outer = outerIt.get_mailbox();
            threadArgs.back()->middleStart = threadRanges[threadIndex].first;
            threadArgs.back()->middleEnd = threadRanges[threadIndex].second;
            threadArgs.back()->engine = engine;
            threadArgs.back()->costs = &costs;
            threadArgs.back()->pathNext = &pathNext;

            /* If we're in serial, just call the method. */
            int rc;
            if (SERIAL_FLOYD_WARSHALL)
            {
                inner_floyd_warshall((void*)threadArgs.back());
                rc = 0;
            }

            else
            {
                rc = pthread_create(&(threads[threadIndex]), PNULL,
                                    inner_floyd_warshall,
                                    (void*)threadArgs.back());
            }

            // <!> Some handy prints and checks - in production, we should be
            // checking rc though.
            if (rc) printf("Oh dear... no thread! (rc = %d)\n", rc);
            //else printf("Spawned thread %u to iterate from %s to %s.\n",
            //            threadIndex,
            //            threadRanges[threadIndex].first->FullName().c_str(),
            //            threadRanges[threadIndex].second->FullName().c_str());
        }

        /* Thread barrier. */
        if (!SERIAL_FLOYD_WARSHALL)
        {
            for (std::vector<pthread_t>::iterator threadIt = threads.begin();
                 threadIt != threads.end(); threadIt++)
            {
                pthread_join(*threadIt, PNULL);
            }
        }

        /* Cleanup */
        threadArgs.clear();

        /* Next iteration */
        middleIt.reset_all_iterators();
        outerIt.next_mailbox();
    }
}

/* Computes the communication cost between two threads, given the cache has
 * been constructed. */
float CostCache::compute_cost(P_thread* fromThread, P_thread* toThread)
{
    P_core* fromCore = fromThread->parent;
    P_core* toCore = toThread->parent;
    if (fromCore != toCore)
    {
        P_mailbox* fromMailbox = fromCore->parent;
        P_mailbox* toMailbox = toCore -> parent;
        if (fromMailbox != toMailbox)
        {
            /* Threads are on different mailboxes. */
            return fromCore->costCoreThread +
                fromMailbox->costMailboxCore +
                costs[fromMailbox][toMailbox] +
                toMailbox->costMailboxCore +
                toCore -> costCoreThread;
        }
        else
        {
            /* Threads are on the same mailbox, but on different cores. */
            return fromCore->costCoreThread +
                fromMailbox->costCoreCore +
                toCore->costCoreThread;
        }
    }
    else
    {
        /* Threads are on the same core. */
        return fromCore->costThreadThread;
    }
}

/* Computes the shortest path from one mailbox to another, using the path
 * cache. Arguments:
 *
 * - from: Mailbox to traverse from
 *
 * - to: Mailbox to traverse from
 *
 * - path: Vector to populate with the path, described as a sequence of
 *   mailboxes. Is kept empty if there  */
void CostCache::get_path(P_mailbox* from, P_mailbox* to,
                         std::vector<P_mailbox*>* path)
{
    /* Must be empty to start. */
    path->clear();

    /* Trivial case - disconnected nodes (shouldn't happen). */
    if (pathNext[from][to] == PNULL) return;

    P_mailbox* currentNode = from;
    path->push_back(currentNode);
    while (currentNode != to)
    {
        currentNode = pathNext[currentNode][to];
        path->push_back(currentNode);
    }
}

/* Performs the middle and inner loop computation of the O(N^3) loop of the
 * Floyd-Warshall algorithm. See the comments in the FWThreadArg declaration
 * for the form of arguments accepted. */
void* CostCache::inner_floyd_warshall(void* arg)
{
    FWThreadArg* args = (FWThreadArg*) arg;

    P_mailbox* outer = args->outer;

    P_mailbox* middle;
    P_mailbox* middleStart = args->middleStart;
    P_mailbox* middleEnd = args->middleEnd;
    HardwareIterator middleIt = HardwareIterator(args->engine);

    P_mailbox* inner;
    HardwareIterator innerIt = HardwareIterator(args->engine);

    std::map<P_mailbox*, std::map<P_mailbox*, float>>* costs = args->costs;
    std::map<P_mailbox*, std::map<P_mailbox*, P_mailbox*>>* pathNext =
        args->pathNext;

    /* Move the middle iterator to middleStart. */
    while (middleIt.get_mailbox() != middleStart) middleIt.next_mailbox();

    /* O(node^2) loop (iterate over mailbox range, including the end point) */
    do
    {
        middle = middleIt.get_mailbox();

        /* O(node) loop (iterate over all mailboxes in the engine) */
        while (!(innerIt.has_wrapped()))
        {
            inner = innerIt.get_mailbox();

            /* Have we found a better path? */
            if ((*costs)[middle][inner] >
                    (*costs)[middle][outer] + (*costs)[outer][inner])
            {
                /* Update! */
                (*costs)[middle][inner] =
                    (*costs)[middle][outer] + (*costs)[outer][inner];
                (*pathNext)[middle][inner] = (*pathNext)[middle][outer];
            }

            innerIt.next_mailbox();
        }

    }
    /* This is a bit weird, but it basically means "was that the last mailbox?
     * If so, leave. If not, we look at the next mailbox in the next
     * iteration. If the first predicate is false (i.e. we just did the last
     * mailbox), we don't care about the second predicate - incrementing it
     * will do no harm. */
    while (middle != middleEnd && middleIt.next_mailbox());

    /* How we exit depends on whether or not this was called from a thread. */
    if (SERIAL_FLOYD_WARSHALL) return PNULL;
    else pthread_exit(PNULL);
}

/* Defines a combined graph from the engine's board graph, and each board's
 * mailbox graph, to aid with cache computation. Arguments:
 *
 * - graph: graph to populate (is cleared).
 *
 * - stream: ofstream to write debugging output to. If PNULL, no output is
 *   written.
 *
 * This is going to change significantly when mailbox-ports are introduced. */
void CostCache::populate_combined_graph(CombinedGraph* graph,
                                        std::ofstream* stream)
{
    graph->Clear();

    /* We don't care about the value of these - as long as they are distinct
     * for every node/vertex/pin. */
    unsigned mailboxKey = 0;
    unsigned edgeKey = 0;
    unsigned portKey = 0;
    unsigned portValue = 0;

    /* This maps mailboxes to their keys in the combined graph. Note that the
     * keys held by this map are not related to the keys in the engine
     * graph. */
    std::map<P_mailbox*, unsigned> mailboxToKey;

    /* Iterate through each board, and translate its nodes and edges into the
     * combined graph. This will result a number of component subgraphs equal
     * to the number of boards (assuming no boards are empty, or other
     * ludicrousness... but we don't fail either way) */
    WALKPDIGRAPHNODES(AddressComponent, P_board*,
                      unsigned, P_link*,
                      unsigned, P_port*, engine->G, boardNode)
    {
        /* Add each node. */
        WALKPDIGRAPHNODES(AddressComponent, P_mailbox*,
                          unsigned, P_link*,
                          unsigned, P_port*,
                          boardNode->second.data->G, mailboxNode)
        {
            mailboxToKey[mailboxNode->second.data] = mailboxKey;
            graph->InsertNode(mailboxKey++, mailboxNode->second.data);

            /* Logging, if enabled. */
            if (stream != PNULL)
                *stream << "Adding mailbox "
                        << mailboxNode->second.data->FullName().c_str()
                        << " to key "
                        << mailboxKey - 1 << ".\n";
        }

        /* Add each edge. */
        WALKPDIGRAPHARCS(AddressComponent, P_mailbox*,
                         unsigned, P_link*,
                         unsigned, P_port*,
                         boardNode->second.data->G, edge)
        {
            /* Forward and not reverse - because the graph is bidirectional. */
            graph->InsertArc(edgeKey++,
                             mailboxToKey[edge->second.fr_n->second.data],
                             mailboxToKey[edge->second.to_n->second.data],
                             edge->second.data->weight,
                             portKey, portValue,
                             portKey + 1, portValue + 1);

            /* Logging, if enabled. */
            if (stream != PNULL)
                *stream << "Joining mailbox "
                        << edge->second.fr_n->second.data->FullName().c_str()
                        << " (key="
                        << mailboxToKey[edge->second.fr_n->second.data]
                        << ") with mailbox "
                        << edge->second.to_n->second.data->FullName().c_str()
                        << " (key="
                        << mailboxToKey[edge->second.to_n->second.data]
                        << ") with cost "
                        << edge->second.data->weight
                        << " and edge key "
                        << edgeKey - 1
                        << ".\n";

            portKey += 2;
            portValue += 2;
        }
    }

    /* Now here's the funky bit - for each edge in the engine's board graph,
     * connect each mailbox in the "from" board to each mailbox in the "to"
     * board. The edge weight for those edges is the weight of the edge in the
     * engine graph, plus the costBoardMailbox value of the board.
     *
     * This is a bit silly, but again, will be replaced with more intelligent
     * logic when mailbox ports are introduced. It's good enough for now. */

    /* For each board edge (each direction)... */
    WALKPDIGRAPHARCS(AddressComponent, P_board*,
                     unsigned, P_link*,
                     unsigned, P_port*, engine->G, boardEdge)
    {
        /* For each mailbox in the 'from' board... */
        WALKPDIGRAPHNODES(AddressComponent, P_mailbox*,
                          unsigned, P_link*,
                          unsigned, P_port*,
                          boardEdge->second.fr_n->second.data->G,
                          mailboxFromNode)
        {
            /* For each mailbox in the 'to' board... */
            WALKPDIGRAPHNODES(AddressComponent, P_mailbox*,
                              unsigned, P_link*,
                              unsigned, P_port*,
                              boardEdge->second.to_n->second.data->G,
                              mailboxToNode)
            {
                /* Join them up. */
                float cost =
                    boardEdge->second.fr_n->second.data->costBoardMailbox +
                    boardEdge->second.data->weight;

                graph->InsertArc(edgeKey++,
                    mailboxToKey[mailboxFromNode->second.data],
                    mailboxToKey[mailboxToNode->second.data],
                    cost, portKey, portValue, portKey + 1, portValue + 1);

                portKey += 2;
                portValue += 2;

                /* Logging, if enabled. */
                if (stream != PNULL)
                    *stream << "Joining mailbox "
                            << mailboxFromNode->second.data->FullName().c_str()
                            << " (key="
                            << mailboxToKey[mailboxFromNode->second.data]
                            << ") with mailbox "
                            << mailboxToNode->second.data->FullName().c_str()
                            << " (key="
                            << mailboxToKey[mailboxToNode->second.data]
                            << ") with cost "
                            << cost
                            << " and edge key "
                            << edgeKey - 1
                            << ".\n";
            }
        }
    }
}

void CostCache::Dump(FILE* file)
{
    std::string prefix = "Cost cache matrix";
    DumpUtils::open_breaker(file, prefix);
    std::map<P_mailbox*, std::map<P_mailbox*, float>>::iterator outerCostsIt;
    std::map<P_mailbox*, float>::iterator innerCostsIt;
    for (outerCostsIt = costs.begin(); outerCostsIt != costs.end();
         outerCostsIt++)
    {
        for (innerCostsIt = outerCostsIt->second.begin();
             innerCostsIt != outerCostsIt->second.end(); innerCostsIt++)
        {
            fprintf(file, "%s -> %s = %f\n",
                    outerCostsIt->first->FullName().c_str(),
                    innerCostsIt->first->FullName().c_str(),
                    innerCostsIt->second);
        }
    }
    DumpUtils::close_breaker(file, prefix);

    prefix = "Path matrix";
    DumpUtils::open_breaker(file, prefix);
    std::map<P_mailbox*, std::map<P_mailbox*, P_mailbox*>>::iterator
        outerPathsIt;
    std::map<P_mailbox*, P_mailbox*>::iterator innerPathsIt;
    for (outerPathsIt = pathNext.begin(); outerPathsIt != pathNext.end();
         outerPathsIt++)
    {
        for (innerPathsIt = outerPathsIt->second.begin();
             innerPathsIt != outerPathsIt->second.end(); innerPathsIt++)
        {
            if (innerPathsIt->second != PNULL)
                fprintf(file, "%s -> %s = %s\n",
                        outerPathsIt->first->FullName().c_str(),
                        innerPathsIt->first->FullName().c_str(),
                        innerPathsIt->second->FullName().c_str());
        }
    }
    DumpUtils::close_breaker(file, prefix);

    fflush(file);
}

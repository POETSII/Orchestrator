#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_HARDWAREITERATOR_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_HARDWAREITERATOR_H

/* Defines iteration over the items in the POETS Engine. Key points:
 *
 * - This iterator allows the user to iterate over different levels of the
 *   hardware hierarchy.
 *
 * - This is not an iterator in the STL sense (and does not attempt to
 *   replicate that interface), though it uses STL iterators over individual
 *   levels of the hardware hierarchy.
 *
 * - Once the end of the stack is reached, the iterator cycles around and sets
 *   the "isWrapped" member to true.
 *
 * - Don't try to use this if the engine changes; just replace it with another
 *   instance (nothing is dynamic here).
 *
 * - Construction will throw an IteratorException if the engine contains a
 *   non-thread item (i.e. a board, mailbox, or core) that has no contained
 *   items. This is because iterating through the threads of the hardware makes
 *   no sense (and is unrealistic) if patches of the hardware are empty.
 */

#include "dumpchan.h"
#include "HardwareModel.h"
#include "IteratorException.h"
#include "pdigraph.hpp"

class HardwareIterator: public NameBase, public DumpChan
{
public:

    HardwareIterator(P_engine* engine, bool checkEngine=true);

    /* Getters */
    P_board* get_board();
    P_mailbox* get_mailbox();
    P_core* get_core();
    P_thread* get_thread();

    /* Convenience methods to determine whether the iterator at a certain level
     * of the hierarchy has changed value since the last time these methods
     * were called. */
    inline bool has_board_changed(){return has_item_changed(0)};
    inline bool has_mailbox_changed(){return has_item_changed(1)};
    inline bool has_core_changed(){return has_item_changed(2)};
    inline bool has_thread_changed(){return has_item_changed(3)};
    bool has_item_changed(int itemIndex);
    bool has_wrapped();

    /* Methods for (re)initialising iterators. These are called when
     * HardwareIterator is initialised, and when the item on the lower level
     * is incremented over a boundary. */
    void reset_all_iterators();
    void reset_board_iterator();
    void reset_mailbox_iterator();
    void reset_core_iterator();
    void reset_thread_iterator();

    /* Incrementers, returns incremented items */
    P_board* next_board();
    P_mailbox* next_mailbox();
    P_core* next_core();
    P_thread* next_thread();
    inline P_thread* operator++(){return next_thread();}

    void Dump(FILE* = stdout);

private:
    P_engine* engine;

    /* Checks whether or not the engine we're iterating over is correctly
     * defined. */
    void check_engine();

    /* The sub-iterators. Board and mailbox iterator iterate over graph nodes,
     * which are stored in a map in the graph object (as index_n). */
    std::map<AddressComponent, P_board*>::iterator boardIterator;
    std::map<AddressComponent, P_mailbox*>::iterator mailboxIterator;
    std::map<AddressComponent, P_core*>::iterator coreIterator;
    std::map<AddressComponent, P_thread*>::iterator threadIterator;

    /* Variables to store whether certain levels of the hierarchy have
     * changed. */
    std::vector<bool> itemChanged;
    bool isWrapped;

#endif
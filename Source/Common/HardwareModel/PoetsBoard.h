#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_POETSBOARD_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_POETSBOARD_H

/* Describes the concept of a "POETS Board" in the model of the POETS
   hardware stack.

   The POETS Board represents an FPGA board in the POETS Engine, and contains
   mailboxes.

   See the hardware model documentation for further information POETS
   boards. */

#include <sstream>

#include "HardwareAddress.h"
#include "dumpchan.h"
#include "NameBase.h"
#include "OwnershipException.h"
#include "PoetsBox.h"
#include "PoetsMailbox.h"
#include "pdigraph.hpp"

#define MAXIMUM_BREAKER_LENGTH 80
#define CALLBACK static void

/* Facilitate out-of-order includes. */
struct PoetsBox;
struct PoetsMailbox;

class PoetsBoard: public NameBase, protected DumpChan
{
public:
    PoetsBoard(std::string name);

    /* Boards live in boxes; this is the box that this board lives in. Note
       that boards also live in the engine by direct reference, but this is
       not captured in the board construct.

       Boards use boxes as their NameBase parent. */
    PoetsBox* parent = NULL;
    void on_being_contained_hook(PoetsBox* container);

    /* Boards contain mailboxes as a graph, mapped by their hardware address
       components. */
    pdigraph<AddressComponent, PoetsMailbox*,
        unsigned int, float,
        unsigned int, unsigned int> PoetsMailboxes;
    void contain(AddressComponent addressComponent, PoetsMailbox* mailbox);
    void connect(AddressComponent start, AddressComponent end, float weight);

    unsigned int dram;
    unsigned int supervisorMemory;
    float costBoardMailbox;

    void dump(FILE* = stdout);

private:
    /* Keys for arcs in the graph, incremented when items are connected. */
    unsigned int arcKey;
};

#endif

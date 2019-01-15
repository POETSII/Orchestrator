#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_POETSBOX_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_POETSBOX_H

/* Describes the concept of a "POETS Box" in the model of the POETS hardware
   stack.

   The POETS Box represents a machine (e.g. a desktop machine) in the POETS
   Engine, and contains boards.

   See the hardware model documentation for further information POETS boxes. */

#include <sstream>

#include "AddressableItem.h"
#include "dumpchan.h"
#include "NameBase.h"
#include "OwnershipException.h"
#include "P_engine.h"
#include "P_board.h"

#define MAXIMUM_BREAKER_LENGTH 80

/* Facilitate out-of-order includes. */
struct P_engine;
struct P_board;

class P_box: public AddressableItem, public NameBase, protected DumpChan
{
public:
    P_box(std::string name);

    /* Destruction */
    ~P_box();
    void clear();

    /* Boxes are contained by a POETS Engine. */
    P_engine* parent = NULL;
    void on_being_contained_hook(P_engine* container);

    /* Boxes contain boards, mapped by their hardware address components. Note
       that engines also contain a graph of boards. */
    std::map<AddressComponent, P_board*> P_boards;
    void contain(AddressComponent addressComponent, P_board* board);

    unsigned int supervisorMemory;
    float costBoxBoard;

    void dump(FILE* = stdout);
};

#endif

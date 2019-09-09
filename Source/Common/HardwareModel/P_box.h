#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_P_BOX_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_P_BOX_H

/* Describes the concept of a "POETS Box" in the model of the POETS hardware
   stack.

   The POETS Box represents a machine (e.g. a desktop machine) in the POETS
   Engine, and contains boards.

   See the hardware model documentation for further information POETS boxes. */

#include <sstream>

#include "AddressableItem.h"
#include "DumpUtils.h"
#include "NameBase.h"
#include "OwnershipException.h"
#include "P_board.h"
#include "P_engine.h"
#include "P_super.h"

/* Facilitate out-of-order includes. */
class P_engine;
class P_board;

class P_box: public AddressableItem, public NameBase, public DumpChan
{
public:
    P_box(std::string name);

    /* Destruction */
    ~P_box();
    void clear();

    /* Boxes are contained by a POETS Engine. */
    P_engine* parent;// = NULL;
    void on_being_contained_hook(P_engine* container);

    /* Boxes contain boards. Note that engines also contain a graph of
     * boards. */
    std::vector<P_board*> P_boardv;
    void contain(AddressComponent addressComponent, P_board* board);

    /* Boxes contain supervisors. Boards can also contain supervisors, but
     * boards point to their parent's (i.e. their box) supervisor vector. */
    std::vector<P_super*> P_superv;

    unsigned int supervisorMemory;
    float costBoxBoard;

    void Dump(FILE* = stdout);
};

#endif

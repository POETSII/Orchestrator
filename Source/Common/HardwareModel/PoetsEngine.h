#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_POETSENGINE_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_POETSENGINE_H

/* Describes the concept of a "POETS Engine" in the model of the POETS hardware
   stack.

   The POETS Engine is a self-contained model of the hardware that contains
   boxes and a graph of boards.

   See the hardware model documentation for further information POETS
   engines. */

#include <sstream>

#include "HardwareAddress.h"
#include "dumpchan.h"
#include "NameBase.h"
#include "OwnershipException.h"
#include "PoetsBox.h"
#include "PoetsBoard.h"
#include "pdigraph.hpp"

#define MAXIMUM_BREAKER_LENGTH 80
#define CALLBACK static void

/* Facilitate out-of-order includes. */
struct PoetsBox;
struct PoetsBoard;

class PoetsEngine: public NameBase, protected DumpChan
{
public:
    PoetsEngine(std::string name);

    /* Engines are not contained by anything, but are have OrchBase as a
       namebase parent. */

    /* Engines contain boxes, mapped by their hardware address component. */
    void contain(AddressComponent addressComponent, PoetsBox* box);
    std::map<AddressComponent, PoetsBox*> PoetsBoxes;

    /* Engines also contain a graph of boards, again mapped by their hardware
       components. */
    pdigraph<AddressComponent, PoetsBoard*,
        unsigned int, float,
        unsigned int, unsigned int> PoetsBoards;
    void contain(AddressComponent addressComponent, PoetsBoard* board);
    void connect(AddressComponent start, AddressComponent end, float weight);

    float costExternalBox;

    void dump(FILE* = stdout);

private:
    /* Keys for arcs in the graph, incremented when items are connected. */
    unsigned int arcKey;
};

#endif

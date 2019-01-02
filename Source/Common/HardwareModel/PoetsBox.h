#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_POETSBOX_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_POETSBOX_H

/* Describes the concept of a "POETS Box" in the model of the POETS hardware
   stack.

   The POETS Box represents a machine (e.g. a desktop machine) in the POETS
   Engine, and contains boards.

   See the hardware model documentation for further information POETS boxes. */

#include <sstream>

#include "HardwareAddress.h"
#include "dumpchan.h"
#include "NameBase.h"
#include "OwnershipException.h"
#include "PoetsEngine.h"
#include "PoetsBoard.h"

#define MAXIMUM_BREAKER_LENGTH 80

/* Facilitate out-of-order includes. */
struct PoetsEngine;
struct PoetsBoard;

class PoetsBox: public NameBase, protected DumpChan
{
public:
    PoetsBox(std::string name);

    /* Boxes are contained by a POETS Engine. */
    PoetsEngine* parent = NULL;
    void on_being_contained_hook(PoetsEngine* container);

    /* Boxes contain boards, mapped by their hardware address components. Note
       that engines also contain a graph of boards. */
    std::map<AddressComponent, PoetsBoard*> PoetsBoards;
    void contain(AddressComponent addressComponent, PoetsBoard* board);

    unsigned int supervisorMemory;
    float costBoxBoard;

    void dump(FILE* = stdout);
};

#endif

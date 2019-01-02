#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_POETSMAILBOX_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_POETSMAILBOX_H

/* Describes the concept of a "POETS Mailbox" in the model of the POETS
   hardware stack.

   The POETS Mailbox services cores, and is contained by an FPGA board on the
   next level of the hierarchy.

   See the hardware model documentation for further information POETS
   mailboxes. */

#include <sstream>

#include "HardwareAddress.h"
#include "dumpchan.h"
#include "NameBase.h"
#include "OwnershipException.h"
#include "PoetsBoard.h"
#include "PoetsCore.h"

#define MAXIMUM_BREAKER_LENGTH 80

/* Facilitate out-of-order includes. */
struct PoetsBoard;
struct PoetsCore;

class PoetsMailbox: public NameBase, protected DumpChan
{
public:
    PoetsMailbox(std::string name);

    /* Mailboxes live in boards; this is the board that this mailbox lives
       in. */
    PoetsBoard* parent = NULL;
    void on_being_contained_hook(PoetsBoard* container);

    /* Mailboxes contain cores, mapped by their hardware address components. */
    std::map<AddressComponent, PoetsCore*> PoetsCores;
    void contain(AddressComponent addressComponent, PoetsCore* core);

    float costCoreCore;
    float costMailboxCore;

    void dump(FILE* = stdout);
};

#endif

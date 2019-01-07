#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_POETSCORE_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_POETSCORE_H

/* Describes the concept of a "POETS Core" in the model of the POETS hardware
   stack.

   The POETS Core is a representation of a core on an FPGA board in the POETS
   hardware, and is served by a "mailbox".

   See the hardware model documentation for further information POETS
   cores. */

#include <sstream>

#include "AddressableItem.h"
#include "Bin.h"
#include "dumpchan.h"
#include "NameBase.h"
#include "OwnershipException.h"
#include "PoetsMailbox.h"
#include "PoetsThread.h"

#define MAXIMUM_BREAKER_LENGTH 80

/* Facilitate out-of-order includes. */
struct PoetsMailbox;
struct PoetsThread;

class PoetsCore: public AddressableItem, public NameBase, protected DumpChan
{
public:
    PoetsCore(std::string name);

    /* Destruction */
    ~PoetsCore();
    void clear();

    /* Cores live in mailboxes; this is the mailbox that this core lives in. */
    PoetsMailbox* parent = NULL;
    void on_being_contained_hook(PoetsMailbox* container);

    /* Cores contain threads, mapped by their hardware address components. */
    std::map<AddressComponent, PoetsThread*> PoetsThreads;
    void contain(AddressComponent addressComponent, PoetsThread* thread);

    /* Instruction and data binaries are loaded onto cores. */
    Bin* dataBinary;
    Bin* instructionBinary;
    unsigned int dataMemory;
    unsigned int instructionMemory;

    float costCoreThread;
    float costThreadThread;

    void dump(FILE* = stdout);
};

#endif

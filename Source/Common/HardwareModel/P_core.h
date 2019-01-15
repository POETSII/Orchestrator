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
#include "P_mailbox.h"
#include "P_thread.h"

#define MAXIMUM_BREAKER_LENGTH 80

/* Facilitate out-of-order includes. */
struct P_mailbox;
struct P_thread;

class P_core: public AddressableItem, public NameBase, protected DumpChan
{
public:
    P_core(std::string name);

    /* Destruction */
    ~P_core();
    void clear();

    /* Cores live in mailboxes; this is the mailbox that this core lives in. */
    P_mailbox* parent = NULL;
    void on_being_contained_hook(P_mailbox* container);

    /* Cores contain threads, mapped by their hardware address components. */
    std::map<AddressComponent, P_thread*> P_threadm;
    void contain(AddressComponent addressComponent, P_thread* thread);

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

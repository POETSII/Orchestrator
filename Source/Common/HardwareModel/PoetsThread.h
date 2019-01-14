#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_POETSTHREAD_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_POETSTHREAD_H

/* Describes the concept of a "POETS Thread" in the model of the POETS hardware
   stack.

   The POETS Thread is a representation of a thread spawned on a core in the
   POETS engine.

   See the hardware model documentation for further information POETS
   threads. */

#include <list>

#include "AddressableItem.h"
#include "dumpchan.h"
#include "macros.h"
#include "NameBase.h"
#include "PoetsCore.h"
#include "P_device.h"

#define MAXIMUM_BREAKER_LENGTH 80

/* Facilitate out-of-order includes. */
struct PoetsCore;

class PoetsThread: public AddressableItem, public NameBase, protected DumpChan
{
public:
    PoetsThread(std::string name);

    /* Threads live in cores; this is the core that this thread lives in, if
       any. */
    PoetsCore* parent = NULL;
    void on_being_contained_hook(PoetsCore* container);

    /* Not part of the hardware, this list acts as an interface between the
       hardware model and the placement implementation; POETS devices (which
       are part of the task graph) get mapped onto threads. Multiple devices
       are services by a single thread. */
    std::list<P_device *> PoetsDevices;

    unsigned int dataMemoryAddress;
    unsigned int instructionMemoryAddress;

    void dump(FILE* = stdout);
};

#endif

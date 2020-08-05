#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_P_THREAD_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_P_THREAD_H

/* Describes the concept of a "POETS Thread" in the model of the POETS hardware
 * stack.
 *
 * The POETS Thread is a representation of a thread spawned on a core in the
 * POETS engine.
 *
 * See the hardware model documentation for further information on POETS
 * threads. */

#include <list>

#include "AddressableItem.h"
#include "DumpUtils.h"
#include "macros.h"
#include "NameBase.h"
#include "P_core.h"
#include "P_device.h"

/* Facilitate out-of-order includes. */
class P_core;

class P_thread: public AddressableItem, public NameBase, public DumpChan
{
public:
    P_thread(std::string name);

    /* Threads live in cores; this is the core that this thread lives in, if
     * any. */
    P_core* parent = NULL;
    void on_being_contained_hook(P_core* container);

    unsigned int dataMemoryAddress;
    unsigned int instructionMemoryAddress;

    void Dump(FILE* = stdout);
};

#endif

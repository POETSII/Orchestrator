#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_P_PORT_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_P_PORT_H

/* Placeholder object for a pin in the Engine's board graph, or in a board's
 * mailbox graph. Is currently a stub.
 *
 * See the hardware model documentation for further information on ports, and
 * for what they may be used for in future. */

#include "HardwareDumpUtils.h"
#include "NameBase.h"
#include "pdigraph.hpp"

class P_port : public NameBase, public DumpChan
{
public:
    P_port();
    P_port(NameBase* parent);
    void Dump(FILE* = stdout);
};

#endif

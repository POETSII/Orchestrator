#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_MOTHERSHIP_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_MOTHERSHIP_H

/* Describes the Mothership object, which is instantiated as part of the
 * Mothership process. */

#include "AppDB.h"
#include "CommonBase.h"
#include "HostLink.h"
#include "OSFixes.hpp"
#include "SuperDB.h"
#include "ThreadComms.h"

class Mothership: public CommonBase
{
public:
    Mothership(int argc, char** argv);  /* Args for CommonBase */

    void dump(ofstream*);
    void go();
    std::string task_from_swaddr(uint32_t address);
    void queue_mpi_message(PMsg_p message, unsigned commIndex);

    AppDB appdb;
    HostLink backend;
    SuperDB superdb;
    ThreadComms threading;

private:
    void load_backend();
    void setup_mpi_hooks();

    unsigned HandleExit(PMsg_p* message, unsigned commIndex);
    unsigned HandleKill(PMsg_p* message, unsigned commIndex);
    unsigned HandleAppSpec(PMsg_p* message, unsigned commIndex);
    unsigned HandleAppDist(PMsg_p* message, unsigned commIndex);
    unsigned HandleAppSupd(PMsg_p* message, unsigned commIndex);
    unsigned HandleCmndRecl(PMsg_p* message, unsigned commIndex);
    unsigned HandleCmndInit(PMsg_p* message, unsigned commIndex);
    unsigned HandleCmndRun(PMsg_p* message, unsigned commIndex);
    unsigned HandleCmndStop(PMsg_p* message, unsigned commIndex);
    unsigned HandleBendCnc(PMsg_p* message, unsigned commIndex);
    unsigned HandleBendSupr(PMsg_p* message, unsigned commIndex);
    unsigned HandlePkts(PMsg_p* message, unsigned commIndex);
    unsigned HandleDump(PMsg_p* message, unsigned commIndex);
};

#endif
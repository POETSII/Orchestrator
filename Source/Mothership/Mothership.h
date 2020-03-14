#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_MOTHERSHIP_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_MOTHERSHIP_H

/* Describes the Mothership object, which is instantiated as part of the
 * Mothership process. */

class ThreadComms;

#include "AppDB.h"
#include "CommonBase.h"
#include "HostLink.h"
#include "OSFixes.hpp"
#include "Pglobals.h"
#include "SuperDB.h"
#include "ThreadComms.h"

class Mothership: public CommonBase
{
public:
    Mothership(int argc, char** argv);  /* Args for CommonBase */

    void dump(std::ofstream*);
    void go();
    void mpi_spin(){MPISpinner();};
    std::string task_from_swaddr(uint32_t address);
    void queue_mpi_message(PMsg_p message, unsigned commIndex);

    AppDB appdb;
    HostLink backend;
    SuperDB superdb;
    ThreadComms threading;

    /* More stuff needed for CommonBase to work. */
    typedef unsigned (Mothership::*pMeth)(PMsg_p*, unsigned);
    typedef std::map<unsigned, pMeth> FnMap_t;
    std::vector<FnMap_t*> FnMapx;
#include "Decode.cpp"

private:
    void load_backend();
    void setup_mpi_hooks();

    unsigned HandleExit(PMsg_p* message, unsigned commIndex);
    unsigned HandleSystKill(PMsg_p* message, unsigned commIndex);
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

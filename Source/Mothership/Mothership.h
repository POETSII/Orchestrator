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

    /* Methods for transitioning applications through states, triggered by
     * different Q::APP message keys. */
    void initialise_application(AppInfo*);
    void run_application(AppInfo*);
    void stop_application(AppInfo*);
    void recall_application(AppInfo*);

    /* More stuff needed for CommonBase to work. */
    typedef unsigned (Mothership::*pMeth)(PMsg_p*, unsigned);
    typedef std::map<unsigned, pMeth> FnMap_t;
    std::vector<FnMap_t*> FnMapx;
#include "Decode.cpp"

private:
    void load_backend();
    void setup_mpi_hooks();

    /* Methods for handling MPI messages. */
    unsigned handle_exit(PMsg_p* message, unsigned commIndex);
    unsigned handle_syst_kill(PMsg_p* message, unsigned commIndex);
    unsigned handle_app_spec(PMsg_p* message, unsigned commIndex);
    unsigned handle_app_dist(PMsg_p* message, unsigned commIndex);
    unsigned handle_app_supd(PMsg_p* message, unsigned commIndex);
    unsigned handle_cmnd_recl(PMsg_p* message, unsigned commIndex);
    unsigned handle_cmnd_init(PMsg_p* message, unsigned commIndex);
    unsigned handle_cmnd_run(PMsg_p* message, unsigned commIndex);
    unsigned handle_cmnd_stop(PMsg_p* message, unsigned commIndex);
    unsigned handle_bend_cnc(PMsg_p* message, unsigned commIndex);
    unsigned handle_bend_supr(PMsg_p* message, unsigned commIndex);
    unsigned handle_pkts(PMsg_p* message, unsigned commIndex);
    unsigned handle_dump(PMsg_p* message, unsigned commIndex);
};

#endif

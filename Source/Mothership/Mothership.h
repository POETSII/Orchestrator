#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_MOTHERSHIP_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_MOTHERSHIP_H

/* Describes the Mothership object, which is instantiated as part of the
 * Mothership process. */

class ThreadComms;

#include "Debug.h"
#include <stdarg.h>  /* Used by debug_post */

#include "AppDB.h"
#include "CommonBase.h"
#include "HostLink.h"
#include "InstrumentationWriter.h"
#include "LogPacketManager.h"
#include "OSFixes.hpp"
#include "Pglobals.h"
#include "SuperDB.h"
#include "ThreadComms.h"

#define SLOW_SPIN_SLEEP_PERIOD 10  /* Milliseconds */

class Mothership: public CommonBase
{
public:
    Mothership(int argc, char** argv);  /* Args for CommonBase */
    ~Mothership();

    void dump(std::ofstream*);
    void go();
    void mpi_spin(){MPISpinner();};
    void queue_mpi_message(PMsg_p* message);
    bool debug_post(int code, unsigned numArgs, ...);

    AppDB appdb;
    HostLink* backend;
    InstrumentationWriter instrumentation;
    LogPacketManager logging;
    SuperDB superdb;
    ThreadComms threading;

    /* Root directory for supervisor output, relative to home. */
    std::string userOutDir;

    /* Methods for transitioning applications through states, triggered by
     * different Q::APP message keys. */
    void initialise_application(AppInfo*);
    void run_application(AppInfo*);
    void stop_application(AppInfo*);
    void recall_application(AppInfo*);

    /* Methods for handling MPI messages (called by consumer threads). */
    unsigned handle_msg_app_spec(PMsg_p* message);
    unsigned handle_msg_app_dist(PMsg_p* message);
    unsigned handle_msg_app_supd(PMsg_p* message);
    unsigned handle_msg_cmnd_brkn(PMsg_p* message);
    unsigned handle_msg_cmnd_recl(PMsg_p* message);
    unsigned handle_msg_cmnd_init(PMsg_p* message);
    unsigned handle_msg_cmnd_run(PMsg_p* message);
    unsigned handle_msg_cmnd_stop(PMsg_p* message);
    unsigned handle_msg_bend_cnc(PMsg_p* message);
    unsigned handle_msg_bend_supr(PMsg_p* message);
    unsigned handle_msg_path(PMsg_p* message);
    unsigned handle_msg_pkts(PMsg_p* message);
    unsigned handle_msg_dump(PMsg_p* message);

    /* Methods for handling command-and-control packets received by the
     * Mothership from the compute fabric (called by consumer threads). */
    void handle_pkt_instr(P_Pkt_t* packet);
    void handle_pkt_log(P_Pkt_t* packet);
    void handle_pkt_barrier(P_Pkt_t* packet);
    void handle_pkt_stop(P_Pkt_t* packet);
    void handle_pkt_kill(P_Pkt_t* packet);
    /* Reducing code duplication. */
    void handle_pkt_barrier_or_stop(P_Pkt_t* packet, bool stop=false);

    /* More stuff needed for CommonBase to work. */
    typedef unsigned (Mothership::*pMeth)(PMsg_p*);
    std::map<unsigned, pMeth> FnMap;
#include "Decode.cpp"

private:
    void load_backend();
    void setup_mpi_hooks();

    /* Reduces code duplication in some app transition methods. */
    void send_cnc_packet_to_all(AppInfo* app, uint8_t opcode);

    /* Methods for handling MPI messages via MPIInputBroker */
    unsigned handle_msg_exit(PMsg_p* message);
    unsigned handle_msg_syst_kill(PMsg_p* message);
    unsigned handle_msg_app(PMsg_p* message);
    unsigned handle_msg_cnc(PMsg_p* message);

    /* Methods for safely decoding MPI messages with certain field
     * configurations. */
    bool decode_app_dist_message(PMsg_p* message, std::string* appName,
                                 std::string* codePath, std::string* dataPath,
                                 uint32_t* coreAddr,
                                 std::vector<uint32_t>* threadsExpected);
    bool decode_app_supd_message(PMsg_p* message, std::string* appName,
                                 std::string* soPath);
    bool decode_app_spec_message(PMsg_p* message, std::string* appName,
                                 uint32_t* distCount, uint8_t* appNumber);
    bool decode_addresses_message(PMsg_p* message,
                                  std::vector<uint32_t>* addresses,
                                  unsigned index=0);
    bool decode_addressed_packets_message(PMsg_p* message,
        std::vector<P_Addr_Pkt_t>* packets, unsigned index=0);
    bool decode_char_message(PMsg_p* message, unsigned char* result,
                             unsigned index=0);
    bool decode_packets_message(PMsg_p* message,
                                std::vector<P_Pkt_t>* packets,
                                unsigned index=0);
    bool decode_string_message(PMsg_p* message, std::string* result,
                               unsigned index=0);
    bool decode_unsigned_message(PMsg_p* message, unsigned* result,
                                 unsigned index=0);

    /* Methods for provisioning the API for Supervisors. */
    bool provision_supervisor_api(std::string appName);
    void supervisor_api_stop_application(std::string appName);

    /* An app is broken! */
    void tell_root_app_is_broken(std::string appName);

    /* Supervisor spinning (virtual from CommonBase). */
    void OnIdle();
};

#endif

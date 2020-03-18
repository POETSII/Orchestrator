#include "Mothership.h"

Mothership::Mothership(int argc, char** argv):
    CommonBase(argc, argv, std::string(csMOTHERSHIPproc),
               std::string(__FILE__)),
    threading(ThreadComms(this))
{
}

/* Dumps dumpable datastructures to a stream. Note that the CommonBase data
 * structure is not dumped by this, because it requires a file pointer (and
 * there's no portable way to interface the two). See the dump handler. */
void Mothership::dump(std::ofstream* stream)
{
    *stream << "Mothership (" << POETS::get_hostname() << ") dump:\n";
    appdb.dump(stream);
    superdb.dump(stream);
}

/* Loads the backend and sets the threads spinning, waiting for them to
 * exit. */
void Mothership::go()
{
    setup_mpi_hooks();
    load_backend();
    threading.go();
}

/* Given a software address, returns the name of the task.
 *
 * NB: This queries SBase (which is currently not here), so this method is a
 * stub for now. */
std::string Mothership::task_from_swaddr(uint32_t address)
{
    return "WARNING (Mothership::task_from_swaddr): This method is a stub!";
}

/* Sends a message using MPI to the process defined in the message.
 *
 * You might reasonably thing this is largely superfluous, but this is here to
 * aid debugging as an elegant hook for all outgoing MPI communications. The
 * compiler (assuming it's not bad) will inline it under optimisation. */
void Mothership::queue_mpi_message(PMsg_p message, unsigned commIndex)
{
    message.Send();
}

/* Loads the compute backend, crashing violently if it does (for now). */
void Mothership::load_backend()
{
    /* Perhaps some box-graph arguments should be passed to HostLink in the
     * one-Mothership-over-many-boxes case, but do we even want to support that
     * once we're multi-box? (It was sarcasm - we don't). */
    backend = HostLink();
}

/* Sets up the function map for MPI communications. See the CommonBase
 * documentation for more information on how this is expected to work. */
void Mothership::setup_mpi_hooks()
{
    FnMapx.push_back(new FnMap_t);
    (*FnMapx[0])[PMsg_p::KEY(Q::EXIT)] = &Mothership::handle_exit;
    (*FnMapx[0])[PMsg_p::KEY(Q::SYST,Q::KILL)] = &Mothership::handle_syst_kill;
    (*FnMapx[0])[PMsg_p::KEY(Q::APP,Q::SPEC)] = &Mothership::handle_cnc;
    (*FnMapx[0])[PMsg_p::KEY(Q::APP,Q::DIST)] = &Mothership::handle_cnc;
    (*FnMapx[0])[PMsg_p::KEY(Q::APP,Q::SUPD)] = &Mothership::handle_cnc;
    (*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::RECL)] = &Mothership::handle_cnc;
    (*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::INIT)] = &Mothership::handle_cnc;
    (*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::RUN)] = &Mothership::handle_cnc;
    (*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::STOP)] = &Mothership::handle_cnc;
    (*FnMapx[0])[PMsg_p::KEY(Q::BEND,Q::CNC)] = &Mothership::handle_cnc;
    (*FnMapx[0])[PMsg_p::KEY(Q::BEND,Q::SUPR)] = &Mothership::handle_app;
    (*FnMapx[0])[PMsg_p::KEY(Q::PKTS)] = &Mothership::handle_app;
    (*FnMapx[0])[PMsg_p::KEY(Q::DUMP)] = &Mothership::handle_cnc;
}

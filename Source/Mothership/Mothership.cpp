#include "Mothership.h"

Mothership::Mothership(int argc, char** argv):
    CommonBase(argc, argv, std::string(csMOTHERSHIPproc),
               std::string(__FILE__)),
    backend(PNULL),
    threading(ThreadComms(this))
{
    try
    {
        instrumentation = InstrumentationWriter();
    }
    catch (InstrumentationException &e)
    {
        Post(509, e.message);
    }
}

Mothership::~Mothership()
{
    /* Tear down MPI function map. */
    WALKVECTOR(FnMap_t*, FnMapx, F) delete *F;

    /* Tear down backend. */
    if (backend != PNULL)
    {
        DebugPrint("[MOTHERSHIP] Closing down the compute backend...\n");
        delete backend;
        DebugPrint("[MOTHERSHIP] Backend closed.\n");
        DebugPrint("[MOTHERSHIP] Disconnecting from MPI (goodbye world).\n");
    }
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
    threading.go();  /* Blocks until all threads exit. */
}

/* Sends a message using MPI to the process defined in the message.
 *
 * You might reasonably thing this is largely superfluous, but this is here to
 * aid debugging as an elegant hook for all outgoing MPI communications. The
 * compiler (assuming it's not bad) will inline it under optimisation.
 *
 * The argument is an address to avoid difficulties working with the copy
 * constructor.  */
void Mothership::queue_mpi_message(PMsg_p* message)
{
    message->Send();
}

/* Loads the compute backend. Crashes violently if the compute backend crashes
 * violently. */
void Mothership::load_backend()
{
    /* Perhaps some box-graph arguments should be passed to HostLink in the
     * one-Mothership-over-many-boxes case, but do we even want to support that
     * once we're multi-box? (It was sarcasm - we don't). */
    DebugPrint("[MOTHERSHIP] Loading Tinsel backend...\n");
    pthread_mutex_lock(&(threading.mutex_backend_api));
    if (backend != PNULL) delete backend;
    backend = new HostLink();
    pthread_mutex_unlock(&(threading.mutex_backend_api));
    DebugPrint("[MOTHERSHIP] Tinsel backend loaded.\n");
}

/* Posts a debugging message, if debugging is enabled. Returns as with
 * CommonBase::Post. If debugging is not enabled, always returns true and does
 * nothing of use. */
bool Mothership::debug_post(int code, unsigned numArgs, ...)
{
    #if ORCHESTRATOR_DEBUG
    std::vector<std::string> strings;
    va_list args;
    unsigned argIndex;

    /* Put all of the arguments into the vector. */
    va_start(args, numArgs);
    for (argIndex = 0; argIndex < numArgs; argIndex++)
        strings.push_back(va_arg(args, const char*));
    va_end(args);

    /* Throw over the fence. */
    bool rc = Post(code, strings);
    if (!rc)
    {
        std::string error;
        error = dformat("Mothership WARNING: Couldn't post message with code "
                        "%d ", code);
        if (!strings.empty()) error.append("and strings ");
        for (std::vector<std::string>::iterator strIt = strings.begin();
             strIt != strings.end(); strIt++)
            error.append(dformat("'%s' ", strIt->c_str()));
        fprintf(stderr, "%s\n", error.c_str());
    }
    return rc;
    #else
    return true;
    #endif
}

/* Sets up the function map for MPI communications. See the CommonBase
 * documentation for more information on how this is expected to work. */
void Mothership::setup_mpi_hooks()
{
    DebugPrint("[MOTHERSHIP] Setting up MPI hooks.\n");
    FnMapx.push_back(new FnMap_t);
    (*FnMapx[0])[PMsg_p::KEY(Q::EXIT)] = &Mothership::handle_msg_exit;
    (*FnMapx[0])[PMsg_p::KEY(Q::SYST,Q::KILL)] =
        &Mothership::handle_msg_syst_kill;
    (*FnMapx[0])[PMsg_p::KEY(Q::APP,Q::SPEC)] = &Mothership::handle_msg_cnc;
    (*FnMapx[0])[PMsg_p::KEY(Q::APP,Q::DIST)] = &Mothership::handle_msg_cnc;
    (*FnMapx[0])[PMsg_p::KEY(Q::APP,Q::SUPD)] = &Mothership::handle_msg_cnc;
    (*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::RECL)] = &Mothership::handle_msg_cnc;
    (*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::INIT)] = &Mothership::handle_msg_cnc;
    (*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::RUN)] = &Mothership::handle_msg_cnc;
    (*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::STOP)] = &Mothership::handle_msg_cnc;
    (*FnMapx[0])[PMsg_p::KEY(Q::BEND,Q::CNC)] = &Mothership::handle_msg_cnc;
    (*FnMapx[0])[PMsg_p::KEY(Q::BEND,Q::SUPR)] = &Mothership::handle_msg_app;
    (*FnMapx[0])[PMsg_p::KEY(Q::PKTS)] = &Mothership::handle_msg_app;
    (*FnMapx[0])[PMsg_p::KEY(Q::DUMP)] = &Mothership::handle_msg_cnc;
}

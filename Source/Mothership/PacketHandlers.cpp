/* This source file defines the Mothership methods for handling incoming
 * packets from the compute fabric. These handlers are to be exclusively called
 * by MPICncResolver (which also handles command-and-control tasks from
 * Root).
 *
 * See the Mothership documentation for descriptions of what these methods
 * do. */

#include "Mothership.h"

void Mothership::handle_pkt_instr(P_Pkt_t* packet)
{
    try
    {
        instrumentation.consume_instrumentation_packet(packet);
    }

    catch (InstrumentationException &e)
    {
        Post(409, e.message);
    }
}

void Mothership::handle_pkt_log(P_Pkt_t* packet)
{
    std::string message;
    logging.consume_log_packet(packet, &message);
    if (!message.empty()) Post(410, message);
}

/* Stub */
void Mothership::handle_pkt_barrier(P_Pkt_t* packet)
{}

/* Stub */
void Mothership::handle_pkt_stop(P_Pkt_t* packet)
{}

void Mothership::handle_pkt_kill(P_Pkt_t* packet)
{
    /* We manage killing the Mothership via MPI - the message is consumed by
     * MPIInputBroker, which tells everything to stop gracefully. */
    PMsg_p message;
    message.Key(Q::EXIT);
    message.Tgt(Urank);  /* Send to ourselves */
    queue_mpi_message(message);
}

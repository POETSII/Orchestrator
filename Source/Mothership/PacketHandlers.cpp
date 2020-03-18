/* This source file defines the Mothership methods for handling incoming
 * packets from the compute fabric. These handlers are to be exclusively called
 * by MPICncResolver (which also handles command-and-control tasks from
 * Root).
 *
 * See the Mothership documentation for descriptions of what these methods
 * do. */

#include "Mothership.h"

/* Stub */
void Mothership::handle_pkt_instr(P_Pkt_t* packet)
{}

/* Stub */
void Mothership::handle_pkt_log(P_Pkt_t* packet)
{}

/* Stub */
void Mothership::handle_pkt_barrier(P_Pkt_t* packet)
{}

/* Stub */
void Mothership::handle_pkt_stop(P_Pkt_t* packet)
{}

/* Stub */
void Mothership::handle_pkt_kill(P_Pkt_t* packet)
{}

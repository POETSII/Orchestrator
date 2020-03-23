#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_LOGMANAGER_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_LOGMANAGER_H

/* Describes how logging packets are handled by the Mothership.
 *
 * Log messages are sent by softswitches in chunks - messages are processed
 * (i.e. Post-ed, for now) once all of the chunks for a message have been
 * received, where the number of chunks is identified from the "latest"
 * (i.e. highest sequence number) message received.
 *
 * This approach has a few weaknesses, as explained in the Softswitch
 * documentation. We'll tackle them when there's more time. */

#include <map>
#include <vector>

#include "dfprintf.h"
#include "flat.h"
#include "poets_pkt.h"

struct ThreadLogDatum
{
    unsigned packetCountReceived;
    unsigned packetCountExpected;
    /* An array of buffers - one per packet up to a hardcoded maximum. */
    P_Log_Pkt_Pyld_t payloads[P_MAX_LOGPKT_FRAG];
};

class LogManager
{
public:
    /* This 'data' buffer holds payloads of log packets that haven't been
     * transmitted yet. The key for this map is the source compute-thread
     * address. */
    std::map<uint32_t, ThreadLogDatum> data;
    void consume_log_packet(P_Pkt_t*, std::string*);
};
#endif

/* Stages and sends logging information about. */

#include "LogPacketManager.h"

/* Take a packet and add it to the data section. If a message has been
 * completed by this operation, write the message to 'out' ('out' is cleared
 * otherwise). */
void LogPacketManager::consume_log_packet(P_Pkt_t* packet, std::string* out)
{

    P_Log_Pkt_Pyld_t* packetDatum;
    ThreadLogDatum* cumulativeDatum;
    std::map<uint32_t, ThreadLogDatum>::iterator dataFinder;
    uint32_t source;

    out->clear();

    /* Extract source address for instrumentation packet, so that the compute
     * thread can be identified. The softswitch explicitly puts the hardware
     * address of the sender of instrumentation packets in the pinAddr
     * field. */
    source = packet->header.pinAddr;

    /* Extract logging data and sequencing from the packet. */
    packetDatum = reinterpret_cast<P_Log_Pkt_Pyld_t*>(packet->payload);

    /* Get the cumulative datum for this thread, if it exists. If it does not
     * exist, create a new entry in 'data'. */
    dataFinder = data.find(source);

    /* Existence */
    if (dataFinder != data.end())
    {
        cumulativeDatum = &(dataFinder->second);
    }

    /* Non-existence - create an entry using packet information. */
    else
    {
        cumulativeDatum = &(data[source]);
        cumulativeDatum->packetCountReceived = 0;  /* Good Samaritan */
        cumulativeDatum->packetCountExpected = 0;
    }

    /* Update cumulative information:
     *
     * - We've received a packet, so increment that.
     *
     * - Compute the number of packets we expect to receive from the sequence
     *   number of this packet - if we've received a "later" packet before this
     *   one (which is intended), then we don't change the number we expect to
     *   receive.
     *
     * - And the cumulative message itself, obviously. */
    cumulativeDatum->packetCountReceived++;
    if (cumulativeDatum->packetCountExpected < packetDatum->seq)
    {
        cumulativeDatum->packetCountExpected = packetDatum->seq;
    }
    memcpy(&(cumulativeDatum->payloads[packetDatum->seq]),
           packetDatum, p_pkt_pyld_size);

    /* If we have now received all of the information we expected to receive,
     * 'Post' the log message, and remove the datum entry from the 'data'
     * map. */
    if(cumulativeDatum->packetCountReceived ==
       (cumulativeDatum->packetCountExpected + 1))
    {
        char message[(p_logpkt_pyld_size << P_LOG_MAX_LOGPKT_FRAG) + 1];
        char* messageChunk = message;

        /* Pilfered from GMB, with a couple of renamed variables... it's all
         * going to be rewritten anyway. */
#ifdef TRIVIAL_LOG_HANDLER
        P_Log_Pkt_Pyld_t* pyld;

        // Re-assemble the full log message string
        for(unsigned int i = 0; i < cumulativeDatum->packetCountReceived; i++)
        {
            pyld = &(cumulativeDatum->payloads[
                         cumulativeDatum->packetCountExpected]);

            memcpy(messageChunk, pyld->payload, p_logpkt_pyld_size);

            messageChunk += p_logpkt_pyld_size;
            cumulativeDatum->packetCountExpected--;
        }
#else
        // (in)sanity check
        strcpy(message, "ERROR: No Log Handler Defined!");
#endif
        /* Remove entry from the map. */
        data.erase(source);

        /* Lob over the fence */
        *out = dformat("%u: %s", source, message);
    }
}

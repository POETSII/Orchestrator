#include "softswitch.h"
#include "tinsel.h"

#include <cstring>  // needed for strlen and memcpy

/*------------------------------------------------------------------------------
 * A very trivial handler log sender that only sends the format string section
 * of a handler_log call.
 *
 * The pinAddr of the packet header is (ab)used for the source device's HW
 * address.
 *
 * Messages up to 220 characters (with the default tinsel config) are supported
 * through simplistic packet fragmentation. The first 8-bits of payload are used
 * to facilitate this by including a decrementing sequence number (e.g.
 * the first packet fragment has the highest sequence number and the last has
 * a sequence number of 0.
 *
 *----------------------------------------------------------------------------*/
void softswitch_trivial_log_handler(uint32_t src, const char* &logStr)
{
    uint32_t pktLen = strlen(logStr) + 1;   // Bytes to send
    uint32_t remLen = pktLen;               // Bytes remaining to be sent
    uint32_t pktCnt;                        // Number of log packets needed

    // Packet pointers
    volatile void* send_buf = tinselSendSlotExtra();
    P_Pkt_t* pkt = static_cast<P_Pkt_t*>(const_cast<void*>(send_buf));
    P_Pkt_Hdr_t* hdr = static_cast<P_Pkt_Hdr_t*>(const_cast<void*>(send_buf));
    P_Log_Pkt_Pyld_t* pyld = reinterpret_cast<P_Log_Pkt_Pyld_t*>(pkt->payload);


    // Work out how many packets we need to send.
    // This will be capped at P_LOG_MAX_LOGPKT_FRAG (e.g. 4)
    if(pktLen <= p_logpkt_1pkt_max_size)        pktCnt = 0; // 1 Packet
    else if(pktLen <= p_logpkt_2pkt_max_size)   pktCnt = 1; // 2 Packet
    else if(pktLen <= p_logpkt_3pkt_max_size)   pktCnt = 2; // 3 Packet
    else                                        pktCnt = 3; // Cap to 4 packets


    //--------------------------------------------------------------------------
    // Set the packet header
    //--------------------------------------------------------------------------
    while(!tinselCanSend());    // Don't touch the buffers until we can send
                                // This check is needed to make sure we don't
                                // corrupt a previously staged log packet that
                                // has not actually been sent.
    hdr->swAddr = P_SW_MOTHERSHIP_MASK | P_SW_CNC_MASK;
    hdr->swAddr |= ((P_CNC_LOG << P_SW_OPCODE_SHIFT) & P_SW_OPCODE_MASK);
    hdr->pinAddr = src;         // The index of the device on the supervisor
    //--------------------------------------------------------------------------

    for(int i = pktCnt; i >= 0; i--)
    {
        while(!tinselCanSend());    // Don't touch the buffers until we can send

        if(remLen > p_logpkt_1pkt_max_size)
        {
            // Not a short packet, put 55 bytes into the payload
            memcpy(pyld->payload, logStr, p_logpkt_pyld_size);

            if(i==0)
            {   //Last packet, but still bytes to send - Truncate logStr
                pyld->payload[p_logpkt_pyld_size-1] = '\0';
            }

            logStr += p_logpkt_pyld_size;      // Increment the logstr pointer.
            remLen -= p_logpkt_pyld_size;     // Decrement the remaining length

            tinselSetLen(TinselMaxFlitsPerMsg-1);   // Set send length
        }
        else
        {   // The last packet. Put all the remaining data into the payload.
            memcpy(pyld->payload, logStr, remLen);

            uint32_t len = p_logpkt_hdr_size + remLen;    // Calc packet length
            tinselSetLen((len - 1) >> TinselLogBytesPerFlit); // Set send length
        }
        pyld->seq = i;

        SUPER_SEND(send_buf); // Send it
    }
}

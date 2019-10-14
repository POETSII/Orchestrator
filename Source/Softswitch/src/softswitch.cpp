#include "softswitch.h"
#include "tinsel.h"

#include <cstring>  // needed for strlen and memcpy

/*------------------------------------------------------------------------------
 * A very trivial handler log sender that only sends the format string section
 * of a handler_log call. 
 *
 * The pinAddr of the message header is (ab)used for the source device's HW 
 * address.
 * 
 * Messages up to 220 characters (with the default tinsel config) are supported
 * through simplistic fragmentation. The first 8-bits of payload are used to
 * facilitate this by including a decrementing sequence number (e.g.
 * the first message fragment has the highest sequence number and the last has 
 * a sequence number of 0.
 *
 *----------------------------------------------------------------------------*/
void softswitch_trivial_log_handler(const char* &logStr)
{
    uint32_t msgLen = strlen(logStr) + 1;   // Bytes to send
    uint32_t remLen = msgLen;               // Bytes remaining to be sent
    uint32_t msgCnt;                        // Number of log messages needed
    
    // Message pointers
    volatile void* send_buf = tinselSlot(P_LOGMSG_SLOT);
    P_Msg_t* msg = static_cast<P_Msg_t*>(const_cast<void*>(send_buf));
    P_Msg_Hdr_t* hdr = static_cast<P_Msg_Hdr_t*>(const_cast<void*>(send_buf));
    P_Log_Msg_Pyld_t* pyld = reinterpret_cast<P_Log_Msg_Pyld_t*>(msg->payload);
    
    
    // Work out how many messages we need to send.
    // This will be capped at P_LOG_MAX_LOGMSG_FRAG (e.g. 4)
    if(msgLen <= p_logmsg_1msg_max_size)        msgCnt = 0; // 1 Message
    else if(msgLen <= p_logmsg_2msg_max_size)   msgCnt = 1; // 2 Messages
    else if(msgLen <= p_logmsg_3msg_max_size)   msgCnt = 2; // 3 Messages
    else                                        msgCnt = 3; // Cap to 4 messages
    
    
    //--------------------------------------------------------------------------
    // Set the message header
    //--------------------------------------------------------------------------
    while(!tinselCanSend());    // Don't touch the buffers until we can send
                                // This check is needed to make sure we don't 
                                // corrupt a previously staged log message that
                                // has not actually been sent.
    hdr->swAddr = P_SW_MOTHERSHIP_MASK | P_SW_CNC_MASK;
    hdr->swAddr |= ((P_CNC_LOG << P_SW_OPCODE_SHIFT) & P_SW_OPCODE_MASK);
    hdr->pinAddr = tinselId(); // usurp Pin Addr for the source HW addr
    //--------------------------------------------------------------------------
    
    for(int i = msgCnt; i >= 0; i--)
    {
        while(!tinselCanSend());    // Don't touch the buffers until we can send
        
        if(remLen > p_logmsg_1msg_max_size)
        {
            // Not a short message, put 55 bytes into the payload
            memcpy(pyld->payload, logStr, p_logmsg_pyld_size);
            
            if(i==0)
            {   //Last message, but still bytes to send - Truncate logStr
                pyld->payload[p_logmsg_pyld_size-1] = '\0';
            }
            
            logStr += p_logmsg_pyld_size;      // Increment the logstr pointer.
            remLen -= p_logmsg_pyld_size;     // Decrement the remaining length
            
            tinselSetLen(TinselMaxFlitsPerMsg-1);   // Set send length
        }
        else
        {   // The last message. Put all the remaining data into the payload.
            memcpy(pyld->payload, logStr, remLen);
            
            uint32_t len = p_logmsg_hdr_size + remLen;    // Calc message length
            tinselSetLen((len - 1) >> TinselLogBytesPerFlit); // Set send length
        }
        pyld->seq = i;
        
        tinselSend(tinselHostId(), send_buf); // Send it
    }
}


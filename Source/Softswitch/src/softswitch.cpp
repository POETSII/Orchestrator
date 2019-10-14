#include "softswitch.h"
#include "tinsel.h"
// we hope these includes don't break the memory limit...
#include <cstring>

void softswitch_trivial_log_handler(const char* &logStr)
{
    uint32_t msgLen = strlen(logStr) + 1;   // Bytes to send
    uint32_t remLen = msgLen;               // Bytes remaining to be sent
    uint32_t msgCnt;                        // Number of log messages needed
    
    // Message pointers
    volatile void* send_buf = tinselSlot(P_LOGMSG_SLOT);
    P_Msg_t* msg = static_cast<P_Msg_t*>(const_cast<void*>(send_buf));
    P_Log_Msg_Pyld_t* pyld = reinterpret_cast<P_Log_Msg_Pyld_t*>(msg->payload);
    
    
    // Work out how many messages we need to send.
    // This will be capped at P_LOG_MAX_LOGMSG_FRAG (e.g. 4)
    if(msgLen <= p_logmsg_1msg_max_size)        msgCnt = 1; // 1 Message
    else if(msgLen <= p_logmsg_2msg_max_size)   msgCnt = 2; // 2 Messages
    else if(msgLen <= p_logmsg_3msg_max_size)   msgCnt = 3; // 3 Messages
    else                                        msgCnt = 4; // Cap to 4 messages
    
    
    for(int i = msgCnt; i > 0; i--)
    {
        while(!tinselCanSend());    // Don't touch the buffers until we can send
        
        if(remLen > p_logmsg_1msg_max_size)
        {
            // Not a short message, put 55 bytes into the payload
            memcpy(pyld->payload, logStr, p_logmsg_pyld_size);
            if(i==1)
            {   //Last message, but still bytes to send - Truncate logStr
                pyld->payload[p_logmsg_pyld_size-1] = '\0';
            }
            
            remLen -= p_logmsg_pyld_size;     // Decrement the remaining length
            
            tinselSetLen(TinselMaxFlitsPerMsg-1);   // Set send length
        }
        else
        {   // The last message. Put all the remaining data into the payload.
            memcpy(pyld->payload, logStr, remLen);
            
            uint32_t len = p_logmsg_hdr_size + remLen;    // Calc message length
            tinselSetLen((len - 1) >> TinselLogBytesPerFlit); // Set send length
        }
        tinselSend(tinselHostId(), send_buf); // Send it
    }
}


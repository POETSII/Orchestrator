#include "softswitch.h"
#include "tinsel.h"
// we hope these includes don't break the memory limit...
#include <cstring>

handler_log_msg::handler_log_msg()
{
    msg_len = seq = 0;
    // get the supervisor message slots needed
    for (unsigned int slot = 0; slot < MAX_LOG_MSG_BUFS; slot++) log_msg[slot] = static_cast<P_Sup_Msg_t*>(const_cast<void*>(tinselSlot(1+NUM_SUP_BUFS+slot)));
}

uint32_t handler_log_msg::pack_fmtstr(const char* &msg)
{
    uint32_t r_len;
    
    r_len = strlen(msg) + 1;
     
#ifndef TRIVIAL_LOG_HANDLER
    if (r_len > (MAX_LOG_MSG_BUFS*p_sup_msg_size()-(MAX_LOG_MSG_BUFS*p_sup_hdr_size()))) // String too long?
    {
        memcpy(log_msg[0]->data, "Logging error: handler_log message too long\n", 45);
	    msg_len = 45 + p_sup_hdr_size();
	    return 1;
    }
#endif

    msg_len = r_len; 
    while ((seq < MAX_LOG_MSG_BUFS) && r_len) // copy the main format string first
    {
        msg_len += p_sup_hdr_size(); // each sequence number in the message has a header
        if (r_len > p_super_data_size)   // copy full-message length blocks
        {
            memcpy(log_msg[seq++]->data, msg, p_super_data_size);
            msg += p_super_data_size;
            r_len -= p_super_data_size;
        }
        else
        {
            memcpy(log_msg[seq]->data, msg, r_len);   // last block may be partial
            r_len = 0;
        }
    }
    return 0;
}

void handler_log_msg::send_msg()
{
#ifndef TRIVIAL_LOG_HANDLER
    if (seq >= MAX_LOG_MSG_BUFS) // format string fit, but arguments didn't.
    {
        seq = 0;
        memcpy(log_msg[0]->data, "Logging error: handler_log message too long with arguments\n", 60);
	    msg_len = 60 + p_sup_hdr_size();
    }
#endif
     
    // message has been extracted into the message buffers. Start setting up for the send.
    for (uint32_t s = 0; s <= seq; s++)
        set_super_hdr(tinselId(), P_SUP_MSG_LOG, P_SUP_PIN_SYS, msg_len, s, &log_msg[s]->header);
    if (tinselCanSend()) // use the fast send channel if we can
    {
	    for (uint32_t m = 0; m+1 <= seq; m++)
	    {
	        // most messages are a full maximum message size 
	        tinselSetLen(TinselMaxFlitsPerMsg-1);
	        tinselSend(tinselHostId(), log_msg[m]);
	        msg_len -= p_sup_msg_size();
	    }
	    // remaining message flit length is computed by shift-division
	    tinselSetLen((msg_len-1) >> (2+TinselLogWordsPerFlit));
	    tinselSend(tinselHostId(), log_msg[seq]);		
    }
    else // otherwise go through the slow UART channel
    {
	    for (uint32_t u = 0; u+1 <= seq; u++)
	    {
	        for (unsigned int v = 0; v < p_sup_msg_size(); v++) while (!tinselUartTryPut(*(static_cast<uint8_t*>(static_cast<void*>(log_msg[u]))+v)));
	        msg_len -= p_sup_msg_size();
	    }
	    for (unsigned int w = 0; w < msg_len; w++) while (!tinselUartTryPut(*(static_cast<uint8_t*>(static_cast<void*>(log_msg[seq]))+w)));
    }
}

#ifndef TRIVIAL_LOG_HANDLER
void __assert_func(const char* file, int line, const char* func, const char* failedexpr)
{
    // report the failure,
    handler_log(0,"assertion \"%s\" failed: file \"%s\", line %d%s%s\n", failedexpr, file, line, func ? ", function: " : "", func ? func : "");
    P_Sup_Msg_t abort_pkt;
    pack_super_msg(tinselId(), P_SUP_MSG_KILL, P_SUP_PIN_SYS, 0, 0, &abort_pkt);
    tinselSetLen(0);
    // then try to send an abort signal either the fast way (over the HostLink),
    if (tinselCanSend()) tinselSend(tinselHostId(),&abort_pkt);
    else
    {
        // or the slow way (over the DebugLink), to the host.
        // we don't ultimately care if this stalls or never finishes because
        // we are stopping anyway. But if it succeeds, the supervisor
        // might be able to shut everyone down.
        uint8_t* UartMsg = static_cast<uint8_t*>(static_cast<void*>(&abort_pkt)); 
        for (uint32_t byte = 0; byte < (1 << TinselLogBytesPerFlit); byte++)
	    {
	        while (!tinselUartTryPut(*UartMsg));
	        ++UartMsg;
	    }
    }
    while (1); // never return from this function.
}
#endif

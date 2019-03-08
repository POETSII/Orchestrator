#include "softswitch.h"

// argument expansion and buffer packing template function
template<typename V> void handler_log_msg::pack_log_arg(uint32_t arglen, V* arg)
{
    if ((msg_len + arglen) > ((seq+1)*sizeof(P_Sup_Msg_t))) // next argument overflows current buffer?
    {
        if (++seq >= MAX_LOG_MSG_BUFS) return; // abort packing if the maximum message length has been exceeded
        msg_len = seq*sizeof(P_Sup_Msg_t) + sizeof(P_Sup_Hdr_t); // otherwise adjust the length accordingly
    }   
    memcpy(&(log_msg[seq]->data[msg_len-(seq*sizeof(P_Sup_Msg_t))-sizeof(P_Sup_Hdr_t)]), arg, arglen); // actually pack the argument
    msg_len += arglen; // increment the length    
}  

// string specialisations
template<> inline void handler_log_msg::prepack_log_arg(const char* arg) {pack_log_arg<const char>(strlen(arg)+1, arg);};

// base function (non-class member)
template<typename... F> void handler_log(int level, const char * msg, F... args)
{
    handler_log_msg log_message;

    if (!log_message.pack_fmtstr(msg))                   // as long as the format string fits into a message,
    log_message.pack_log_args(args...);                  // insert arguments
    log_message.send_msg();                             // and send whatever results (possibly an error message)
}

#ifndef _SOFTSWITCH_H_
#define _SOFTSWITCH_H_ 

#include "poets_msg.h"

#define MAX_LOG_MSG_BUFS 4

#define P_DEFAULT_LOG_LEVEL     2

#ifndef P_LOG_LEVEL
#define P_LOG_LEVEL P_DEFAULT_LOG_LEVEL
#endif

//------------------------------------------------------------------------------
// Tinsel Slot Allocations.
// Slot 0 is used to send "normal" messages. Slot 1 is used for log messages.
// Remaining slots are used for receive slots.
//------------------------------------------------------------------------------
#define P_MSG_SLOT      0
#define P_LOGMSG_SLOT   2
#define P_RXSLOT_START  3
//------------------------------------------------------------------------------

const uint32_t p_logmsg_max_size = p_logmsg_pyld_size << P_LOG_MAX_LOGMSG_FRAG;
const uint32_t p_logmsg_1msg_max_size = p_logmsg_pyld_size;
const uint32_t p_logmsg_2msg_max_size = p_logmsg_pyld_size << 1;
const uint32_t p_logmsg_3msg_max_size = p_logmsg_2msg_max_size + p_logmsg_pyld_size;

void softswitch_trivial_log_handler(const char* &logStr);

#ifdef TRIVIAL_LOG_HANDLER
// Call a truly trivial log handler.
template<typename... F> inline void handler_log(int level, const char * msg, F... args) 
{
    if(level >= P_LOG_LEVEL) softswitch_trivial_log_handler(msg);
};

inline void assert(int expression) {return;};

#else
// Placeholder that does nothing.
template<typename... F> inline void handler_log(int level, const char * msg, F... args) 
{
    return;
};
inline void assert(int expression) {return;};

#endif

#endif //_SOFTSWITCH_H_

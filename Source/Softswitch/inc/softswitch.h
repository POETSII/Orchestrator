#ifndef _SOFTSWITCH_H_
#define _SOFTSWITCH_H_ 

#include "poets_pkt.h"

#define MAX_LOG_PKT_BUFS 4

#define P_DEFAULT_LOG_LEVEL     2

#ifndef P_LOG_LEVEL
#define P_LOG_LEVEL P_DEFAULT_LOG_LEVEL
#endif

const uint32_t p_logpkt_max_size = p_logpkt_pyld_size << P_LOG_MAX_LOGPKT_FRAG;
const uint32_t p_logpkt_1pkt_max_size = p_logpkt_pyld_size;
const uint32_t p_logpkt_2pkt_max_size = p_logpkt_pyld_size << 1;
const uint32_t p_logpkt_3pkt_max_size = p_logpkt_2pkt_max_size + p_logpkt_pyld_size;

void softswitch_trivial_log_handler(const char* &logStr);

#ifdef TRIVIAL_LOG_HANDLER
// Call a truly trivial log handler.
template<typename... F> inline void handler_log(int level, const char * pkt, F... args) 
{
    if(level >= P_LOG_LEVEL) softswitch_trivial_log_handler(pkt);
};

inline void assert(int expression) {return;};

#else
// Placeholder that does nothing.
template<typename... F> inline void handler_log(int level, const char * pkt, F... args) 
{
    return;
};
inline void assert(int expression) {return;};

#endif

#endif //_SOFTSWITCH_H_

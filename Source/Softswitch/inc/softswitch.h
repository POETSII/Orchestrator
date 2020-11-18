#ifndef _SOFTSWITCH_H_
#define _SOFTSWITCH_H_

#include "poets_pkt.h"

// Define operation in single-supervisor or multi-supervisor mode, where:
//
//  - Single-supervisor mode is where applications are serviced by one
//    supervisor device on the cluster host machine, regardless of where the
//    application is placed (even if it is placed across multiple boxes).
//
//  - Multi-supervisor mode is where applications are serviced by one
//    supervisor device for each box the application is placed on. Each normal
//    device is serviced by the supervisor device running on the same box as
//    the normal device.
//
// At time of writing (2020-11-18), multi-supervisor mode is not supported.
#ifndef SINGLE_SUPERVISOR_MODE
#define SINGLE_SUPERVISOR_MODE 1
#else
#define SINGLE_SUPERVISOR_MODE 0
#endif

#if SINGLE_SUPERVISOR_MODE
#define SUPER_SEND(PAYLOAD) tinselSend(tinselHostId(), PAYLOAD)
#else
#define SUPER_SEND(PAYLOAD) tinselSend(tinselMyBridgeId(), PAYLOAD)
#end

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

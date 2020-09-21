#ifndef _SOFTSWITCH_H_
#define _SOFTSWITCH_H_ 

#include <cstring>
#include "poets_msg.h"

#define NUM_SUP_BUFS 4
#define MAX_LOG_MSG_BUFS 4

class handler_log_msg
{
    public:
    handler_log_msg();
    uint32_t      pack_fmtstr(const char* &);
    void          send_msg();
      
#ifndef TRIVIAL_LOG_HANDLER
    // argument expansion and buffer packing
    template<typename... U> inline void pack_log_args(U&&... args) {uint32_t _a[] = {0, (prepack_log_arg(args),0)...};}
    // argument extractor
    template<typename P> inline void prepack_log_arg(P arg) {pack_log_arg<P>(sizeof(P),&arg);};
    // does the actual work
    template<typename V> void pack_log_arg(uint32_t, V*);
#endif
     
    P_Sup_Msg_t*  log_msg[MAX_LOG_MSG_BUFS];   // buffer for sequenced parts of the log message
    uint32_t      msg_len;                     // running message length
    uint32_t      seq;                         // and running sequence count
};

#ifdef TRIVIAL_LOG_HANDLER
//inline void handler_log(int level, const char * msg, ...) {handler_log_msg message; if (!message.pack_fmtstr(msg)) message.send_msg();};
template<typename... F> inline void handler_log(int level, const char * msg, F... args)  {handler_log_msg message; if (!message.pack_fmtstr(msg)) message.send_msg();};
inline void assert(int expression) {return;};

#else

#include <assert.h>

template<typename... F> void handler_log(int, const char *, F... );

// implementation of log function templates
#include "handler_log_t.h"

#endif

#endif //_SOFTSWITCH_H_

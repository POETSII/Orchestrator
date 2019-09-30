#include "poets_msg.h"
#include <cstring>

// bodgey declaration as a short-term hack to get around the problem of
// this TU not linking when building non-Tinsel targets. Proper fix
// is to separate tinsel.h into tinsel.h and tinsel.cpp.
extern int tinselId();

unsigned set_msg_hdr(uint8_t ms, uint8_t cnc, uint8_t task, uint8_t opcode, 
                    uint32_t dev, uint8_t pin, uint32_t edge, 
                    uint8_t len, P_Msg_Hdr_t* hdr)
{
    if (len > p_msg_pyld_size) return 1;            // die if the message is too big
    
    hdr->swAddr = ((ms << P_SW_MOTHERSHIP_SHIFT) & P_SW_MOTHERSHIP_MASK);
    hdr->swAddr |= ((cnc << P_SW_CNC_SHIFT) & P_SW_CNC_MASK);
    hdr->swAddr |= ((task << P_SW_TASK_SHIFT) & P_SW_TASK_MASK);
    hdr->swAddr |= ((opcode << P_SW_OPCODE_SHIFT) & P_SW_OPCODE_MASK);
    hdr->swAddr |= ((dev << P_SW_DEVICE_SHIFT) & P_SW_DEVICE_MASK);
    
    hdr->pinAddr = ((pin << P_HD_TGTPIN_SHIFT) & P_HD_TGTPIN_MASK);
    hdr->pinAddr |= ((pin << P_HD_DESTEDGEINDEX_SHIFT) & P_HD_DESTEDGEINDEX_MASK);

    return 0;
}


unsigned pack_msg(uint8_t ms, uint8_t cnc, uint8_t task, uint8_t opcode, 
                    uint32_t dev, uint8_t pin, uint32_t edge, 
                    uint8_t len, void* pyld, P_Msg_Hdr_t* hdr)
{
    if(set_msg_hdr(ms, cnc, task, opcode, dev, pin, edge, len, &msg->header))
    {   // pack up the header
        return 1;
    }        
    
    memcpy(msg->payload, pyld, len);                     // and the payload 
}


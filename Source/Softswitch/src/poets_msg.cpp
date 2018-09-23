#include "poets_msg.h"
#include <cstring>

// bodgey declaration as a short-term hack to get around the problem of
// this TU not linking when building non-Tinsel targets. Proper fix
// is to separate tinsel.h into tinsel.h and tinsel.cpp.
extern int tinselId();

void set_msg_hdr(uint32_t dst, uint32_t edge, uint8_t pin, uint8_t len, uint16_t tag, P_Msg_Hdr_t* hdr)
{
        if (len > p_msg_pyld_size) return;            // die if the message is too big
	hdr->destDeviceAddr = dst;                    // pack all the header information
	hdr->destEdgeIndex = edge;
	hdr->destPin = pin;
	hdr->messageLenBytes = len+sizeof(P_Msg_Hdr_t);
	hdr->messageTag = tag;
}

void pack_msg(uint32_t dst, uint32_t pin, uint32_t edge, uint8_t len, uint16_t tag, void* pyld, P_Msg_t* msg)
{
	set_msg_hdr(dst, pin, edge, len, tag, &msg->header); // pack up the header
	memcpy(msg->payload, pyld, len);                     // and the payload 
}

void set_super_hdr(uint32_t src, uint16_t command, uint16_t pin, uint32_t len, uint32_t seq, P_Sup_Hdr_t* hdr)
{
        hdr->sourceDeviceAddr = src | P_SUP_MASK;            // messages to Supervisor have a flag to indicate this fact. 
        hdr->command = command;
        hdr->destPin = pin;
        hdr->cmdLenBytes = len;
	hdr->seq = seq;
}

uint32_t pack_super_msg(uint32_t src, uint16_t command, uint16_t pin, uint32_t len, void* data, P_Sup_Msg_t* msg_q)
{
     uint32_t num_msgs = 1;
     uint32_t l = 0;
     uint8_t* part_data = static_cast<uint8_t*>(data);   
     while ((l += p_super_data_size) < len) num_msgs++;
     l = p_super_data_size + len - l; 
     set_super_hdr(src, command, pin, len+(sizeof(P_Sup_Hdr_t)*num_msgs), num_msgs-1, &msg_q[num_msgs-1].header);
     memcpy(msg_q[num_msgs-1].data, static_cast<uint8_t*>(data)+((num_msgs-1)*p_super_data_size), l);
     for (unsigned int m = 0; m < num_msgs-1; m++)
     {
         msg_q[m].header = msg_q[num_msgs-1].header;
	 msg_q[m].header.seq = m;
	 memcpy(msg_q[m].data, part_data, p_super_data_size);
     }
     return num_msgs;
}

void super_buf_clr(P_Sup_Msg_t* msg)
{
// Clear a full supervisor buffer. A supervisor buffer is an array of supervisor messages.
// If the buffer isn't full (cmdLenBytes in the first seq is 0), does nothing.
     uint32_t last_seq = msg->header.cmdLenBytes/sizeof(P_Sup_Msg_t) + (msg->header.cmdLenBytes%sizeof(P_Sup_Msg_t) ? 1 : 0);
     for (uint32_t seq = 0; seq < last_seq; seq++)
         msg[seq].header.seq = 0;
     msg->header.cmdLenBytes = 0;
}
   
bool super_buf_recvd(const P_Sup_Msg_t* msg)
{
// simple way to detect if an entire supervisor buffer has been received: see if all the
// sequence numbers have been filled.
     uint32_t len = msg->header.cmdLenBytes;
     if (len == 0) return false; // no length in seq 0 => seq 0 not received.
     while (len > sizeof(P_Sup_Msg_t))
     {
        len -= sizeof(P_Sup_Msg_t);
        if ((++msg)->header.seq == 0) return false; // any zero seq anywhere else => not done.
     }
     return true;
}

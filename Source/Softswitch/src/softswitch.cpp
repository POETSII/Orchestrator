#include "softswitch.h"
#include "tinsel.h"
// we hope these includes don't break the memory limit...
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <cstdarg>

int parse_log_msg(const char* msg, unsigned char* numargs, unsigned char* arglens)
{
  // parser has a state table to determine where it's at in the (format) string
  // states are:
  const unsigned char state_transition_tbl[12][19] = {{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                      { 0, 2, 2, 3, 5, 2, 4, 7, 8, 9,10, 0, 0, 0, 0, 0, 0, 0,11},
                                                      {11,11,11, 3, 5, 4, 4, 7, 8, 9,10, 0, 0, 0, 0, 0, 0, 0,11},
                                                      {11,11,11,11, 5,11,11, 7, 8, 9,10, 0, 0, 0, 0, 0, 0, 0,11},
                                                      {11,11,11,11, 5, 4, 4, 7, 8, 9,10, 0, 0, 0, 0, 0, 0, 0,11},
                                                      {11,11,11, 6,11, 5, 5, 7, 8, 9,10, 0, 0, 0, 0, 0, 0, 0,11},
                                                      {11,11,11,11,11,11,11, 7, 8, 9,10, 0, 0, 0, 0, 0, 0, 0,11},
                                                      {11,11,11,11,11,11,11, 9,11,11,11, 0, 0,11,11,11,11, 0,11},
                                                      {11,11,11,11,11,11,11,11, 9,11,11, 0, 0,11, 0, 0,11, 0,11},
                                                      {11,11,11,11,11,11,11,11,11,11,11, 0, 0,11,11,11,11, 0,11},
                                                      {11,11,11,11,11,11,11,11,11,11,11,11,11, 0,11,11,11,11,11},
                                                      {11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11}};
  unsigned char state = 0;
  unsigned int msgLen = 1; // length includes a terminating null-character in the format string
  unsigned char token;
  *numargs = 0;
  // suck up the characters one at a time. 
  while (*msg != '\0')
  {
        ++msgLen;
	
        /* a multipart if is used instead of the probably-more-efficient multimap
           because we don't want to include the multimap header and all its associated
           memory footprint. Cases are ordered roughly by probability, so that at
           least the common cases will be reasonably fast.
        */
	switch (*msg)
	{
	case '%' :
	token = 0;
	break;
	case 'd':
	case 'i':
	token = 11;
	break;
	case 's':
	token = 15;
	break;
	case 'f':
	case 'F':
	case 'e':
	case 'E':
	case 'g':
	case 'G':
	case 'a':
	case 'A':
	token = 13;
	break;
	case 'u':
	case 'o':
	case 'x':
	case 'X':
	token = 12;
	break;
	case 'c':
	token = 14;
	break;
	case 'p':
	token = 16;
	break;
	case 'n':
	token = 17;
	break;
	case ' ':
	token = 1;
	break;
	case '+':
	case '-':
	case '#':
	token = 2;
	break;
	case '*':
	token = 3;
	break;
        case '.':
	token = 4;
	break;
	case '0':
	token = 5;
	break;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	token = 6;
	break;
	case 'h':
	token = 7;
	break;
	case 'l':
	token = 8;
	break;
	case 'j':
	case 'z':
	case 't':
	token = 9;
	break;
	case 'L':
	token = 10;
	break;
	default: token = 18;
	}
        if ((token != 0) && (token != 18))
	{
	switch (state)
	{
	case 1:
	{
	++(*numargs);
	if (token == 9)
	{
	   *arglens++ = 4;
	   break;
	}
	if (token == 10)
	{
	   *arglens++ = 10;
	   break;
	}
	}
	case 2:
	case 5:
	if (token == 3) ++(*numargs);
	case 3:
	case 4:
	case 6:
	{
	if ((token == 1) || (token == 2) || (token > 3 && token < 11)) break;
	if (token == 13)
	{
	   *arglens++ = 8;
	   break;
	}
	if (token == 15)
	{
	   *arglens++ = -1; // char* and its derivatives are coded as a negative value to indicate pointer derivation
	   break;
	}
	*arglens++ = 4;
	break;
        }
	case 7:
	{
        if (token == 7) *arglens++ = 1;
	else if ((token == 11) || (token == 12)) *arglens++ = 2;
        else if (token == 16) *arglens++ = 4;
	break;
	}
	case 8:
	{
	if (token == 8) *arglens++ = -8;       // long long is coded as a negative value to distinguish from double. 
	else if (token == 15) *arglens++ = -2; // wchar_t* is coded as a negative value to ensure its actual size will be computed
	}
	case 9:
	{
	if ((token == 11) || (token == 12) || (token == 17)) *arglens++ = 4;
	break;
	}
	case 10:
	if (token == 13) *arglens++ = 10;
	break;
	case 11:
	return -1;
	}
	}
	state = state_transition_tbl[state][token];
	++msg;
  }
  return msgLen;
}

void handler_log(int level, const char * msg, ...)
{
     P_Sup_Msg_t*  log_msg[4];   // arbitrarily we will support 256-byte (192-character) max messages.
     unsigned char arg_lens[(4*(sizeof(P_Sup_Msg_t)-sizeof(P_Sup_Hdr_t)))/5]; // max possible number of args is ((5*num_args) == 191) = 38
     unsigned char num_args;
     int           r_len;
     int           msg_len;
     int           arg_len;
     void*         p_arg;
     int           seq = 0;

     va_list       va_args;

     // get the supervisor message slots needed
     for (unsigned int slot = 1; slot < 5; slot++) log_msg[slot] = static_cast<P_Sup_Msg_t*>(const_cast<void*>(tinselSlot(slot)));
     msg_len = arg_len = parse_log_msg(msg, &num_args, arg_lens);
     
     if (arg_len < 0) // zero-length return means the string or its arguments were invalid in some way.
     {
        msg_len = 44;
        memcpy(log_msg[0]->data, "Logging error: invalid handler_log message\n", msg_len);
     }
     else
     {
       va_start(va_args, msg); // as long as everything is valid we can start pulling out arguments.
    
        r_len = p_super_data_size; // keep track of where we are in the current buffer
        while ((seq < 4) && arg_len) // copy the main format string first
        {
	  if (arg_len > p_super_data_size) // copy full-message length blocks
	      {
	         memcpy(log_msg[seq]->data, msg, p_super_data_size);
	         msg += p_super_data_size;
	         arg_len -= p_super_data_size;
	         if (++seq > 3)  // string already too long. Abort parsing arguments
		 {
		    num_args = 0;
		    break;
		 }
	      }
	      else
	      {
		memcpy(log_msg[seq]->data, msg, arg_len); // last block may be partial,
		r_len -= arg_len;                         // so adjust the remaining length
	         arg_len = 0;
	      }
        }
        for (unsigned int arg = 0; arg < num_args; arg++) // now go through arguments
        {
	    // preparse means we can pull out arguments based on type.
	    // string arguments are fully copied into the buffer using the same tactics
	    // as the main string
	    if (arg_lens[arg] == -1) // an ordinary char* string
	    {
	       char* str_arg = va_arg(va_args, char*);
	       arg_len = strlen(str_arg) + 1;
	       msg_len += arg_len;
	       memcpy(log_msg[seq]->data+(p_super_data_size-r_len), str_arg, arg_len > r_len ? r_len : arg_len);
	       if ((arg_len -= r_len) > 0)
	       {
	          str_arg += r_len;
	          ++seq;
	       }
	       while ((seq < 4) && (arg_len > 0))
	       {
		  memcpy(log_msg[seq]->data, str_arg, arg_len > p_super_data_size ? p_super_data_size : arg_len);
	          if ((arg_len -= p_super_data_size) > 0)
	          {
		     str_arg += p_super_data_size;
		     ++seq;
	          }
	          else r_len = p_super_data_size-arg_len;
	       }
	    }
	    else if (arg_lens[arg] == -2)  // a wchar_t string. Probably an exotic case.
	    {
	       wchar_t* wstr_arg = va_arg(va_args, wchar_t*);
	       arg_len = (wcslen(wstr_arg) + 1)*sizeof(wchar_t);
	       int w_len = r_len/sizeof(wchar_t); // we need to compute the number of wide characters
	       int s_len = w_len*sizeof(wchar_t); // that can fit into the spare buffer space
	       memcpy(log_msg[seq]->data+(p_super_data_size-r_len), wstr_arg, arg_len > s_len ? s_len : arg_len);
	       if ((arg_len -= s_len) > 0)
	       {
	          wstr_arg += w_len;
		  msg_len += s_len;
	          ++seq;
	       }
	       else msg_len += arg_len;
	       while ((seq < 4) && (arg_len > 0))
	       {
	          w_len = p_super_data_size/sizeof(wchar_t);
	          s_len = w_len*sizeof(wchar_t);
	          memcpy(log_msg[seq]->data, wstr_arg, arg_len > s_len ? s_len : arg_len);
	          if ((arg_len -= s_len) > 0)
	          {
		     wstr_arg += w_len;
		     msg_len += s_len;
		     ++seq;
	          }
	          else
		  {
		     r_len = p_super_data_size-arg_len;
		     msg_len += arg_len;
		  }
	       }
	    }
	    else // nonstrings are easier to handle - just cast to type and copy
	    {
	       switch (arg_lens[arg])
	       {
	       case -8: // long long integer
	       {
	       uint64_t ll_arg = va_arg(va_args, uint64_t);
	       p_arg = static_cast<void*>(&ll_arg);
	       arg_len = sizeof(uint64_t);
	       break;
	       }
	       case 1:  // char and its derivatives
	       {
	       uint8_t c_arg = static_cast<uint8_t>(va_arg(va_args, uint32_t));
	       p_arg = static_cast<void*>(&c_arg);
	       arg_len = sizeof(uint8_t);
	       break;
	       }
	       case 2:  // short and its derivatives
	       {
	       uint16_t s_arg = static_cast<uint16_t>(va_arg(va_args, uint32_t));
	       p_arg = static_cast<void*>(&s_arg);
	       arg_len = sizeof(uint16_t);
	       break;
	       }
	       case 8: // double
	       {
	       double d_arg = va_arg(va_args, double);
	       p_arg = static_cast<void*>(&d_arg);
	       arg_len = sizeof(double);
	       break;
	       }
	       case 10: // long double
	       {
	       long double ld_arg = va_arg(va_args, long double);
	       p_arg = static_cast<void*>(&ld_arg);
	       arg_len = sizeof(long double);
	       break;
	       }
	       default: // everything else can be treated as an int
	       {
	       uint32_t l_arg = va_arg(va_args, uint32_t);
	       p_arg = static_cast<void*>(&l_arg);
	       arg_len = sizeof(uint32_t);
	       }
	       }
	       if (arg_len > r_len) // yet again, does it fit into the spare space?
	       {
		 if (++seq < 3) // if not, go to the next available buffer
	          {
	             memcpy(log_msg[seq]->data, p_arg, arg_len);
		     msg_len += (arg_len + r_len);
	             r_len = p_super_data_size-arg_len;
	          }
	       }
	       else
	       {
	          memcpy(log_msg[seq]->data+(p_super_data_size-r_len), p_arg, arg_len);
		  msg_len += arg_len;
	          r_len -= arg_len;
	       }
	    }
	    if (seq > 3) break;
        }  
        if (seq > 3) // base string fit, but not after argument expansion.
        {
	   msg_len = 45;
	   seq = 0;
           memcpy(log_msg[0]->data, "Logging error: handler_log message too big\n", msg_len);
        }
	va_end(va_args); // done extracting.
     }
     // message has (at last!) been parsed and extracted into the message buffers. Start setting up
     // for the send.
     for (int s = 0; s <= seq; s++)
         set_super_hdr(tinselId(), P_SUP_MSG_LOG, P_SUP_PIN_SYS, msg_len, s, &log_msg[s]->header);
     if (tinselCanSend()) // use the fast send channel if we can
     {
	for (int m = 0; m <= seq-1; m++)
	{
	    // most messages are a full maximum message size 
	    tinselSetLen(TinselMaxFlitsPerMsg-1);
	    tinselSend(tinselHostId(), log_msg[m]);
	    msg_len -= sizeof(P_Sup_Msg_t);
	}
	// remaining message flit length is computed by shift-division and mask-modulus. Since the tinselSetLen
	// command uses a value of (flits-1) the modulo-piece (is one more flit needed?) can selectively subtract
	// 1 when not needed rather than adding 1 when needed.
	tinselSetLen((msg_len >> (2+TinselLogWordsPerFlit)) - ((msg_len & ((1 << (2+TinselLogWordsPerFlit))-1)) ? 0 : 1));
	tinselSend(tinselHostId(), log_msg[seq]);		
     }
     else // otherwise go through the slow UART channel
     {
	for (int u = 0; u <= seq-1; u++)
	{
	    for (unsigned int v = 0; v < sizeof(P_Sup_Msg_t); v++) while (!tinselUartTryPut(*(static_cast<uint8_t*>(static_cast<void*>(log_msg[u]))+v)));
	    msg_len -= sizeof(P_Sup_Msg_t);
	}
	for (unsigned int w = 0; w < msg_len; w++) while (!tinselUartTryPut(*(static_cast<uint8_t*>(static_cast<void*>(log_msg[seq]))+w)));
     }
}

void __assert_func(const char* file, int line, const char* func, const char* failedexpr)
{
     // report the failure,
     handler_log(0,"assertion \"%s\" failed: file \"%s\", line %d%s%s\n", failedexpr, file, line, func ? ", function: " : "", func ? func : "");
     P_Sup_Msg_t abort_pkt;
     pack_super_msg(tinselId(), P_SUP_MSG_KILL, P_SUP_PIN_SYS, 0, 0, &abort_pkt);
     tinselSetLen(0);
     // then try to send a message either the fast way (over the HostLink),
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

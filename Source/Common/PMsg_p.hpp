#ifndef __PMsg_p__HPP
#define __PMsg_p__HPP

#include "mpi.h"
#include "Msg_p.hpp"

//==============================================================================

class PMsg_p : public Msg_p {
public :
              PMsg_p();
              PMsg_p(byte *,int);
              PMsg_p(PMsg_p &);
virtual ~     PMsg_p(void);
void          Bcast();
void          Send();
void          Send(int);
};

//==============================================================================

#endif


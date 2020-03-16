#ifndef __PMsg_p__HPP
#define __PMsg_p__HPP

#include "mpi.h"
#include "Msg_p.hpp"

//==============================================================================

class PMsg_p : public Msg_p {
public :
              PMsg_p(MPI_Comm c=MPI_COMM_NULL);
              PMsg_p(byte * pb,int l,MPI_Comm c=MPI_COMM_NULL);
              PMsg_p(PMsg_p & r);
              PMsg_p(const PMsg_p & r);
virtual ~     PMsg_p(void);
void          Bcast();
void          Send();
void          Send(int);

MPI_Comm      comm;
};

//==============================================================================

#endif


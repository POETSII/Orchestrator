#include "PMsg_p.hpp"

//==============================================================================

/* This is the MPI-aware class derived from Msg_p (which is MPI-agnostic).
It's not my finest. Ultimately we should be able to send to sets of ranks, and
lots of other fiddly stuff, but at the moment I'm just shoving functions in as
and when I come across a need.
*/

//==============================================================================

PMsg_p::PMsg_p():Msg_p(){}
PMsg_p::PMsg_p(byte * pb,int l):Msg_p(pb,l){}
PMsg_p::PMsg_p(PMsg_p & r):Msg_p(r){}
PMsg_p::~PMsg_p(void){}

//------------------------------------------------------------------------------

void PMsg_p::Send()
{
Send(Tgt());
}

//------------------------------------------------------------------------------

void PMsg_p::Bcast()
// I don't *know* that MPI_Bcast doesn't work.
// BUT I *do* know that this does.
{
int flag;
MPI_Finalized(&flag);
if (flag) return;                      // MPI closed down already?
int Usize,Urank;
MPI_Comm_size(MPI_COMM_WORLD,&Usize);  // MPI Universe size
MPI_Comm_rank(MPI_COMM_WORLD,&Urank);  // My place within it
Src(Urank);
byte * bs = Stream();                  // Turn the message into a unified stream
unsigned len = Length();               // Length of stream
for (int i=0;i<Usize;i++) {            // Send a copy everywhere
  if (i==Urank) continue;              // ...except to myself
  Tgt(i);                              // Load target rank
  Ztime(0,MPI_Wtime());                // Timestamp departure
  MPI_Request request;
  MPI_Ibsend(bs,len,MPI_CHAR,i,tag,MPI_COMM_WORLD,&request);
  MPI_Status status;
  do MPI_Test(&request,&flag,&status);
  while (flag==0);
}
}

//------------------------------------------------------------------------------

void PMsg_p::Send(int dest)
{
int flag;
if (MPI_Finalized(&flag)!=0) return;          // MPI closed down already?

// TODO: Add some proper out-of-range rank detection. Old version had:
// if (dest < 0 || dest >= Usize) return;        // Invalid rank?
// but this should probably be a throw.

Tgt(dest);                                    // In case it's not there
Ztime(0,MPI_Wtime());                         // Timestamp departure
MPI_Request request;
MPI_Status status;
MPI_Ibsend(Stream(),Length(),MPI_CHAR,dest,tag,MPI_COMM_WORLD,&request);
int i=0;
do { i++; MPI_Test(&request,&flag,&status); } while (flag==0);
}

//==============================================================================


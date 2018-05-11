//------------------------------------------------------------------------------

#include "RTCL.h"
#include "CommonBase.h"
#include "pthread.h"
#include <stdio.h>
#include "PMsg_p.hpp"
#include "Pglobals.h"
#include "Cli.h"

//------------------------------------------------------------------------------

void * rtcl_func(void * args)
// Thread function spin on the MPI RTC, bleating every so often.
// DO NOT post messages from this thread, because you cannot rely on anything
// being alive at the end to get them, and MPI will block.
{
RTCL::comms_t * pC = static_cast<RTCL::comms_t *>(args);
double t_ = MPI_Wtime();
double t;
for(;;) {
  if (pC->l_kill) break;
  if (pC->l_stop) continue;
  t = MPI_Wtime();
  if (MPI_Wtime() <= t_ + pC->tick) continue;
  t_ = t;
  if (t > pC->t_stop) {
    pC->l_stop = true;
    continue;
  }
  printf("TICK\n");
  fflush(stdout);
}
pthread_exit(NULL);
return NULL;
}

//==============================================================================

RTCL::RTCL(int argc,char * argv[],string d):
  CommonBase(argc,argv,d,string(__FILE__))
{
FnMapx.push_back(new FnMap_t);    // create a new event map in the derived class

                                       // Load the message map
(*FnMapx[0])[PMsg_p::KEY(Q::RTCL,Q::N000,Q::N000,Q::N000)] = &RTCL::OnRTCL;
(*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::EXIT,Q::N000,Q::N000)] = &RTCL::OnExit;

comms.pthis  = this;
comms.tick   = 1.0;
comms.l_stop = true;
comms.t_stop = 10.0;
comms.l_kill = false;
void * args  = & comms;

pthread_t rtcl_thread;
if(pthread_create(&rtcl_thread,NULL,rtcl_func,args))
  fprintf(stdout,"Error creating rtcl_thread\n");

//printf("Back in main thread\n"); fflush(stdout);

MPISpinner();                          // Spin on *all* messages: exit on DIE
comms.l_kill = true;                   // Kill the clock thread
                                       // pthread_cancel() doesn't seem to work?
printf("********* RTC rank %d on the way out\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

RTCL::~RTCL()
{
WALKVECTOR(FnMap_t*, FnMapx, F)
    delete *F;

//printf("********* RTC rank %d destructor\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

void RTCL::Dump(FILE * fp)
{
fprintf(fp,"RTCL dump++++++++++++++++++++++++++++++++++\n");  fflush(stdout);
unsigned cIdx = 0;
WALKVECTOR(FnMap_t*,FnMapx,F)
{
fprintf(fp,"Function table for comm %d:\n", cIdx++);
fprintf(fp,"Key        Method\n");
WALKMAP(unsigned,pMeth,(**F),i)
  fprintf(fp,"%#010x %#010p\n",(*i).first,(*i).second);
}
fprintf(fp,"Communication pool:\n");
fprintf(fp,"pthis   : %p\n",comms.pthis);
fprintf(fp,"tick    : %e\n",comms.tick);
fprintf(fp,"l_stop  : %c\n",comms.l_stop ? 'T' : 'F');
fprintf(fp,"t_stop  : %e\n",comms.t_stop);
fprintf(fp,"l_kill  : %c\n",comms.l_kill ? 'T' : 'F');
fprintf(fp,"d_stop  : %e\n",comms.d_stop);
fprintf(fp,"d_start : %e\n",comms.t_start);
fprintf(fp,"RTCL dump----------------------------------\n");  fflush(stdout);
CommonBase::Dump(fp);
}

//------------------------------------------------------------------------------

unsigned RTCL::OnExit(PMsg_p * Z, unsigned cIdx)
// Exit command received from Root: we need to close down the RTC thread before
// the RTC process itself
{
comms.l_kill = true;                   // Set the "die" flag in the comms pool
return CommonBase::OnExit(Z,cIdx);          // Drop to the base class exit handler
}

//------------------------------------------------------------------------------

unsigned RTCL::OnRTCL(PMsg_p * Z, unsigned cIdx)
// RTCL control
// RTCL|   -|   -|   -|(1:string)Originating command line string
// The monkey command line has passed lex and syntax, but possibly not semantics
// These must be fixed with defaults because there's no simple way back to the
// console from here
{
string str;
Z->Get(1,str);
Cli Cl(str);
Cl.Trim();
if (Cl.Co!="rtcl") Post(901,"RTCL::OnRTCL",Cl.Co);
WALKVECTOR(Cli::Cl_t,Cl.Cl_v,i) {
  string sCl = (*i).Cl;
  if (sCl=="tick") {                   // Not 1 value for tick ?
    if ((*i).Pa_v.size()!=1) if (Post(51,Cl.Co,(*i).Cl))continue;
    comms.tick = str2dble((*i).Pa_v[0].Val,1.0);// Ignore any sign, default to 1.0
    Post(52,Cl.Co,(*i).Cl,dbl2str(comms.tick));
  }
  if (sCl=="stop") {                   // Not 1 value for stop
    if ((*i).Pa_v.size()!=1) if (Post(51,Cl.Co,(*i).Cl)) continue;
    comms.d_stop = str2dble((*i).Pa_v[0].Val,10.0);
    Post(52,Cl.Co,(*i).Cl,dbl2str(comms.d_stop));
  }
  if (sCl=="star") {                   // Not 0 values for start
    if ((*i).Pa_v.size()!=0) if (Post(54,Cl.Co,(*i).Cl)) continue;
    comms.t_stop = comms.d_stop + MPI_Wtime();
    comms.t_start = MPI_Wtime();
    comms.l_stop = false;
    Post(53,Cl.Co,(*i).Cl);
  }
  if (sCl=="paus") {                   // Not 0 values for pause
    if ((*i).Pa_v.size()!=0) if (Post(54,Cl.Co,(*i).Cl)) continue;
    comms.l_stop = true;
    Post(53,Cl.Co,(*i).Cl);
  }
  if (sCl=="kill") {                   // Not 0 values for start
    if ((*i).Pa_v.size()!=0) if (Post(51,Cl.Co,(*i).Cl)) continue;
    comms.l_kill = true;
    Post(53,Cl.Co,(*i).Cl);
  }
  if (sCl=="dump") Dump();             // Ignore parameters for now
}
return 0;
}

//==============================================================================




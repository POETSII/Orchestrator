//------------------------------------------------------------------------------

#include "RTCL.h"
#include "CommonBase.h"
#include "pthread.h"
#include <stdio.h>
#include "PMsg_p.hpp"
#include "Pglobals.h"
#include "Cli.h"

#include "OSFixes.hpp"

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
                                       // Load the message map
FnMap[PMsg_p::KEY(Q::RTCL,Q::N000,Q::N000,Q::N000)] = &RTCL::OnRTCL;
FnMap[PMsg_p::KEY(Q::CMND,Q::EXIT,Q::N000,Q::N000)] = &RTCL::OnExit;

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
                                       // possibly because pthread_setcanceltype()
                                       // defaults to deferred, which waits until
                                       // the thread reaches a cancellation point,
                                       // and both printf and fflush MAY be
                                       // cancellation points but are not required
                                       // to be. Setting the cancel type to asynchronous
                                       // could avoid this, with possibly nasty
                                       // side effects.
}

//------------------------------------------------------------------------------

RTCL::~RTCL()
{

//printf("********* RTC rank %d destructor\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

void RTCL::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sRTCL dump ++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sKey        Method\n",os);
WALKMAP(unsigned,pMeth,FnMap,i)
{
  fprintf(fp,"%s%#010x %" PTR_FMT "\n",os,(*i).first,
                        OSFixes::getAddrAsUint((*i).second));
}
fprintf(fp,"%sCommunication pool:\n",os);
fprintf(fp,"%spthis   : %p\n",os,static_cast<void*>(comms.pthis));
fprintf(fp,"%stick    : %e\n",os,comms.tick);
fprintf(fp,"%sl_stop  : %c\n",os,comms.l_stop ? 'T' : 'F');
fprintf(fp,"%st_stop  : %e\n",os,comms.t_stop);
fprintf(fp,"%sl_kill  : %c\n",os,comms.l_kill ? 'T' : 'F');
fprintf(fp,"%sd_stop  : %e\n",os,comms.d_stop);
fprintf(fp,"%sd_start : %e\n",os,comms.t_start);
CommonBase::Dump(off+2,fp);
fprintf(fp,"%sRTCL dump ------------------------------------------------\n",os);
fflush(stdout);
}

//------------------------------------------------------------------------------

unsigned RTCL::OnExit(PMsg_p * Z)
// Exit command received from Root: we need to close down the RTC thread before
// the RTC process itself
{
comms.l_kill = true;                   // Set the "die" flag in the comms pool
return CommonBase::OnExit(Z);          // Drop to the base class exit handler
}

//------------------------------------------------------------------------------

unsigned RTCL::OnRTCL(PMsg_p * Z)
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
    comms.tick = str2dble((*i).Pa_v[0].Va_v[0],1.0);// Ignore any sign, default to 1.0
    Post(52,Cl.Co,(*i).Cl,dbl2str(comms.tick));
  }
  if (sCl=="stop") {                   // Not 1 value for stop
    if ((*i).Pa_v.size()!=1) if (Post(51,Cl.Co,(*i).Cl)) continue;
    comms.d_stop = str2dble((*i).Pa_v[0].Va_v[0],10.0);
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




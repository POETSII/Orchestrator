//------------------------------------------------------------------------------

#include "Dummy.h"
#include "CommonBase.h"
#include "PMsg_p.hpp"
#include "mpi.h"
#include "Pglobals.h"
#include <stdio.h>

//==============================================================================

Dummy::Dummy(int argc,char * argv[],string d):
  CommonBase(argc,argv,d,string(__FILE__))
{
FnMapx.push_back(new FnMap_t);  // create a new function table in the derived class
                                       // Load the message map
//FnMapx[Msg_p::KEY(Msg_p::PMAP ,Msg_p::N000,Msg_p::N000,Msg_p::N000)] = &Dummy::OnPmap;
                                       // Spin on incoming MPI messages

PMsg_p Pkt;                            // Send out a bunch of random packets
Pkt.Src(Urank);
Pkt.comm = Comms[0];
char buf[512] = "Ooogle";
for(int i=0;i<10;i++) {
  int len = strlen(buf)+1;             // Include the terminal '\0'
  Pkt.Put<char>(1,buf,len);
  Pkt.Key(Q::TEST);
  Pkt.Send(Q::ROOT); // dangerous: sending to a literal rank before procmap is loaded
  buf[0]++;
}

MPISpinner();

//printf("********* Dummy1 rank %d on the way out\n",Urank); fflush(stdout);

}

//------------------------------------------------------------------------------

Dummy::~Dummy()
{
//printf("********* Dummy1 rank %d destructor\n",Urank); fflush(stdout);
WALKVECTOR(FnMap_t*, FnMapx, F)
    delete *F;
}

//------------------------------------------------------------------------------

void Dummy::Dump(FILE * fp)
{
fprintf(fp,"Dummy dump+++++++++++++++++++++++++++++++++++\n");
printf("Key        Method\n");
WALKVECTOR(FnMap_t*,FnMapx,F)
{
WALKMAP(unsigned,pMeth,(**F),i)printf("%#010x %#016x\n",(*i).first,(*i).second);
}
fprintf(fp,"Dummy dump-----------------------------------\n");
CommonBase::Dump(fp);
}

//==============================================================================





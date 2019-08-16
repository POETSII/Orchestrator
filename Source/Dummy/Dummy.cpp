//------------------------------------------------------------------------------

#include "Dummy.h"
#include "CommonBase.h"
#include "PMsg_p.hpp"
#include "mpi.h"
#include "Pglobals.h"
#include <stdio.h>

#include "OSFixes.hpp"

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
WALKMAP(unsigned,pMeth,(**F),i)
{
  //fprintf(fp,"%#010x 0x%#016x\n",(*i).first,(*i).second);
  fprintf(fp,"%#010x ",(*i).first);
  
  // Now for a horrible double type cast to get us a sensible function pointer.
  // void*s are only meant to point to objects, not functions. So we get to a 
  // void** as a pointer to a function pointer is an object pointer. We can then
  // follow this pointer to get to the void*, which we then reinterpret to get 
  // the function's address as a uint64_t.
  fprintf(fp,"%" PTR_FMT "\n",reinterpret_cast<uint64_t>(
                                *(reinterpret_cast<void**>(&((*i).second))))
          );
}
}
fprintf(fp,"Dummy dump-----------------------------------\n");
CommonBase::Dump(fp);
}

//==============================================================================





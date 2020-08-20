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
                                       // Spin on incoming MPI messages

PMsg_p Pkt;                            // Send out a bunch of random packets
Pkt.Src(Urank);
char buf[512] = "Ooogle";
for(int i=0;i<10;i++) {
  int len = strlen(buf)+1;             // Include the terminal '\0'
  Pkt.Put<char>(1,buf,len);
  Pkt.Key(Q::TEST);
  Pkt.Send(Q::ROOT); // dangerous: sending to a literal rank before procmap is loaded
  buf[0]++;
}

MPISpinner();

}

//------------------------------------------------------------------------------

void Dummy::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sDummy dump +++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sKey        Method\n",os);
WALKMAP(unsigned,pMeth,FnMap,i)
{
  fprintf(fp,"%s%#010x %" PTR_FMT "\n",os,(*i).first,
                        OSFixes::getAddrAsUint((*i).second));
}
CommonBase::Dump(off+2,fp);
fprintf(fp,"%sDummy dump -----------------------------------------------\n",os);

}

//==============================================================================





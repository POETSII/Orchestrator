//------------------------------------------------------------------------------

#include "mdecoder.h"
#include "mpi.h"
#include <stdio.h>
#include <string>
#include <vector>
using namespace std;

//==============================================================================

MDecoder::MDecoder(void * p)
{
par = p;
printf("*** MDecoder:: constructor\n"); fflush(stdout);
}

//------------------------------------------------------------------------------

MDecoder::~MDecoder()
{
printf("*** MDecoder:: destructor\n"); fflush(stdout);
}

//------------------------------------------------------------------------------

unsigned MDecoder::Decode(Msg_p * pZ)
{
printf("*** MDecoder::Decode(...)\n"); fflush(stdout);

CommonBase * pr = static_cast<CommonBase *>(par);
//Root * pr = static_cast<Root *>(par);

return (pr->*FnMap2[pZ->Key()])(pZ);

}

//------------------------------------------------------------------------------

void MDecoder::Dump(FILE * fp)
{
fprintf(fp,"MDecoder+++++++++++++++++++++++++++++++++\n");  fflush(stdout);
printf("Key        Method\n");
WALKMAP(unsigned,pMeth,FnMap2,i)printf("%#010x %#010x\n",(*i).first,(*i).second);
fprintf(fp,"MDecoder---------------------------------\n");  fflush(stdout);
}

//------------------------------------------------------------------------------
/*
void MDecoder::MPIcallback(string & s)
// Non-static receiver
{
}

//------------------------------------------------------------------------------

bool MDecoder::MPIcallback(void * p,string & s)
// Static receiver from the asynchronous input
{
static_cast<MDecoder *>(p)->MPIcallback(s);
return true;
}
  */
//==============================================================================





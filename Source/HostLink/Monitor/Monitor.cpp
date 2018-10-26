//------------------------------------------------------------------------------

#include "Monitor.h"
#include "CommonBase.h"
#include "PMsg_p.hpp"
#include "mpi.h"
#include "Pglobals.h"
#include "jnj.h"
#include <stdio.h>

//==============================================================================

Monitor::Monitor(int argc,char * argv[],string d):
  CommonBase(argc,argv,d,string(__FILE__))
{
                                       // File to hold stuff sent to monitor
sMonFile = pPmap->M[Urank] + "_" + uint2str(Urank);
for(unsigned i=0;i<sMonFile.size();i++)
  if(isalnum(sMonFile[i])==0)sMonFile[i]='_';
sMonFile += ".txt";
fMonFile = fopen(sMonFile.c_str(),"w");
item     = 0;                          // Initialise item counter

                                       // Load the message map
// ...There never was any message map....

MPISpinner();                          // Spin on MPI messages; exit only on DIE

//printf("********* Monitor rank %d on the way out\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

Monitor::~Monitor()
{
//printf("********* Monitor rank %d destructor\n",Urank); fflush(stdout);
fclose(fMonFile);                      // Lose the monitor file
}

//------------------------------------------------------------------------------

void Monitor::Dump(FILE * fp)
{
fprintf(fp,"Monitor dump+++++++++++++++++++++++++++++++++++\n");

fprintf(fp,"Monitor dump-----------------------------------\n");
CommonBase::Dump(fp);
}

//------------------------------------------------------------------------------

unsigned Monitor::OnNull(PMsg_p * p)
// Overload of CommonBase::OnNull. Here if the message has not been caught by
// anything in the Monitor message map OR the CommonBase message map.
// If you want it swallowed here, return 0 or 1.
// If you want it passed back to Decode() to be recorded as dropped, return 2.
// Note you cannot force a closedown from OnNull.
{
if (p==0) return 0;                    // Paranoia
if (fMonFile==0) return 0;             // Even more paranoia
fprintf(fMonFile,"............................................\n"
                 "Item %u \n",item++);
p->Dump(fMonFile);
fprintf(fMonFile,"............................................\n");

return 0;
}

//==============================================================================


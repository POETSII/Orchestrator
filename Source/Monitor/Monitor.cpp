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

FnMapx.push_back(new FnMap_t);  
                                       // Load the message map
(*FnMapx[0])[PMsg_p::KEY(Q::LOG,Q::POST,Q::N000,Q::N000)] = &Monitor::Onxxxx;

MPISpinner();                          // Spin on MPI messages; exit only on DIE

//printf("********* Monitor rank %d on the way out\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

Monitor::~Monitor()
{
WALKVECTOR(FnMap_t*,FnMapx,F) delete *F;
//printf("********* Monitor rank %d destructor\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

void Monitor::Dump(FILE * fp)
{
fprintf(fp,"Monitor dump+++++++++++++++++++++++++++++++++++\n");

fprintf(fp,"Monitor dump-----------------------------------\n");
CommonBase::Dump(fp);
}

//------------------------------------------------------------------------------

unsigned Monitor::Onxxxx(PMsg_p *,unsigned)
{


return 0;
}

//------------------------------------------------------------------------------

//==============================================================================


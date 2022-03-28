//------------------------------------------------------------------------------

#include "Monitor.h"
#include "CommonBase.h"
#include "PMsg_p.hpp"
#include "Pglobals.h"
#include <stdio.h>

//==============================================================================

Monitor::Monitor(int argc,char * argv[],string d):
  CommonBase(argc,argv,d,string(__FILE__))
{
                                       // Load the message map
FnMap[PMsg_p::KEY(Q::LOG,Q::POST,Q::N000,Q::N000)] = &Monitor::Onxxxx;

MPISpinner();                          // Spin on MPI messages; exit only on DIE

//printf("********* Monitor rank %d on the way out\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

Monitor::~Monitor()
{
//WALKVECTOR(FnMap_t*,FnMapx,F) delete *F;
//printf("********* Monitor rank %d destructor\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

void Monitor::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sMonitor dump +++++++++++++++++++++++++++++++++++++++++++++\n",os);
CommonBase::Dump(off+2,fp);
fprintf(fp,"%sMonitor dump ---------------------------------------------\n",os);
}

//------------------------------------------------------------------------------

unsigned Monitor::Onxxxx(PMsg_p *)
{


return 0;
}

//==============================================================================


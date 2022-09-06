//------------------------------------------------------------------------------

#include "ProcMap.h"
#include "CommonBase.h"
#include "Pglobals.h"

#include "OSFixes.hpp"

//==============================================================================

ProcMap::ProcMap(CommonBase * p)
{
par = p;                               // Who owns me?
M[Q::NAP] = string(csNOTaPROCproc);    // The name of the Unknown Process
}

//------------------------------------------------------------------------------

ProcMap::~ProcMap()
{
//printf("********* ProcMap rank %d destructor\n",par->Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

void ProcMap::Register(PMsg_p * Z)
// Load a single record into the map
{
ProcMap_t rec;                         // Create the record
int cnt;                               // Unload the message
int * pi = Z->Get<int>(1,cnt);         // ... rank
if (pi!=0) rec.P_rank = *pi;
Z->Get(2,rec.P_proc);                  // ... machine name
Z->Get(3,rec.P_user);                  // ... user name on machine
Z->Get(4,rec.P_class);                 // ... C++ class
unsigned * pu = Z->Get<unsigned>(5,cnt);
if (pu!=0) rec.P_BPW = *pu;            // ... bits per word
Z->Get(6,rec.P_compiler);              // ... compiler
Z->Get(7,rec.P_OS);                    // ... operating system
Z->Get(8,rec.P_source);                // ... source file
Z->Get(9,rec.P_binary);                // ... binary file
Z->Get(10,rec.P_TIME);                 // ... compilation time
Z->Get(11,rec.P_DATE);                 // ... compilation date
pi = Z->Get<int>(12,cnt);              // ... *provided* MPI thread class
if (pi!=0) rec.P_ttype = *pi;
rec.P_tig = (bool)MPI_WTIME_IS_GLOBAL; // Are we synchronised?
rec.P_tick = MPI_Wtick();              // Timer resolution

vPmap.push_back(rec);                  // Load the map
M[rec.P_rank] = rec.P_class;           // ...and the special process ranks
if (rec.P_class==string(csROOTproc))        U.Root       = rec.P_rank;
if (rec.P_class==string(csLOGSERVERproc))   U.LogServer  = rec.P_rank;
if (rec.P_class==string(csDUMMYproc))       U.Dummy.push_back(rec.P_rank);
if (rec.P_class==string(csRTCLproc))        U.RTCL       = rec.P_rank;
if (rec.P_class==string(csINJECTORproc))    U.Injector   = rec.P_rank;
if (rec.P_class==string(csNAMESERVERproc))  U.NameServer = rec.P_rank;
if (rec.P_class==string(csMONITORproc))     U.Monitor.push_back(rec.P_rank);
if (rec.P_class==string(csMOTHERSHIPproc))  U.Mothership.push_back(rec.P_rank);
if (rec.P_class==string(csMONSERVERproc))   U.MonServer  = rec.P_rank;
}

//------------------------------------------------------------------------------

void ProcMap::Show(FILE * fp)
{
fprintf(fp,"Process Map start\n"
           "++++++++++++++++++++++++++++++++++++++"
           "++++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"Me,Parent      %" PTR_FMT ",%" PTR_FMT "\n",
        OSFixes::getAddrAsUint(this),OSFixes::getAddrAsUint(par));

fprintf(fp,"Process map (%u entries):\n",static_cast<unsigned>(vPmap.size()));
WALKVECTOR(ProcMap_t,vPmap,i) {
  fprintf(fp,"\n- - - - - - - - - - - - - - - - - - - "
             "- - - - - - - - - - - - - - - - - - - \n");
  fprintf(fp,"\nClass %s rank %d compiled with %u-bit ",
             (*i).P_class.c_str(),(*i).P_rank,(*i).P_BPW);
  fprintf(fp,"%s under %s\n",
             (*i).P_compiler.c_str(),(*i).P_OS.c_str());
  fprintf(fp,"from %s\nto   %s\n",
             (*i).P_source.c_str(),(*i).P_binary.c_str());
  fprintf(fp,"at   %s on %s\n",
             (*i).P_TIME.c_str(),(*i).P_DATE.c_str());
  fprintf(fp,"executing on %s ",(*i).P_proc.c_str());
  fprintf(fp,"as user %s\n", (*i).P_user.c_str());
  fprintf(fp,"\nMPI hybrid programming model: ");
  switch ((*i).P_ttype) {
    case MPI_THREAD_MULTIPLE   : fprintf(fp,"Thread-multiple\n");   break;
    case MPI_THREAD_SERIALIZED : fprintf(fp,"Thread-serialized\n"); break;
    case MPI_THREAD_FUNNELED   : fprintf(fp,"Thread-funneled\n");   break;
    case MPI_THREAD_SINGLE     : fprintf(fp,"Thread-single\n");     break;
  }
  fprintf(fp,"MPI asserts time is%sglobal\n",(*i).P_tig ? " " : " NOT ");
  fprintf(fp,"MPI timer tick %e seconds\n",(*i).P_tick);
  fflush(fp);
}
fprintf(fp,"\n- - - - - - - - - - - - - - - - - - - "
           "- - - - - - - - - - - - - - - - - - - \n\n");

int * io_chan;
int   io_flag;
MPI_Comm_get_attr(MPI_COMM_WORLD,MPI_IO,&io_chan,&io_flag); // replaces deprecated MPI_attr_get

if (io_flag==0) fprintf(fp,"Rank %d asserts:"
                           " No MPI IO attribute found ?\n",par->Urank);
else {
  switch (*io_chan) {
    case MPI_PROC_NULL  : fprintf(fp,"Rank %d asserts:"
                                     " No rank has IO access ?\n",
                                     par->Urank);                   break;
    case MPI_ANY_SOURCE : fprintf(fp,"Rank %d asserts:"
                                     " IO access from any rank\n",
                                     par->Urank);                   break;
    default             : fprintf(fp,"Rank %d asserts:"
                                     " rank %d has IO\n",
                                     par->Urank,*io_chan);          break;
  }
}
fprintf(fp,"\n"); fflush(fp);
fprintf(fp,"Named process ranks... \n");
fprintf(fp,"---------------------- \n");
fprintf(fp,"Root rank       : %2d\n",U.Root);
fprintf(fp,"LogServer rank  : %2d\n",U.LogServer);
fprintf(fp,"RTCL rank       : %2d\n",U.RTCL);
fprintf(fp,"Injector rank   : %2d\n",U.Injector);
fprintf(fp,"NameServer rank : %2d\n",U.NameServer);
fprintf(fp,"MonServer rank  : %2d\n",U.MonServer);
fprintf(fp,"Mothership ranks: ");
WALKVECTOR(int,U.Mothership,i) fprintf(fp,"%2d ",*i);
fprintf(fp,"\nMonitor ranks   : ");
WALKVECTOR(int,U.Monitor,i) fprintf(fp,"%2d ",*i);
fprintf(fp,"\nDummy ranks     : ");
WALKVECTOR(int,U.Dummy,i) fprintf(fp,"%2d ",*i);
fprintf(fp,"\n\n");
WALKMAP(int,string,M,i) fprintf(fp,"%2d: %s\n",(*i).first,(*i).second.c_str());
fprintf(fp,"\n\n");
fprintf(fp,"Process Map end\n"
           "--------------------------------------"
           "--------------------------------------\n\n");
fflush(fp);
}

//==============================================================================

ProcMap::ProcMap_t::ProcMap_t()
// Defined 'horrible' values, all of which should be overwritten in normal use
{
P_rank  = Q::NAP;                      // Process rank
P_BPW   = 0;                           // Compiled bits per word
P_ttype = MPI_THREAD_SINGLE;           // Perceived MPI thread handling model
P_tig   = false;                       // Local MPI time_is_global opinion
P_tick  = 0.0;                         // Local MPI timer tick
}

//==============================================================================

ProcMap::U_t::U_t()
{
// Defined 'horrible' values, all of which should be overwritten in normal use
Root       = Q::NAP;
LogServer  = Q::NAP;
RTCL       = Q::NAP;
Injector   = Q::NAP;
NameServer = Q::NAP;
MonServer  = Q::NAP;
}

//==============================================================================

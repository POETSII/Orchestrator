//------------------------------------------------------------------------------

#include "Debug.h"
#include "Root.h"
#include "Environment.h"
#include "Unrec_t.h"
#include "Pglobals.h"
#include <stdio.h>
using namespace std;


//------------------------------------------------------------------------------

int main(int argc, char * argv[])
{
#if ORCHESTRATOR_DEBUG
printf("Attach debugger (Root process %d (%#0x)), hit <return>.....\n",
       GetPID(),GetPID());
fflush(stdout);
getchar();
#else
Cli cl(argc,argv);                     // Interpret any command line
int l,c;
cl.Err(l,c);
if (l<0) {                             // No errors?
  WALKVECTOR(Cli::Cl_t,cl.Cl_v,i) {    // Walk clause list
    if ((*i).Cl=="debug") {            // Is there a "debug" ?
      printf("Attach debugger (Root process %d (%#0x)), hit <return>.....\n",
             GetPID(),GetPID());
      fflush(stdout);
      getchar();
      break;
    }
  }
}
#endif
Root * pRoot = 0;
try {
  pRoot = new Root(argc,argv,string(csROOTproc));
}
catch(bad_alloc &) {
  printf("\n\n%s Main out of memory...    \n\n",csROOTproc);
}
catch(Unrec_t & u) {
  u.Post();
}
catch(...) {
  printf("\n\n%s Main unhandled exception...???   \n\n",csROOTproc);
}

delete pRoot;
DebugPrint("%s Main closing down.\n",csROOTproc);
return 0;
}

//------------------------------------------------------------------------------

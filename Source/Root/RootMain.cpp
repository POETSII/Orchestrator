//------------------------------------------------------------------------------

#include "Root.h"
#include "Environment.h"
#include "Unrec_t.h"
#include "Pglobals.h"
#include <stdio.h>

//------------------------------------------------------------------------------

int main(int argc, char * argv[])
{
printf("Attach debugger to Root process %d (%#0x).....\n",GetPID(),GetPID());
fflush(stdout);
getchar();

Root * pRoot = 0;
try {
  pRoot = new Root(argc,argv,string(csROOTproc));
}
catch(bad_alloc) {
  printf("\n\n%s Main out of memory...    \n\n",csROOTproc);
  fflush(stdout);
}
catch(Unrec_t u) {
  u.Post();
}
catch(...) {
  printf("\n\n%s Main unhandled exception...???   \n\n",csROOTproc);
  fflush(stdout);
}

delete pRoot;
printf("%s Main closing down\n"
       "Hit a key to really, finally go\n",csROOTproc);
fflush(stdout);
getchar();
return 0;

}

//------------------------------------------------------------------------------


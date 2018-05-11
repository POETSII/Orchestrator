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
  printf("\n\nRootMain out of memory...    \n\n");
  fflush(stdout);
}
catch(Unrec_t u) {
  u.Post();
}
catch(...) {
  printf("\n\nRootMain unhandled exception...???   \n\n");
  fflush(stdout);
}

delete pRoot;
printf("RootMain closing down\n"
       "Hit a key to really, finally go\n");
fflush(stdout);
getchar();
return 0;
}

//------------------------------------------------------------------------------


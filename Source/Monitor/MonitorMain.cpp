//------------------------------------------------------------------------------

#include "Debug.h"
#include "Monitor.h"
#include "Unrec_t.h"
#include "Pglobals.h"
#include <stdio.h>

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
Monitor * pMonitor = 0;
try {
  pMonitor = new Monitor(argc,argv,string(csMONITORproc));
}
catch(bad_alloc) {
  printf("\n\n%s Main out of memory...    \n\n",csMONITORproc);
  fflush(stdout);
}
catch(Unrec_t u) {
  u.Post();
}
catch(...) {
  printf("\n\n%s Main unhandled exception...???   \n\n",csMONITORproc);
  fflush(stdout);
}
DebugPrint("%s Main closing down.\n",csMONITORproc);
delete pMonitor;
return 0;
}

//------------------------------------------------------------------------------

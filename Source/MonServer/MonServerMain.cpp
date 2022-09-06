//------------------------------------------------------------------------------

#include "Debug.h"
#include "MonServer.h"
#include "Unrec_t.h"
#include "Pglobals.h"
#include <stdio.h>

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
MonServer * pMonServer = 0;
try {
  pMonServer = new MonServer(argc,argv,string(csMONSERVERproc));
}
catch(bad_alloc &) {
  printf("\n\n%s Main out of memory...    \n\n",csMONSERVERproc);
  fflush(stdout);
}
catch(Unrec_t & u) {
  u.Post();
}
catch(...) {
  printf("\n\n%s Main unhandled exception...???   \n\n",csMONSERVERproc);
  fflush(stdout);
}
DebugPrint("%s Main closing down.\n",csMONSERVERproc);
delete pMonServer;
return 0;
}

//------------------------------------------------------------------------------

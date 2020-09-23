//------------------------------------------------------------------------------

#include "Debug.h"
#include "LogServer.h"
#include "Unrec_t.h"
#include "Pglobals.h"
#include <stdio.h>

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
LogServer * pLogServer = 0;
try {
  pLogServer = new LogServer(argc,argv,string(csLOGSERVERproc));
}
catch(bad_alloc &) {
  printf("\n\n%s Main out of memory...    \n\n",csLOGSERVERproc);
  fflush(stdout);
}
catch(Unrec_t & u) {
  u.Post();
}
catch(...) {
  printf("\n\n%s Main unhandled exception...???   \n\n",csLOGSERVERproc);
  fflush(stdout);
}
DebugPrint("%s Main closing down.\n",csLOGSERVERproc);
delete pLogServer;
return 0;
}

//------------------------------------------------------------------------------

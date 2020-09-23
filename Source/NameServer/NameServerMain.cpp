//------------------------------------------------------------------------------

#include "Debug.h"
#include "NameServer.h"
#include "Unrec_t.h"
#include "Pglobals.h"
#include <stdio.h>

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
NameServer * pNameServer = 0;
try {
  pNameServer = new NameServer(argc,argv,string(csNAMESERVERproc));
}
catch(bad_alloc) {
  printf("\n\n%s Main out of memory...    \n\n",csNAMESERVERproc);
  fflush(stdout);
}
catch(Unrec_t u) {
  u.Post();
}
catch(...) {
  printf("\n\n%s Main unhandled exception...???   \n\n",csNAMESERVERproc);
  fflush(stdout);
}
DebugPrint("%s Main closing down.\n",csNAMESERVERproc);
delete pNameServer;
return 0;
}

//------------------------------------------------------------------------------

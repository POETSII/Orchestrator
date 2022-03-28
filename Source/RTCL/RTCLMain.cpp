//------------------------------------------------------------------------------

#include <stdio.h>
#include "Debug.h"
#include "RTCL.h"
#include "Unrec_t.h"
#include "Pglobals.h"

//------------------------------------------------------------------------------

int main(int argc, char * argv[])
{
RTCL * pRTCL = 0;
try {
  pRTCL = new RTCL(argc,argv,string(csRTCLproc));
}
catch(bad_alloc &) {
  printf("\n\n%s Main out of memory...    \n\n",csRTCLproc);
  fflush(stdout);
}
catch(Unrec_t & u) {
  u.Post();
}
catch(...) {
  printf("\n\n%s Main unhandled exception...???   \n\n",csRTCLproc);
  fflush(stdout);
}
DebugPrint("%s Main closing down.\n",csRTCLproc);
delete pRTCL;
return 0;
}

//------------------------------------------------------------------------------

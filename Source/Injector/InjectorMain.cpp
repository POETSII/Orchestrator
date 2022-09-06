//------------------------------------------------------------------------------

#include "Debug.h"
#include "Injector.h"
#include "Unrec_t.h"
#include "Pglobals.h"
#include <stdio.h>

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
Injector * pInjector = 0;
try {
  pInjector = new Injector(argc,argv,string(csINJECTORproc));
}
catch(bad_alloc &) {
  printf("\n\n%s Main out of memory...    \n\n",csINJECTORproc);
  fflush(stdout);
}
catch(Unrec_t & u) {
  u.Post();
}
catch(...) {
  printf("\n\n%s Main unhandled exception...???   \n\n",csINJECTORproc);
  fflush(stdout);
}
DebugPrint("%s Main closing down.\n",csINJECTORproc);
delete pInjector;
return 0;
}

//------------------------------------------------------------------------------

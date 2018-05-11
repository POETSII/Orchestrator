//------------------------------------------------------------------------------

#include "Dummy.h"
#include "Pglobals.h"
#include <stdio.h>

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
Dummy * pDummy = new Dummy(argc,argv,string(csDUMMYproc));
printf("%s Main closing down\n",csDUMMYproc);
fflush(stdout);
delete pDummy;
return 0;
}

//------------------------------------------------------------------------------
 

//------------------------------------------------------------------------------

#include "Debug.h"
#include "Dummy.h"
#include "Pglobals.h"
#include <stdio.h>

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
Dummy * pDummy = new Dummy(argc,argv,string(csDUMMYproc));
DebugPrint("%s Main closing down.\n",csDUMMYproc);
delete pDummy;
return 0;
}

//------------------------------------------------------------------------------

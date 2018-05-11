//------------------------------------------------------------------------------

#include <stdio.h>
#include "Environment.h"
#ifdef __BORLANDC__
#include <process.h>
#endif
#include <string>
using namespace std;

//------------------------------------------------------------------------------

unsigned BPW()
{
return 8*sizeof(void *);
}

//------------------------------------------------------------------------------

string GetCompiler()
{
char buff[256];
#ifdef __BORLANDC__
sprintf(buff,"Borland compiler %0x",__BORLANDC__);
#elif _MSC_VER
sprintf(buff,"u$oft compiler %d",_MSC_VER);
#elif __GNUC__
sprintf(buff, "gcc compiler %d",__GNUC__);
#else
sprintf(buff,"Unidentified compiler");
#endif
return string(buff);
}

//------------------------------------------------------------------------------

string GetOS()
{
#ifdef __FreeBSD__
return string("FreeBSD");
#elif __GNU__
return string("GNU");
#elif __linux__
return string("Linux");
#elif __unix__
return string("Unix");
#elif _WIN32                           // Set for both 32 and 64 bit
return string("Windows");
#else
return string("Unidentified OS");
#endif
}

//------------------------------------------------------------------------------

int GetPID()
{
#ifdef __BORLANDC__
return getpid();
#else
return 0;
#endif
}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

#include <stdio.h>
#include <cstdlib>
#include "Environment.h"
// Should this preprocessor fork be in OSFixes.hpp?
// Yes, TODO:
#ifdef __BORLANDC__
#include <process.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __linux__
#include <sys/types.h>
#include <unistd.h>
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
#elif __linux__
return getpid();
#elif __unix__
return getpid();
#elif _WIN32                           // Set for both 32 and 64 bit
return GetCurrentProcessId();
#else
return 0;
#endif
}

//------------------------------------------------------------------------------

string GetUser()
{
char * pU = 0;
#ifdef __BORLANDC__
pU = getenv("USERNAME");
return string(pU);
#elif _WIN32
pU = getenv("USERNAME");
return string(pU);
#elif __linux__
pU = getenv("USER");
return string(pU);
#elif __unix__
pU = getenv("USER");
return string(pU);
#else
return string("Unknown_User");
#endif
}

//------------------------------------------------------------------------------

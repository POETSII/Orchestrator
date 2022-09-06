#include "Misc.h"
#include <stdio.h>

//==============================================================================

unsigned Cprint(char * buf,unsigned bytes)
// Routine to allow writing character strings to a (the) console from within a
// Windoze program. Input "bytes" is the number of characters you want to write
// from the buffer, return value is the number of characters actually written.
// A process can have only one console - AllocConsole creates one the first
// time it is called, and it's subsequently ignored, so we don't need to shield
// ourselves with static flags. Likewise, we don't need to clear up after
// ourselves. The only point to FreeConsole is to allow a subsequent call to
// AllocConsole to succeed, but I've never seen the point.
// This is all Windoze stuff; not a chance under Linux.
{
AllocConsole();                        // Get a console
                                       // Get the output handle
HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
unsigned long nwrite;                  // Number of characters written

SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN     |  FOREGROUND_RED   |
                                 FOREGROUND_INTENSITY |  BACKGROUND_GREEN |
                                 BACKGROUND_BLUE);

_OVERLAPPED * dummy;                   // Allows asynchronous IO - we don't care
if (bytes==0) bytes = strlen(buf);     // If no length supplied, work it out
                                       // And do it...
WriteFile(hStdout,buf,bytes,&nwrite,dummy);
return nwrite;
}

//------------------------------------------------------------------------------

unsigned Cprintf(const char * fmt, ...)
// And now we're really showing off: formatted write to the console.
// This may not work in Linux-land, because gcc puts register pointers on the
// stack
{
int cnt;                                          // Characters written
const int LEN = 256;                              // Buffer length
char buffer[LEN];                                 // Zer bufferings
unsigned char * argptr;                           // aka va_list()
argptr = (((unsigned char *)&fmt) + sizeof(fmt)); // aka va_start()
cnt = vsprintf(buffer,fmt,argptr);                // Do it in the buffer
Cprint(buffer,cnt);                               // -> Console
if (cnt>LEN) exit(1);                             // Buffer overrun ?
return(cnt);                                      // va_end does nothing anyway
}

//------------------------------------------------------------------------------

int getline(FILE * fp,string & rstr,char delim)
// I really cannot believe I'm having to write this......
// Although the interface is a little more sane:
// If the input hits "delim", it is discarded and the string returned; the
// function returns the (+ve) string length
// If the input hits EOF, it is discarded and the function returns EOF (-1)
// The string may/may not be empty.
{
char uc;
rstr.clear();

for(;;) {
  uc = fgetc(fp);
  if (uc==delim) return rstr.size();
  if (uc==EOF)   return EOF;
  rstr += uc;
}

}

//------------------------------------------------------------------------------

AnsiString STL2VCL(string STL_s)
{
return AnsiString(STL_s.c_str());
}

//------------------------------------------------------------------------------

string VCL2STL(AnsiString VCL_s)
{
return string(VCL_s.c_str());
}

//------------------------------------------------------------------------------



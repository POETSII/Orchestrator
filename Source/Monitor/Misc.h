#ifndef __MiscH__H
#define __MiscH__H

#include <vcl.h>
#include <stdio.h>
#include <string>
using namespace std;

//==============================================================================

unsigned      Cprint(char *,unsigned=0);
unsigned      Cprintf(const char *, ...);
int           getline(FILE *,string &,char = '\n');
AnsiString    STL2VCL(string);
string        VCL2STL(AnsiString);



//==============================================================================



#endif

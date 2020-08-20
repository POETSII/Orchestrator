#include "DumpChan.h"
#include <string>
using namespace std;

FILE * DumpChan::dfp = stdout;

//==============================================================================

void DumpChan::Dump(unsigned off,FILE * fp)
{
string s = string(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sDumpChan +++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sDump file channel =  %#018lx\n",os,(uint64_t)dfp);
fprintf(fp,"%sDumpChan -------------------------------------------------\n",os);
fflush(fp);
}

//==============================================================================

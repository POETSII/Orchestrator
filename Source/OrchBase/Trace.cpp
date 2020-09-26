//------------------------------------------------------------------------------

#include "Trace.h"
#include "flat.h"

//==============================================================================
/* The trace object straddles Root::MPISpinner. It is effectively a low-level
LogServer. It is not intended to be user-facing; it stores events in a machine-
readable form to be dissected later (leading onto runtime signatures for
long-term testing. What's an event? Anything you choose to shove in, consisting
(at the moment) of a string key and a data item, the definition of which will -
evolve.
The cool way to do this would be to shove all the pushed events onto a UIF
datastructure, then stream this to a file as and when, but time is not on my
side here. KISS: whack everything to a text file as soon as it comes in.
*/
//==============================================================================

Trace::Trace()
{
alive = false;                         // Trace capability off by default

}

//------------------------------------------------------------------------------

Trace::~Trace()
{
if (!alive) return;                    // It wasn't on
fclose(ft);
}

//------------------------------------------------------------------------------

void Trace::Close()
{
if (!alive) return;                    // It wasn't on
fprintf(ft,"\n[POETS_trace_file(close)]\n");
fprintf(ft,"\"%10s\" : \n",GetTime());
fprintf(ft,"\n");
}

//------------------------------------------------------------------------------

void Trace::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sTrace ++++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sTrace --------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void Trace::Init()
{
ft = fopen("POETS_Trace.log","w");     // Try to open the file
if (ft==0) return;
alive = true;                          // Good to go; write the header
fprintf(ft,"[POETS_trace_file(open)]\n");
fprintf(ft,"\"%10s\" : Date = \"%s\"\n",GetTime(),GetDate());
}

//------------------------------------------------------------------------------

void Trace::Push(string s)
{
fprintf(ft,"\"%10s\" : %s\n",GetTime(),s.c_str());
}

//------------------------------------------------------------------------------

void Trace::Push(string s,string s2)
{
fprintf(ft,"\"%10s\" : %s = %s\n",GetTime(),s.c_str(),s2.c_str());
}

//------------------------------------------------------------------------------

void Trace::Push(string s,unsigned u)
{
fprintf(ft,"\"%10s\" : %s = %u\n",GetTime(),s.c_str(),u);
}

//------------------------------------------------------------------------------

void Trace::PushS(string s)
// Push a section header to the trace file
{
if (!alive) return;                    // Open for business?
fprintf(ft,"\n[\"%s\"]\n",s.c_str());
}


//==============================================================================

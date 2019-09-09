//------------------------------------------------------------------------------

#include "Config_t.h"
#include "P_graph.h"

//==============================================================================

Config_t::Config_t(P_graph * _p,string _s):par(_p)
{
Name(_s);
Npar(_p);
bMem    = 0;
boards  = 0;
cores   = 0;
threads = 0;
bMem    = 1000000;
boards  = 2;
cores   = 4;
threads = 5;
}

//------------------------------------------------------------------------------

Config_t::~Config_t()
{

}

//------------------------------------------------------------------------------

void Config_t::Dump(FILE * fp)
{
string s = FullName();
fprintf(fp,"Config_t %35s++++++++++++++++++++++++++++++++++++\n",s.c_str());
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      0x%#08p,0x%#08p\n",this,par);
fprintf(fp,"bMem           = %u\n",bMem);
fprintf(fp,"boards         = %u\n",boards);
fprintf(fp,"cores          = %u\n",cores);
fprintf(fp,"threads        = %u\n",threads);
fprintf(fp,"Config_t %35s------------------------------------\n",s.c_str());
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned Config_t::GetBMem()
// Return available memory on each board
{
return bMem;
}

//------------------------------------------------------------------------------

unsigned Config_t::GetBoards()
// Return board count in the box
{
return boards;
}

//------------------------------------------------------------------------------

unsigned Config_t::GetCores()
// Return core count in a board
{
return cores;
}

//------------------------------------------------------------------------------

unsigned Config_t::GetThreads()
// Return thread count in
{
return threads;
}

//------------------------------------------------------------------------------

unsigned Config_t::ThreadsPerBox()
{
return threads * cores * boards;
}

//==============================================================================




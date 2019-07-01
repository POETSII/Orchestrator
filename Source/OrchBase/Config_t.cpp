//------------------------------------------------------------------------------

#include "Config_t.h"
#include "P_graph.h"
#include  "build_defs.h"
#include "stdint.h"

//==============================================================================

Config_t::Config_t(P_graph * _p,string _s):par(_p)
{
Name(_s);
Npar(_p);
// initialise all members according to the default POETS configuration
bMem    = MEM_PER_BOARD;
boards  = BOARDS_PER_BOX;
cores   = CORES_PER_BOARD;
threads = THREADS_PER_CORE;
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
fprintf(fp,"Me,Parent      %#018lx,%#018lx\n",
        (uint64_t) this, (uint64_t) par);
fprintf(fp,"bMem           = %llu\n",bMem);
fprintf(fp,"boards         = %u\n",boards);
fprintf(fp,"cores          = %u\n",cores);
fprintf(fp,"threads        = %u\n",threads);
fprintf(fp,"Config_t %35s------------------------------------\n",s.c_str());
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned long long Config_t::GetBMem()
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

void Config_t::SetBMem(unsigned long long mem)
// Assign available memory on each board
{
bMem = mem;
}

//------------------------------------------------------------------------------

void Config_t::SetBoards(unsigned nBoards)
// Assign board count in the box
{
boards = nBoards;
}

//------------------------------------------------------------------------------

void Config_t::SetCores(unsigned nCores)
// Assign core count in a board
{
cores = nCores;
}

//------------------------------------------------------------------------------

void Config_t::SetThreads(unsigned nThreads)
// Assign thread count in
{
threads = nThreads;
}

//------------------------------------------------------------------------------

unsigned Config_t::ThreadsPerBox()
{
return threads * cores * boards;
}

//==============================================================================




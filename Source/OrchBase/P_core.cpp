//------------------------------------------------------------------------------

#include "P_core.h"
#include "macros.h"

//==============================================================================

P_core::P_core(P_board * _p,string _s):par(_p)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
pCoreBin = new Bin();                  // Instruction binary file holder
pDataBin = new Bin();                  // Data binary file holder
}

//------------------------------------------------------------------------------

P_core::~P_core()
{
WALKVECTOR(P_thread *,P_threadv,i) delete *i;
if (pCoreBin!=0) delete pCoreBin;
if (pDataBin != 0) delete pDataBin;
}

//------------------------------------------------------------------------------

void P_core::Dump(FILE * fp)
{
string s = FullName();
fprintf(fp,"P_core %35s++++++++++++++++++++++++++++++++++++++\n",s.c_str());
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      %#018lx,%#018lx\n",
        (uint64_t) this, (uint64_t) par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
if (pCoreBin!=0) pCoreBin->Dump(fp);
else fprintf(fp,"Core binary undefined\n");
if (pDataBin!=0) pDataBin->Dump(fp);
else fprintf(fp,"Threads' data binary undefined\n");
fprintf(fp,"THREADS IN CORE %35s+++++++++++++++++++++++++++++\n",s.c_str());
WALKVECTOR(P_thread *,P_threadv,i) (*i)->Dump(fp);
fprintf(fp,"THREADS IN CORE %35s-----------------------------\n",s.c_str());
NameBase::Dump(fp);
fprintf(fp,"P_core %35s--------------------------------------\n",s.c_str());
fflush(fp);
}

//==============================================================================




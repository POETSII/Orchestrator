//------------------------------------------------------------------------------

#include "P_core.h"
#include "macros.h"

//==============================================================================

P_core::P_core(P_board * _p,string _s):par(_p)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
pCoreBin = new Bin();                  // Binary file holder
}

//------------------------------------------------------------------------------

P_core::~P_core()
{
WALKVECTOR(P_thread *,P_threadv,i) delete *i;
if (pCoreBin!=0) delete pCoreBin;
}

//------------------------------------------------------------------------------

void P_core::Dump(FILE * fp)
{
string s = FullName();
fprintf(fp,"P_core %35s++++++++++++++++++++++++++++++++++++++\n",s.c_str());
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      0x%#08p,0x%#08p\n",this,par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
if (pCoreBin!=0) pCoreBin->Dump(fp);
else fprintf(fp,"Core binary undefined\n");
fprintf(fp,"THREADS IN CORE %35s+++++++++++++++++++++++++++++\n",s.c_str());
WALKVECTOR(P_thread *,P_threadv,i) (*i)->Dump(fp);
fprintf(fp,"THREADS IN CORE %35s-----------------------------\n",s.c_str());
NameBase::Dump(fp);
fprintf(fp,"P_core %35s--------------------------------------\n",s.c_str());
fflush(fp);
}

//==============================================================================




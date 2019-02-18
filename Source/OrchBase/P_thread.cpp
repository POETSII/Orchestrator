//------------------------------------------------------------------------------

#include "P_thread.h"
#include "P_core.h"
#include "macros.h"

//==============================================================================

P_thread::P_thread(P_core * _p,string _s):par(_p)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
}

//------------------------------------------------------------------------------

P_thread::~P_thread()
{
}

//------------------------------------------------------------------------------

void P_thread::Dump(FILE * fp)
{
string s = FullName();
fprintf(fp,"P_thread %35s++++++++++++++++++++++++++++++++++++\n",s.c_str());
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      %#018lx,%#018lx\n",
        (uint64_t) this, (uint64_t) par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
fprintf(fp,"DEVICES IN THREAD %35s+++++++++++++++++++++++++++\n",s.c_str());
if (P_devicel.empty())fprintf(fp,"Device list empty\n");
else WALKLIST(P_device *,P_devicel,i)fprintf(fp,"%s\n",(*i)->FullName().c_str());
fprintf(fp,"DEVICES IN THREAD %35s---------------------------\n",s.c_str());
NameBase::Dump(fp);
fprintf(fp,"P_thread %35s------------------------------------\n",s.c_str());
fflush(fp);
}

//==============================================================================




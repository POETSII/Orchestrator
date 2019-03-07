//------------------------------------------------------------------------------

#include "P_board.h"
#include "P_box.h"
#include "P_device.h"
#include "P_core.h"
#include "macros.h"

//==============================================================================

P_board::P_board(P_box * _p,string _s):par(_p),pSup(0),uMem(0)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
}

//------------------------------------------------------------------------------

P_board::~P_board()
{
WALKVECTOR(P_core *,P_corev,i) delete *i;
}

//------------------------------------------------------------------------------

void P_board::Dump(FILE * fp)
{
string s = FullName();
fprintf(fp,"P_board %35s+++++++++++++++++++++++++++++++++++++\n",s.c_str());
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      %#018lx,%#018lx\n",
        (uint64_t) this, (uint64_t) par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
fprintf(fp,"Memory         %u\n",uMem);
fprintf(fp,"Supervisor     %s\n",pSup==0?"Undefined":pSup->FullName().c_str());
fprintf(fp,"CORES ON BOARD %35s++++++++++++++++++++++++++++++\n",s.c_str());
WALKVECTOR(P_core *,P_corev,i)(*i)->Dump(fp);
fprintf(fp,"CORES ON BOARD %35s------------------------------\n",s.c_str());
NameBase::Dump(fp);
fprintf(fp,"P_board %35s-------------------------------------\n",s.c_str());
fflush(fp);
}

//==============================================================================




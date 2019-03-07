//------------------------------------------------------------------------------

#include "P_task.h"
#include "P_super.h"

//==============================================================================

P_task::P_task(OrchBase * _p,string _s):par(_p),pSup(0),pD(0),pP_typdcl(0)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
linked = false;                        // Not linked yet
pOwn = 0;                              // No owner yet
pD = new D_graph(this,"D_graph");
}

//------------------------------------------------------------------------------

P_task::~P_task()
{
par->Post(802,Name());
if (pD!=0) delete pD;
}

//------------------------------------------------------------------------------

void P_task::Clear()
{
}

//------------------------------------------------------------------------------

void P_task::Dump(FILE * fp)
{
fprintf(fp,"P_task++++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      %#018lx,%#018lx\n",
        (uint64_t) this, (uint64_t) par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
if (pP_typdcl==0) fprintf(fp,"No type declare block\n");
else fprintf(fp,"Type declare block %s\n",pP_typdcl->FullName().c_str());
if (pSup==0) fprintf(fp,"No supervisor device defined\n");
else fprintf(fp,"Supervisor device %s\n",pSup->FullName().c_str());
fprintf(fp,"Linked       %s\n",linked?"yes":"no");
fprintf(fp,"Owner        %s\n",pOwn==0?"No owner":pOwn->FullName().c_str());
fprintf(fp,"User source file (if any) ||%s||\n",filename.c_str());
fprintf(fp,"PROOF OF LIFE+++++++++++++++++++++++++++++++\n");
if (!PoL.IsPoL) fprintf(fp,"User task\n");
else PoL.Dump(fp);
fprintf(fp,"PROOF OF LIFE-------------------------------\n");
if (pD==0) fprintf(fp,"No device graph defined\n");
else pD->Dump(fp);
fprintf(fp,"P_task--------------------------------------\n");
fflush(fp);
}

//------------------------------------------------------------------------------

bool P_task::IsPoL()
{
return PoL.IsPoL;
}

//------------------------------------------------------------------------------

//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

P_task::PoL_t::PoL_t(): IsPoL(false){}

//------------------------------------------------------------------------------

void P_task::PoL_t::Dump(FILE * fp)
{
fprintf(fp,"P_task::PoL_t + + + + + + + + + + + + + + + \n");
fprintf(fp,"IsPoL      : %s\n",IsPoL ? "TRUE" : "FALSE");
fprintf(fp,"PoL type   : %s\n",type.c_str());
fprintf(fp,"Parameters :\n");
WALKVECTOR(string,params,i) fprintf(fp,"  %s\n",(*i).c_str());
fprintf(fp,"P_task::PoL_t - - - - - - - - - - - - - - - \n");
fflush(fp);
}

//==============================================================================




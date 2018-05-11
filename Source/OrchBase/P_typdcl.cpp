//------------------------------------------------------------------------------

#include "P_typdcl.h"
#include "OrchBase.h"
#include "P_task.h"
#include "P_devtyp.h"

//==============================================================================

P_typdcl::P_typdcl(OrchBase * _p,string _s):par(_p),pPropsI(0),pPropsD(0)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
}

//------------------------------------------------------------------------------

P_typdcl::~P_typdcl()
{
WALKVECTOR(P_devtyp *,P_devtypv,i) delete *i;
WALKVECTOR(P_message *,P_messagev,i) delete *i;
WALKVECTOR(CFrag *,General,i) delete *i;
if (pPropsI!=0) delete pPropsI;
if (pPropsD!=0) delete pPropsD;
}

//------------------------------------------------------------------------------

void P_typdcl::Dump(FILE * fp)
{
fprintf(fp,"P_typdcl++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      0x%#08p,0x%#08p\n",this,par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
fprintf(fp,"REFERENCED BY+++++++++++++++++++++++++++++++\n");
WALKLIST(P_task *,P_taskl,i)fprintf(fp,"%s\n",(*i)->FullName().c_str());
fprintf(fp,"REFERENCED BY-------------------------------\n");
fprintf(fp,"MESSAGE TYPES+++++++++++++++++++++++++++++++\n");
WALKVECTOR(P_message *,P_messagev,i)(*i)->Dump(fp);
fprintf(fp,"MESSAGE TYPES-------------------------------\n");
fprintf(fp,"SHARED CODE+++++++++++++++++++++++++++++++++\n");
WALKVECTOR(P_message *,P_messagev,i)(*i)->Dump(fp);
fprintf(fp,"SHARED CODE---------------------------------\n");
fprintf(fp,"Default properties initialiser    %#08p\n",pPropsI);
if (pPropsI!=0) pPropsI->Dump(fp);
fprintf(fp,"Properties declaration            %#08p\n",pPropsD);
if (pPropsD!=0) pPropsD->Dump(fp);
fprintf(fp,"DEVICE TYPES++++++++++++++++++++++++++++++++\n");
WALKVECTOR(P_devtyp *,P_devtypv,i)(*i)->Dump(fp);
fprintf(fp,"DEVICE TYPES--------------------------------\n");
NameBase::Dump(fp);
fprintf(fp,"P_typdcl------------------------------------\n");
fflush(fp);
}

//==============================================================================




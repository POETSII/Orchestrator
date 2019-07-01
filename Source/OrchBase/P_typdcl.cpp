//------------------------------------------------------------------------------

#include "P_typdcl.h"
#include "OrchBase.h"
#include "P_task.h"
#include "P_devtyp.h"
#include "stdint.h"
// #include "P_datatype.h"

//==============================================================================

P_typdcl::P_typdcl(OrchBase * _p,string _s):par(_p),pPropsI(0),pPropsD(0)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
// pProps=0;                           // temporary until P_datatype is debugged
}

//------------------------------------------------------------------------------

P_typdcl::~P_typdcl()
{
WALKMAP(string,P_devtyp *,P_devtypm,i) delete i->second;
// WALKMAP(string,P_message *,P_messagem,i) delete i->second;
// WALKMAP(string,P_devicetype *,P_typedefm, t) delete t->second;
WALKVECTOR(CFrag *,General,i) delete *i;
// if (pProps!=0) delete pProps;
if (pPropsI!=0) delete pPropsI;
if (pPropsD!=0) delete pPropsD;
}

//------------------------------------------------------------------------------

void P_typdcl::Dump(FILE * fp)
{
fprintf(fp,"P_typdcl++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      %#018lx,%#018lx\n",
        (uint64_t) this, (uint64_t) par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
fprintf(fp,"REFERENCED BY+++++++++++++++++++++++++++++++\n");
WALKLIST(P_task *,P_taskl,i)fprintf(fp,"%s\n",(*i)->FullName().c_str());
fprintf(fp,"REFERENCED BY-------------------------------\n");
fprintf(fp,"MESSAGE TYPES+++++++++++++++++++++++++++++++\n");
WALKMAP(string,P_message *,P_messagem,i) i->second->Dump(fp);
fprintf(fp,"MESSAGE TYPES-------------------------------\n");
fprintf(fp,"SHARED CODE+++++++++++++++++++++++++++++++++\n");
WALKVECTOR(CFrag *,General,i)(*i)->Dump(fp);
fprintf(fp,"SHARED CODE---------------------------------\n");
// fprintf(fp,"Properties %#018lx\n", (uint64_t) pProps);
// if (pProps!=0) pProps->Dump(fp);
fprintf(fp,"Default properties initialiser    %#018lx\n", (uint64_t) pPropsI);
if (pPropsI!=0) pPropsI->Dump(fp);
fprintf(fp,"Properties declaration            %#018lx\n", (uint64_t) pPropsD);
if (pPropsD!=0) pPropsD->Dump(fp);
fprintf(fp,"DEVICE TYPES++++++++++++++++++++++++++++++++\n");
WALKMAP(string,P_devtyp *,P_devtypm,i) i->second->Dump(fp);
fprintf(fp,"DEVICE TYPES--------------------------------\n");
NameBase::Dump(fp);
fprintf(fp,"P_typdcl------------------------------------\n");
fflush(fp);
}

//==============================================================================




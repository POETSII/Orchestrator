//------------------------------------------------------------------------------

#include "P_devtyp.h"
#include "P_typdcl.h"
#include "macros.h"
#include "P_pintyp.h"

//==============================================================================

P_devtyp::P_devtyp(P_typdcl * _p,string _s):par(_p)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
pOnIdle = 0;
pOnRTS = 0;
pOnCtl = 0;
pPropsI = 0;
pStateI = 0;
pPropsD = 0;
pStateD = 0;
idx = 0;
}

//------------------------------------------------------------------------------

P_devtyp::~P_devtyp()
{

}

//------------------------------------------------------------------------------

void P_devtyp::Dump(FILE * fp)
{
fprintf(fp,"P_devtyp++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      %#018lx,%#018lx\n",
        (uint64_t) this, (uint64_t) par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
fprintf(fp,"HANDLER CODE++++++++++++++++++++++++++++++++\n");
WALKVECTOR(CFrag *,pHandlv,i)(*i)->Dump(fp);
fprintf(fp,"OnRTS          %#018lx\n", (uint64_t) pOnRTS);
if (pOnRTS!=0) pOnRTS->Dump(fp);
fprintf(fp,"OnIdle         %#018lx\n", (uint64_t) pOnIdle);
if (pOnIdle!=0) pOnIdle->Dump(fp);
fprintf(fp,"Properties default initialiser %#018lx\n", (uint64_t) pPropsI);
if (pPropsI!=0) pPropsI->Dump(fp);
fprintf(fp,"State default initialiser %#018lx\n", (uint64_t) pPropsI);
if (pStateI!=0) pStateI->Dump(fp);
fprintf(fp,"Properties declaration %#018lx\n", (uint64_t) pPropsD);
if (pPropsD!=0) pPropsD->Dump(fp);
fprintf(fp,"State declaration %#018lx\n", (uint64_t) pStateD);
if (pStateD!=0) pStateD->Dump(fp);
fprintf(fp,"HANDLER CODE--------------------------------\n");
fprintf(fp,"OUTPUT PIN TYPES++++++++++++++++++++++++++++\n");
WALKVECTOR(P_pintyp *,P_pintypOv,i)(*i)->Dump(fp);
fprintf(fp,"OUTPUT PIN TYPES----------------------------\n");
fprintf(fp,"INPUT PIN TYPES+++++++++++++++++++++++++++++\n");
WALKVECTOR(P_pintyp *,P_pintypIv,i)(*i)->Dump(fp);
fprintf(fp,"INPUT PIN TYPES-----------------------------\n");
NameBase::Dump(fp);
fprintf(fp,"P_devtyp------------------------------------\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned int P_devtyp::MemPerDevice()
{
    return 32; // temporary to get things running, later this will be transformed
               // to a real function
}

//==============================================================================




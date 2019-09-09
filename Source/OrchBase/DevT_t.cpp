//------------------------------------------------------------------------------

#include "DevT_t.h"
#include "GraphT_t.h"
#include "macros.h"
#include "PinT_t.h"

//==============================================================================

DevT_t::DevT_t(GraphT_t * _p,string _s):par(_p)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
pOnIdle = 0;
pOnHW   = 0;
pOnRTS  = 0;
pOnCtl  = 0;
//pPropsI = 0;
//pStateI = 0;
pPropsD = 0;
pStateD = 0;
devTyp = 'U';                          // Undefined
}

//------------------------------------------------------------------------------

DevT_t::~DevT_t()
{
WALKVECTOR(CFrag *,ShCd_v,i) delete *i;
WALKVECTOR(PinT_t *,PinTI_v,i) delete *i;
WALKVECTOR(PinT_t *,PinTO_v,i) delete *i;
if(pOnIdle!=0) delete pOnIdle;
if(pOnHW  !=0) delete pOnHW;
if(pOnRTS !=0) delete pOnRTS;
if(pOnCtl !=0) delete pOnCtl;
if(pPropsD!=0) delete pPropsD;
if(pStateD!=0) delete pStateD;
WALKVECTOR(Meta_t *,Meta_v,i) delete *i;
}

//------------------------------------------------------------------------------

void DevT_t::Dump(FILE * fp)
{
fprintf(fp,"DevT_t++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      0x%#08p,0x%#08p\n",this,par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
fprintf(fp,"Shared code:\n");
WALKVECTOR(CFrag *,ShCd_v,i)(*i)->Dump(fp);
fprintf(fp,"Handler code:\n");
fprintf(fp,"OnRTS          %#08p\n",pOnRTS);
if (pOnRTS!=0) pOnRTS->Dump(fp);
fprintf(fp,"OnIdle         %#08p\n",pOnIdle);
if (pOnIdle!=0) pOnIdle->Dump(fp);
//fprintf(fp,"Properties default initialiser %#08p\n",pPropsI);
//if (pPropsI!=0) pPropsI->Dump(fp);
//fprintf(fp,"State default initialiser %#08p\n",pStateI);
//if (pStateI!=0) pStateI->Dump(fp);
fprintf(fp,"Properties declaration %#08p\n",pPropsD);
if (pPropsD!=0) pPropsD->Dump(fp);
fprintf(fp,"State declaration %#08p\n",pStateD);
if (pStateD!=0) pStateD->Dump(fp);
fprintf(fp,"HANDLER CODE--------------------------------\n");
fprintf(fp,"OUTPUT PIN TYPES++++++++++++++++++++++++++++\n");
WALKVECTOR(PinT_t *,PinTO_v,i)(*i)->Dump(fp);
fprintf(fp,"OUTPUT PIN TYPES----------------------------\n");
fprintf(fp,"INPUT PIN TYPES+++++++++++++++++++++++++++++\n");
WALKVECTOR(PinT_t *,PinTI_v,i)(*i)->Dump(fp);
fprintf(fp,"INPUT PIN TYPES-----------------------------\n");
NameBase::Dump(fp);
fprintf(fp,"DevT_t------------------------------------\n");
fflush(fp);
}

//------------------------------------------------------------------------------

PinT_t * DevT_t::Loc_pintyp(string s,char IO)
// Locate a pintype with name "s". IO == 'I': search inpins, == 'O': ... guess
{
if (IO=='I') {
  WALKVECTOR(PinT_t *,PinTI_v,i) if ((*i)->Name()==s) return *i;
  return 0;
}
WALKVECTOR(PinT_t *,PinTO_v,i) if ((*i)->Name()==s) return *i;
return 0;
}

//------------------------------------------------------------------------------

unsigned int DevT_t::MemPerDevice()
{
    return 32; // temporary to get things running, later this will be transformed
               // to a real function
}

//==============================================================================




//------------------------------------------------------------------------------

#include "DevT_t.h"
#include "GraphT_t.h"
#include "macros.h"
#include "PinT_t.h"
#include "CFrag.h"
#include "Meta_t.h"
#include <stdint.h>

//==============================================================================

DevT_t::DevT_t(GraphT_t * _p,string _s):par(_p)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
pPinTSI = 0;                           // (Single) input pin from supervisor
pPinTSO = 0;                           // (Single) output pin to supervisor
pShCd   = 0;                           // Shared code
pOnDeId = 0;                           // On device idle handler
pOnHWId = 0;                           // On hardware idle handler
pOnRTS  = 0;                           // On RTS handler
pOnInit = 0;                           // On init handler
pPropsD = 0;                           // Properties code
pStateD = 0;                           // State code
devTyp  = 'U';                         // Undefined
}

//------------------------------------------------------------------------------

DevT_t::~DevT_t()
{
WALKVECTOR(PinT_t *,PinTI_v,i) delete *i;
WALKVECTOR(PinT_t *,PinTO_v,i) delete *i;
if(pPinTSI != 0) delete pPinTSI;
if(pPinTSO != 0) delete pPinTSO;
if(pShCd   != 0) delete pShCd;
if(pOnDeId != 0) delete pOnDeId;
if(pOnHWId != 0) delete pOnHWId;
if(pOnRTS  != 0) delete pOnRTS;
if(pOnInit != 0) delete pOnInit;
if(pPropsD != 0) delete pPropsD;
if(pStateD != 0) delete pStateD;
WALKVECTOR(Meta_t *,Meta_v,i) delete *i;
}

//------------------------------------------------------------------------------

void DevT_t::Dump(unsigned off,FILE * fp)
{
string s = string(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n%sDevT_t++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sNameBase       %s\n",os,FullName().c_str());
fprintf(fp,"%sMe,Parent     %#018lx,%#018lx\n",os,(uint64_t)this,(uint64_t)par);
if (par!=0) fprintf(fp,"%s...%s\n",os,par->FullName().c_str());
fprintf(fp,"%sShared code:   %#018lx\n",os,(uint64_t)pShCd);
if (pShCd!=0) pShCd->Dump(off+2,fp);
fprintf(fp,"%sOnDevice idle    %#018lx\n",os,(uint64_t)pOnDeId);
if (pOnDeId!=0) pOnDeId->Dump(off+2,fp);
fprintf(fp,"%sOnHardware idle  %#018lx\n",os,(uint64_t)pOnHWId);
if (pOnHWId!=0) pOnHWId->Dump(off+2,fp);
fprintf(fp,"%sOnReady to send  %#018lx\n",os,(uint64_t)pOnRTS);
if (pOnRTS!=0) pOnRTS->Dump(off+2,fp);
fprintf(fp,"%sOnInitialisation %#018lx\n",os,(uint64_t)pOnInit);
if (pOnInit!=0) pOnInit->Dump(off+2,fp);
fprintf(fp,"%sProperties declaration %#018lx\n",os,(uint64_t)pPropsD);
if (pPropsD!=0) pPropsD->Dump(off+2,fp);
fprintf(fp,"%sState declaration %#018lx\n",os,(uint64_t)pStateD);
if (pStateD!=0) pStateD->Dump(off+2,fp);
fprintf(fp,"%sdevTyp           %c\n",os,devTyp);
fprintf(fp,"%sPIN HANDLER CODE ...\n",os);
fprintf(fp,"%sSUPERVISOR INPUT PIN TYPE %#018lx\n",os,(uint64_t)pPinTSI);
if (pPinTSI!=0) pPinTSI->Dump(off+2,fp);
fprintf(fp,"%sSUPERVISOR OUTPUT PIN TYPE %#018lx\n",os,(uint64_t)pPinTSO);
if (pPinTSO!=0) pPinTSO->Dump(off+2,fp);
fprintf(fp,"%sINPUT PIN TYPES\n",os);
WALKVECTOR(PinT_t *,PinTI_v,i)(*i)->Dump(off+2,fp);
fprintf(fp,"%sOUTPUT PIN TYPES\n",os);
WALKVECTOR(PinT_t *,PinTO_v,i)(*i)->Dump(off+2,fp);
fprintf(fp,"%sMetadata vector has %lu entries:\n",os,Meta_v.size());
WALKVECTOR(Meta_t *,Meta_v,i) (*i)->Dump(off+2,fp);
NameBase::Dump(off+2,fp);
fprintf(fp,"%sDevT_t--------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

PinT_t * DevT_t::Loc_pintyp(string s,char IO)
// Locate a pintype with name "s". IO == 'I': search inpins, == 'O': ... guess
{
switch (IO) {
  case 'I' : WALKVECTOR(PinT_t *,PinTI_v,i) if ((*i)->Name()==s) return *i;
             return 0;
  case 'O' : WALKVECTOR(PinT_t *,PinTO_v,i) if ((*i)->Name()==s) return *i;
             return 0;
}
return 0;
}

//==============================================================================

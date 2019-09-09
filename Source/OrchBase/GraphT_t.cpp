//------------------------------------------------------------------------------

#include "GraphT_t.h"
#include "OrchBase.h"
#include "Apps_t.h"
#include "DevT_t.h"
#include "SupT_t.h"
#include "PinT_t.h"

//==============================================================================

GraphT_t::GraphT_t(Apps_t * _p,string _s):par(_p),pPropsD(0),TyFlag(false)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
pSup = 0;                              // (Optional) graph-level supervisor
}

//------------------------------------------------------------------------------

GraphT_t::~GraphT_t()
{
WALKVECTOR(DevT_t *,DevT_v,i) delete *i;
WALKVECTOR(MsgT_t *,MsgT_v,i) delete *i;
WALKVECTOR(CFrag *,ShCd_v,i) delete *i;
if (pPropsD!=0) delete pPropsD;
if (pSup!=0)    delete pSup;
WALKVECTOR(Meta_t *,Meta_v,i) delete *i;
}

//------------------------------------------------------------------------------

void GraphT_t::Dump(FILE * fp)
{
fprintf(fp,"GraphT_t++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      0x%#08p,0x%#08p\n",this,par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
fprintf(fp,"Referenced by:\n");
WALKVECTOR(GraphI_t *,GraphI_v,i)fprintf(fp,"%s\n",(*i)->FullName().c_str());
fprintf(fp,"...end referenced by list\n");
fprintf(fp,"Message types:\n");
WALKVECTOR(MsgT_t *,MsgT_v,i)(*i)->Dump(fp);
fprintf(fp,"...end message types vector\n");
fprintf(fp,"Messages:\n");
WALKVECTOR(MsgT_t *,MsgT_v,i)(*i)->Dump(fp);
fprintf(fp,"...end messages\n");
fprintf(fp,"Properties declaration            %#08p\n",pPropsD);
if (pPropsD!=0) pPropsD->Dump(fp);
fprintf(fp,"Device types\n");
WALKVECTOR(DevT_t *,DevT_v,i)(*i)->Dump(fp);
fprintf(fp,"...end device types\n");
fprintf(fp,"Supervisor declaration            %#08p\n",pPropsD);
if (pSup!=0) pSup->Dump(fp);
NameBase::Dump(fp);
fprintf(fp,"GraphT_t------------------------------------\n");
fflush(fp);
}

//------------------------------------------------------------------------------

DevT_t * GraphT_t::FindDev(string s)
// Locate the P_devtyp object held in here with the name "s" (if any)
{
WALKVECTOR(DevT_t *,DevT_v,i) if ((*i)->Name()==s) return *i;
return 0;
}

//------------------------------------------------------------------------------

MsgT_t * GraphT_t::FindMsg(string s)
// Find the message with name "s" (if any)
{
WALKVECTOR(MsgT_t *,MsgT_v,i) if ((*i)->Name()==s) return *i;
return 0;
}

//------------------------------------------------------------------------------

PinT_t * GraphT_t::FindPin(DevT_t * pD,string s)
// Find the named pin in a given device type
{
if (pD==0) return 0;
WALKVECTOR(PinT_t *,pD->PinTI_v,i) if ((*i)->Name()==s) return *i;
WALKVECTOR(PinT_t *,pD->PinTO_v,i) if ((*i)->Name()==s) return *i;
return 0;
}

//==============================================================================




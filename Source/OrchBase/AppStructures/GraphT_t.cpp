//------------------------------------------------------------------------------

#include "GraphT_t.h"
#include "OrchBase.h"
#include "Apps_t.h"
#include "DevT_t.h"
#include "SupT_t.h"
#include "PinT_t.h"
#include "Pglobals.h"
#include "Meta_t.h"
#include "DS_XML.h"
#include "MsgT_t.h"
#include "CFrag.h"

//==============================================================================

const string GraphT_t::dMsgName = string("__default");

//==============================================================================

GraphT_t::GraphT_t(Apps_t * _p,string _s):par(_p),pPropsD(0)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
pSup    = 0;                           // (Optional) graph-level supervisor
pPropsD = 0;                           // Properties
pShCd   = 0;                           // Shared code
}

//------------------------------------------------------------------------------

GraphT_t::~GraphT_t()
{
WALKVECTOR(DevT_t *,DevT_v,i) delete *i;
WALKVECTOR(MsgT_t *,MsgT_v,i) delete *i;
if (pShCd!=0)   delete pShCd;
if (pPropsD!=0) delete pPropsD;
if (pSup!=0)    delete pSup;
WALKVECTOR(Meta_t *,Meta_v,i) delete *i;
}

//------------------------------------------------------------------------------

void GraphT_t::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n%sGraphT_t +++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sNameBase     %s\n",os,FullName().c_str());
fprintf(fp,"%sMe,Parent    %#018lx,%#018lx\n",os,(uint64_t)this,(uint64_t)par);
if (par!=0) fprintf(fp,"%s...%s\n",os,par->FullName().c_str());
fprintf(fp,"%sTypelinked by %lu application graphs:\n",os,GraphI_v.size());
WALKVECTOR(GraphI_t *,GraphI_v,i)
  fprintf(fp,"%s...%s\n",os,(*i)->FullName().c_str());
fprintf(fp,"%sContains %lu device types\n",os,DevT_v.size());
WALKVECTOR(DevT_t *,DevT_v,i)(*i)->Dump(off+2,fp);
fprintf(fp,"%s...end device types vector\n",os);
fprintf(fp,"%sContains %lu message types:\n",os,MsgT_v.size());
WALKVECTOR(MsgT_t *,MsgT_v,i)(*i)->Dump(off+2,fp);
fprintf(fp,"%s...end message types vector\n",os);
fprintf(fp,"%sShared code         %#018lx\n",os,(uint64_t)pShCd);
if (pShCd!=0) pShCd->Dump(off+2,fp);
fprintf(fp,"%sProperties          %#018lx\n",os,(uint64_t)pPropsD);
if (pPropsD!=0) pPropsD->Dump(off+2,fp);
fprintf(fp,"%sSupervisor          %#018lx\n",os,(uint64_t)pSup);
if (pSup!=0) pSup->Dump(off+2,fp);
fprintf(fp,"%sMetadata vector has %lu entries:\n",os,Meta_v.size());
WALKVECTOR(Meta_t *,Meta_v,i) (*i)->Dump(off+2,fp);
fprintf(fp,"%s...end metadata instances\n",os);
NameBase::Dump(off+2,fp);
DefRef::Dump(off+2,fp);
fprintf(fp,"%sGraphT_t -----------------------------------------------\n\n",os);
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
// Find the message with name "s" (if any). If there isn't, return the default
// name (that ought to be [0]).
// If not, return 0, but how far can you take paranoia?
{
WALKVECTOR(MsgT_t *,MsgT_v,i) if ((*i)->Name()==s) return *i;
if (MsgT_v[0]->Name()==dMsgName) return MsgT_v[0];
par->par->Post(997,__FILE__,int2str(__LINE__));
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

//------------------------------------------------------------------------------

void GraphT_t::MsgDefault()
// Create and attach a default message type to the graph type definition.
// This will be linked to any pins that have an undefined messagetype
{
MsgT_t * pM = new MsgT_t(this,dMsgName);
static const string defs = string(
"// Auto-generated default POETS packet format\n"
"uint8_t payload[56];\n"
);
pM->pPropsD = new CFrag(defs);
MsgT_v.push_back(pM);
}

//------------------------------------------------------------------------------

void GraphT_t::MsgLink()
// Link all the pins in this graphType to their message types. If the referenced
// message type does not exist, grab the default message type (which is always
// there)
{
PinT_t * pP;
WALKVECTOR(DevT_t *,DevT_v,i) {        // Walk device types
  WALKVECTOR(PinT_t *,(*i)->PinTI_v,j){// Walk input pins
    (*j)->pMsg = FindMsg((*j)->tyId);  // Find message type
    if ((*j)->pMsg==MsgT_v[0]) MsgLinkW("Input",*j);
    (*j)->pMsg->Ref((*j)->Def());
  }
  WALKVECTOR(PinT_t *,(*i)->PinTO_v,j){// Walk output pins
    (*j)->pMsg = FindMsg((*j)->tyId);  // Find message type
    if ((*j)->pMsg==MsgT_v[0]) MsgLinkW("Output",*j);
    (*j)->pMsg->Ref((*j)->Def());
  }
  pP = (*i)->pPinTSI;                  // Device pin FROM supervisor
  if (pP!=0) {
    pP->pMsg = FindMsg(pP->tyId);
    if (pP->pMsg==MsgT_v[0]) MsgLinkW("Device from supervisor",pP);
    pP->pMsg->Ref(pP->Def());
  }
  pP = (*i)->pPinTSO;                  // Device pin TO supervisor
  if (pP!=0) {
    pP->pMsg = FindMsg(pP->tyId);
    if (pP->pMsg==MsgT_v[0]) MsgLinkW("Device to supervisor",pP);
    pP->pMsg->Ref(pP->Def());
  }
}
if (pSup==0) return;                   // Not necessarily a supervisor
pP = pSup->pPinTSO;                    // Supervisor pin TO devices
if (pP!=0) {
  pP->pMsg = FindMsg(pP->tyId);
  if (pP->pMsg==MsgT_v[0]) MsgLinkW("Supervisor to device",pP);
  pP->pMsg->Ref(pP->Def());
}
pP = pSup->pPinTSI;                    // Supervisor pin FROM devices
if (pP!=0) {
  pP->pMsg = FindMsg(pP->tyId);
  if (pP->pMsg==MsgT_v[0]) MsgLinkW("Device to supervisor",pP);
  pP->pMsg->Ref(pP->Def());
}
}

//------------------------------------------------------------------------------

void GraphT_t::MsgLinkW(string ptype,PinT_t * pP)
//
{
FILE * fp = par->par->fd;        // Tell the user
fprintf(fp,"%s pin %s (defined line %u) packet type %s \n"
           "  not found in %s: default packet format used\n",
           ptype.c_str(),pP->FullName().c_str(),pP->Def(),
           pP->tyId.c_str(),FullName().c_str());
fflush(fp);
}

//==============================================================================

//------------------------------------------------------------------------------

#include <stdio.h>
#include "DS_XML.h"
#include "macros.h"
#include "Apps_t.h"
#include "CFrag.h"
#include "Meta_t.h"
#include "PinT_t.h"
#include "SupT_t.h"
#include "MsgT_t.h"
#include "pathDecode.h"
#include "EdgeI_t.h"
#include "PinI_t.h"
#include "DevI_t.h"
#include "DevT_t.h"
#include "GraphI_t.h"
#include "GraphT_t.h"
#include "Pglobals.h"
#include "OrchBase.h"

//==============================================================================

DS_XML::DS_XML(OrchBase * _par)
{
par  = _par;                           // Parent: OrchBase NOT CmLoad
ecnt = 0;                              // Local error counter
wcnt = 0;                              // Local warning counter
fd   = par->fd;                        // Output stream
                                       // Set up name:enumerated type map
DS_map["CDATA"]            = cCDATA;
DS_map["Code"]             = cCode;
DS_map["DeviceType"]       = cDeviceType;
DS_map["DeviceTypes"]      = cDeviceTypes;
DS_map["ExternalType"]     = cExternalType;
DS_map["GraphInstance"]    = cGraphInstance;
DS_map["Graphs"]           = cGraphs;
DS_map["GraphType"]        = cGraphType;
DS_map["InputPin"]         = cInputPin;
DS_map["MessageType"]      = cMessageType;
DS_map["MessageTypes"]     = cMessageTypes;
DS_map["MetaData"]         = cMetaData;
DS_map["OnCTL"]            = cOnCTL;
DS_map["OnDeviceIdle"]     = cOnDeviceIdle;
DS_map["DevI"]             = cDevI;
DS_map["DeviceInstances"]  = cDeviceInstances;
DS_map["EdgeI"]            = cEdgeI;
DS_map["EdgeInstances"]    = cEdgeInstances;
DS_map["ExtI"]             = cExtI;
DS_map["OnHardwareIdle"]   = cOnHardwareIdle;
DS_map["OnInit"]           = cOnInit;
DS_map["OnReceive"]        = cOnReceive;
DS_map["OnRTCL"]           = cOnRTCL;
DS_map["OnSend"]           = cOnSend;
DS_map["OnStop"]           = cOnStop;
DS_map["OnSupervisorIdle"] = cOnSupervisorIdle;
DS_map["OutputPin"]        = cOutputPin;
DS_map["Properties"]       = cProperties;
DS_map["ReadyToSend"]      = cReadyToSend;
DS_map["SharedCode"]       = cSharedCode;
DS_map["State"]            = cState;
DS_map["SupervisorInPin"]  = cSupervisorInPin;
DS_map["SupervisorOutPin"] = cSupervisorOutPin;
DS_map["SupervisorType"]   = cSupervisorType;
DS_map["XML"]              = cXML;

}

//------------------------------------------------------------------------------

DS_XML::~DS_XML()
{
}

//------------------------------------------------------------------------------

void DS_XML::_Apps_t(xnode * pn)
// XML::Graphs
{
string aname = pn->FindAttr("appname");// Get the app name
Apps_t * pAP = new Apps_t(par,aname);  // Uniquely knits itself into skyhook
pAP->Def(pn->lin);                     // Defined on line...
pAP->filename = pn->par->ename;        // Filename is root element name
pAP->fl = fd;                          // Output channel

WALKVECTOR(xnode *,pn->vnode,i) {
  (*i)->col = DS_map[(*i)->ename];
  switch ((*i)->col) {
    case cGraphType     : { GraphT_t * pGT = _GraphT_t(pAP,*i);
                            pAP->GraphT_v.push_back(pGT);
                            break; }
    case cGraphInstance : { GraphI_t * pGI = _GraphI_t(pAP,*i);
                            pAP->GraphI_v.push_back(pGI);
                            break; }
    default             : par->Post(998,__FILE__,int2str(__LINE__),(*i)->ename);
  }
}
}

//------------------------------------------------------------------------------

CFrag * DS_XML::_CFrag(xnode * pn)
// XML everything that resolves to a fragment of C.
// Here's where we decorate the C with comments that allow the user to connect
// the cross-compiler output with the source XML.
// The cfrag itself is not held in cfrag - far too simple - it's in a child node
// called CDATA.
{
string Cstr;
xnode * pC = pn->FindChild("CDATA");   // Go fetch CDATA
if (pC!=0) Cstr = pC->cfrag;           // It may legitimately not be there
                                       // Empty? then bleat
if (Cstr.empty()) Cstr = "// CFrag " + pn->FullName() + " empty\n";
                                       // Decorate with line number
Cstr = "// Line " + int2str(pn->lin) + "\n" + Cstr;
return new CFrag(Cstr);
}

//------------------------------------------------------------------------------

DevI_t * DS_XML::_DevI_t(GraphI_t * pGI,xnode * pn)
// XML::DevI
// Device instantiation. This is not as simple as everything else, because it
// might already have been defined (by accident) and we need to check for this.
// OR it might already have been referenced, but not yet defined and this is it
// OR it might be the first we've seen of it.
// We can't check for undefined references until we leave the XML::GraphInstance
// handler, because devices and edges can be defined in any order
// We hand out the device itself so that the external instance code can tweak
// the device type. Usually it's ignored.
{

string dname = pn->FindAttr("id");     // Device id
                                       // Is it already in the graph?
map<string,unsigned>::iterator i = pGI->Dmap.find(dname);
if (i != pGI->Dmap.end()) {            // Yes
  unsigned k = (*i).second;            // Get the key
  DevI_t * pD = *(pGI->G.FindNode(k)); // Get the device
  if (pD->Def()!=0) {                  // Already defined?
    fprintf(fd,"(%3u,-) W%3u: Device %s already defined on line %u\n",
               pn->lin,++wcnt,dname.c_str(),pD->Def());     // Cockup. Bail.
    fflush(fd);
    return pD;
  }
                                       // Already there, but not defined
  pD->tyId = pn->FindAttr("type");     // Device type
  pD->Def(pn->lin);                    // Save definition line
  pD->devTyp = 'D';                    // It's an internal device
  return pD;
}
                                       // This is the first we've seen of it
DevI_t * pD = new DevI_t(pGI,dname);   // New device instance
pD->tyId = pn->FindAttr("type");       // Device type
WALKVECTOR(xnode *,pn->vnode,i) {      // There should be only one (type of)....
  (*i)->rTyp() = DS_map[(*i)->ename];
  switch ((*i)->rTyp()) {
    case cProperties     : pD->pPropsI = _CFrag(*i);             break;
    case cState          : pD->pStateI = _CFrag(*i);             break;
    case cMetaData       : pD->Meta_v.push_back(_Meta(*i));      break;
    default              : par->Post(998,__FILE__,int2str(__LINE__),(*i)->ename);
  }
}
pD->Def(pn->lin);                      // Save definition line
pD->devTyp = 'D';                      // Internal device
pD->Key = UniU();                      // Cosmically unique key
pGI->Dmap[dname] = pD->Key;            // Store the *inverse* key
pGI->G.InsertNode(pD->Key,pD);         // Shove it in the graph
return pD;
}

//------------------------------------------------------------------------------

void DS_XML::_DevI_ts(GraphI_t * pGI,xnode * pn)
// XML::DeviceInstances
{
WALKVECTOR(xnode *,pn->vnode,i) {
  (*i)->col = DS_map[(*i)->ename];
  switch ((*i)->col) {
    case cDevI : _DevI_t(pGI,*i);                  break;
    case cExtI : _ExtI_t(pGI,*i);                  break;
    default    : par->Post(998,__FILE__,int2str(__LINE__),(*i)->ename);
  }
}
}

//------------------------------------------------------------------------------

DevT_t * DS_XML::_DevT_t(GraphT_t * pGT,xnode * pn)
// XML::DeviceType
{
string dname = pn->FindAttr("id");
DevT_t * pD = new DevT_t(pGT,dname);
pD->Def(pn->lin);
WALKVECTOR(xnode *,pn->vnode,i) {
  PinT_t * pP;
  (*i)->col = DS_map[(*i)->ename];
  switch ((*i)->col) {
    case cProperties       : pD->pPropsD = _CFrag(*i);               break;
    case cState            : pD->pStateD = _CFrag(*i);               break;
    case cSharedCode       : pD->pShCd   = _CFrag(*i);               break;
    case cSupervisorOutPin : pD->pPinTSO = _PinT_t(pD,*i);           break;
    case cSupervisorInPin  : pD->pPinTSI = _PinT_t(pD,*i);           break;
    case cInputPin         : pP = _PinT_t(pD,*i);
                             if (pP!=0) {   pD->PinTI_v.push_back(pP);
                                            pP->Idx = (pD->PinTO_v.size()-1);
                                                                   } break;
    case cOutputPin        : pP = _PinT_t(pD,*i);
                             if (pP!=0) {   pD->PinTO_v.push_back(pP);
                                            pP->Idx = (pD->PinTO_v.size()-1);
                                                                   } break;
    case cReadyToSend      : pD->pOnRTS  = _CFrag(*i);               break;
    case cOnInit           : pD->pOnInit = _CFrag(*i);               break;
    case cOnHardwareIdle   : pD->pOnHWId = _CFrag(*i);               break;
    case cOnDeviceIdle     : pD->pOnDeId = _CFrag(*i);               break;
    case cMetaData         : pD->Meta_v.push_back(_Meta(*i));        break;
    default                : par->Post(998,__FILE__,int2str(__LINE__),
                                       (*i)->ename);
  }
}
return pD;
}

//------------------------------------------------------------------------------

void DS_XML::_DevT_ts(GraphT_t * pGT,xnode * pn)
// XML::DeviceTypes
{
WALKVECTOR(xnode *,pn->vnode,i) {
  (*i)->rTyp() = DS_map[(*i)->ename];
  switch ((*i)->rTyp()) {
    case cExternalType   : pGT->DevT_v.push_back(_DevT_t(pGT,*i));
                           pGT->DevT_v.back()->devTyp = 'X';       break;
    case cDeviceType     : pGT->DevT_v.push_back(_DevT_t(pGT,*i));
                           pGT->DevT_v.back()->devTyp = 'D';       break;
    case cSupervisorType : pGT->pSup = _SupT_t(pGT,*i);            break;
    default              : par->Post(998,__FILE__,int2str(__LINE__),(*i)->ename);
  }
}
}

//------------------------------------------------------------------------------

void DS_XML::_EdgeI_t(GraphI_t * pGI,xnode * pn)
// XML::EdgeI
// Function to knit an XML edge into the datastructure.
// Supervisors: just ignore them, and edges going to and from them.
{
unsigned lin = pn->lin;                // Defining/referencing file line
string pname = pn->FindAttr("path");
pathDecode PD;                         // We *know* path inner grammar is OK....
vector<string> path_v = PD(pname);     // ....trust, but verify
if (!path_v[0].empty()) par->Post(931,path_v[0]);
                                       // If the user has connected an undefined
                                       // device to [S], they get away with it
if(path_v[1].empty()) return;          // To [S]
if(path_v[3].empty()) return;          // From [S]

                                       // One way or another, get 'to' device
DevI_t * pDto = pGI->GetDevice(path_v[1]);
pDto->Ref(lin);                        // Increment the references
PinI_t * pPto = pDto->GetPin(path_v[2]);
pPto->Def(lin);
pPto->Ref(lin);
                                       // One way or another, get 'fr' device
DevI_t * pDfr = pGI->GetDevice(path_v[3]);
pDfr->Ref(lin);                        // Increment the references
PinI_t * pPfr = pDfr->GetPin(path_v[4]);
pPfr->Def(lin);
pPfr->Ref(lin);

unsigned kE = UniU();                  // Unique edge key
EdgeI_t * pE = new EdgeI_t(pGI,string());
pE->Def(lin);
pE->Ref(lin);
pE->Key = kE;                          // Store the key inside its data
pE->Idx = (pPto->Key_v.size() - 1);    // Index of edge in the to pin's Key_v
                                       // Insert into instance graph
                                       
WALKVECTOR(xnode *,pn->vnode,i) {
  (*i)->rTyp() = DS_map[(*i)->ename];
  switch ((*i)->rTyp()) {                                       
    case cProperties     : pE->pPropsI = _CFrag(*i);             break;
    case cState          : pE->pStateI = _CFrag(*i);             break;
    default              : par->Post(998,__FILE__,int2str(__LINE__),(*i)->ename);
  }
}

pGI->G.InsertArc(kE,pDto->Key,pDfr->Key,pE,
                 pPto->Key_v.back(),pPto,pPfr->Key_v.back(),pPfr);
}

//------------------------------------------------------------------------------

void DS_XML::_EdgeI_ts(GraphI_t * pGI,xnode * pn)
// XML::EdgeInstances
{
WALKVECTOR(xnode *,pn->vnode,i) {
  (*i)->col = DS_map[(*i)->ename];
  switch ((*i)->col) {
    case cEdgeI : _EdgeI_t(pGI,*i);                break;
    default     : par->Post(998,__FILE__,int2str(__LINE__),(*i)->ename);
  }
}
}

//------------------------------------------------------------------------------

void DS_XML::_ExtI_t(GraphI_t * pGI,xnode * pn)
// XML::ExtI
// Call the device instantiation code and overwrite the type flag
{
DevI_t * pD = _DevI_t(pGI,pn);
pD->devTyp = 'X';

WALKVECTOR(xnode *,pn->vnode,i) {
  (*i)->rTyp() = DS_map[(*i)->ename];
  switch ((*i)->rTyp()) {
    case cProperties     : pD->pPropsI = _CFrag(*i);             break;
    default              : par->Post(998,__FILE__,int2str(__LINE__),(*i)->ename);
  }
}
}

//------------------------------------------------------------------------------

GraphI_t * DS_XML::_GraphI_t(Apps_t * pAP,xnode * pn)
// XML::GraphInstance
{
string gname = pn->FindAttr("id");
GraphI_t * pGI = new GraphI_t(pAP,gname);
pGI->Def(pn->lin);
pGI->tyId = pGI->tyId2 = pn->FindAttr("graphTypeId");
WALKVECTOR(xnode *,pn->vnode,i) {
  (*i)->rTyp() = DS_map[(*i)->ename];
  switch ((*i)->rTyp()) {
    case cMetaData        : pGI->Meta_v.push_back(_Meta(*i));        break;
    case cProperties      : pGI->pPropsI = _CFrag(*i);               break;
    case cDeviceInstances : _DevI_ts(pGI,*i);                        break;
    case cEdgeInstances   : _EdgeI_ts(pGI,*i);                       break;
    default               : par->Post(998,__FILE__,int2str(__LINE__),(*i)->ename);
  }
}
                                       // Report undefined devices;
                                       // Minor integrity check;
                                       // Dismantle a bit of scaffold
pGI->UndefDevs();
return pGI;
}

//------------------------------------------------------------------------------

GraphT_t * DS_XML::_GraphT_t(Apps_t * pAP,xnode * pn)
// XML::GraphType
{
string gname = pn->FindAttr("id");     // Get the GraphType name
GraphT_t * pGT = new GraphT_t(pAP,gname); // Instantiate it
pGT->Def(pn->lin);
pGT->MsgDefault();                     // Insert default message type

WALKVECTOR(xnode *,pn->vnode,i) {
  (*i)->rTyp() = DS_map[(*i)->ename];
  switch ((*i)->rTyp()) {
    case cDeviceTypes    : _DevT_ts(pGT,*i);                    break;
    case cMessageTypes   : _MsgT_ts(pGT,*i);                    break;
    case cSharedCode     : pGT->pShCd = _CFrag(*i);             break;
    case cProperties     : pGT->pPropsD = _CFrag(*i);           break;
    case cMetaData       : pGT->Meta_v.push_back(_Meta(*i));    break;
    case cSupervisorType : pGT->pSup = _SupT_t(pGT,(*i));       break;
    default              : par->Post(998,__FILE__,int2str(__LINE__),(*i)->ename);
  }
}
pGT->MsgLink();                        // MessageLink all the pins (cannot fail)
return pGT;
}

//------------------------------------------------------------------------------

Meta_t * DS_XML::_Meta(xnode * pn)
// XML:: MetaData
{
return new Meta_t(pn->attr);
}

//------------------------------------------------------------------------------

MsgT_t * DS_XML::_MsgT_t(GraphT_t * pGT,xnode * pn)
// XML::MessageType
{
string mname = pn->FindAttr("id");
MsgT_t * pM = new MsgT_t(pGT,mname);
pM->Def(pn->lin);
pM->pPropsD = _CFrag(pn);
return pM;
}

//------------------------------------------------------------------------------

void DS_XML::_MsgT_ts(GraphT_t * pGT,xnode * pn)
// XML::MessageTypes
{
WALKVECTOR(xnode *,pn->vnode,i) {
  (*i)->col = DS_map[(*i)->ename];
  switch ((*i)->col) {
    case cMessageType : pGT->MsgT_v.push_back(_MsgT_t(pGT,*i)); break;
    default           : par->Post(998,__FILE__,int2str(__LINE__),(*i)->ename);
  }
}
}

//------------------------------------------------------------------------------

PinT_t * DS_XML::_PinT_t(DevT_t * pD,xnode * pn)
// XML::PinType
{
string pname = pn->FindAttr("name");
unsigned line = pn->lin;               // Go get file location
PinT_t * pP;
pP  = pD->Loc_pintyp(pname,'I');       // Search input list for prior definition
if (pP==0) pP = pD->Loc_pintyp(pname,'O'); // Output list?
if (pP!=0) {                           // Already defined?
  fprintf(fd,"(%3u,-) W%3u: Pin type %s already exists - ignored\n",
          line,++wcnt,pname.c_str());
  fflush(fd);
  return 0;
}
pP  = new PinT_t(pD,pname);            // Good to go - create new one
pP->Def(line);                         // Define it
pP->tyId     = pn->FindAttr("messageTypeId");
WALKVECTOR(xnode *,pn->vnode,i) {
  (*i)->col = DS_map[(*i)->ename];
  switch ((*i)->col) {
    case cOnSend     : pP->pHandl  = _CFrag(*i);             break;
    case cOnReceive  : pP->pHandl  = _CFrag(*i);             break;
    case cProperties : pP->pPropsD = _CFrag(*i);             break;
    case cState      : pP->pStateD = _CFrag(*i);             break;
    default          : par->Post(998,__FILE__,int2str(__LINE__),(*i)->ename);
  }
}
return pP;
}

//------------------------------------------------------------------------------

SupT_t * DS_XML::_SupT_t(GraphT_t * pGT,xnode * pn)
// XML::SupervisorType
// <Code> is stored in Device::pShCd
// OnSupervisorIdle is stored in Device::pOnDeId
// The *supervisor* input and output pins are stored in Device::
// THERE APPEARS TO BE NO "OnPkt" HANDLER???? Check w/ MLV   19/6/20
// Everythng else is stored in the SupT_t
{
string sname = pn->FindAttr("id");
SupT_t * pS = new SupT_t(pGT,sname);
pS->Def(pn->lin);
WALKVECTOR(xnode *,pn->vnode,i) {
  (*i)->rTyp() = DS_map[(*i)->ename];
  switch ((*i)->rTyp()) {
    case cCode             : pS->pShCd    = _CFrag(*i);               break;
    case cSupervisorOutPin : pS->pPinTSO = _PinT_t(pS,*i);            break;
    case cSupervisorInPin  : pS->pPinTSI = _PinT_t(pS,*i);            break;
    case cOnSupervisorIdle : pS->pOnDeId  = _CFrag(*i);               break;
    case cOnRTCL           : pS->pOnRTCL  = _CFrag(*i);               break;
    case cOnStop           : pS->pOnStop  = _CFrag(*i);               break;
    case cOnCTL            : pS->pOnCTL   = _CFrag(*i);               break;
    case cMetaData         : pS->Meta_v.push_back(_Meta(*i));         break;
    default                : par->Post(998,__FILE__,int2str(__LINE__),
                                       (*i)->ename);
  }
}
return pS;
}

//------------------------------------------------------------------------------

void DS_XML::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%s\nDS_XML::Dump +++++++++++++++++++++++++++++++++++++++++\n\n",os);
fprintf(fp,"%sThis object contains no interesting state data at all\n",os);
fprintf(fp,"%sDS_XML::Dump -------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void DS_XML::ErrCnt(unsigned & re,unsigned & rw)
// Return error and warning counts
{
re = ecnt;
rw = wcnt;
}

//------------------------------------------------------------------------------

void DS_XML::PBuild(xnode * pn)
// Expecting to be handed a xml node; build whatever Graphs may be defined
{
ecnt = wcnt = 0;                       // Reset diagnostic counters
WALKVECTOR(xnode *,pn->vnode,i) {      // Walk the children
  (*i)->col = DS_map[(*i)->ename];
  if ((*i)->col!=cGraphs) par->Post(998,__FILE__,int2str(__LINE__),(*i)->ename);
  else _Apps_t(*i);
}

}

//------------------------------------------------------------------------------

void DS_XML::Show(FILE * fp)
{
fprintf(fp,"The datastructure-from-XML object has no "
           "internal state of interest.\n");
}

//==============================================================================

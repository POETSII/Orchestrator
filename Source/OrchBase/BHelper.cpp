//---------------------------------------------------------------------------

#include "BHelper.h"
#include "Apps_t.h"
#include "A_builder.h"
#include "DevT_t.h"
#include "SupT_t.h"
#include "PinI_t.h"
#include "DevI_t.h"
#include "FileName.h"
#include "Meta_t.h"
#include <map>
#include <algorithm>                   // For ::tolower
#include "macros.h"
using namespace std;

//==============================================================================
/* The guts of the functionality here is derived off xmlParser. We walk the
input file - again, and we build a stack that mirrors our depth in the parse
tree. The stack contains structs called 'contexts', which are a bunch of
pointers that are 'relevant' to each point, so we can construct the main
datastructure. A lot of the time, some (most?) of the context fields are
irrelevant, but we just ignore the ones we don't want.
*/
//==============================================================================

BHelper::BHelper(string fn,OrchBase * p) : xmlParser()
// The usual, plus we set up the map that lists the XML keywords, and associates
// each one with an enumerated type variable.
{
filename = fn;
par = p;                               // My parent

Potype_map.Add(P_0     ,string("NoTag"));
Potype_map.Add(Po_CdP  ,string("Properties"));
Potype_map.Add(Po_CdS  ,string("State"));
Potype_map.Add(Po_CdSh ,string("SharedCode"));
Potype_map.Add(Po_DeI  ,string("DevI"));
Potype_map.Add(Po_DeIs ,string("DeviceInstances"));
Potype_map.Add(Po_DeT  ,string("DeviceType"));
Potype_map.Add(Po_DeTs ,string("DeviceTypes"));
Potype_map.Add(Po_EgI  ,string("EdgeI"));
Potype_map.Add(Po_EgIs ,string("EdgeInstances"));
Potype_map.Add(Po_ExI  ,string("ExtI"));
Potype_map.Add(Po_ExT  ,string("ExternalType"));
Potype_map.Add(Po_FILE ,string("FILE"));
Potype_map.Add(Po_GrI  ,string("GraphInstance"));
Potype_map.Add(Po_GrS  ,string("Graphs"));
Potype_map.Add(Po_GrT  ,string("GraphType"));
Potype_map.Add(Po_Meta ,string("MetaData"));
Potype_map.Add(Po_Msg  ,string("Message"));
Potype_map.Add(Po_MsT  ,string("MessageType"));
Potype_map.Add(Po_MsTs ,string("MessageTypes"));
Potype_map.Add(Po_OCTL ,string("OnControl"));
Potype_map.Add(Po_OHW  ,string("OnHardware"));
Potype_map.Add(Po_OIDL ,string("OnDeviceIdle"));
Potype_map.Add(Po_ORTS ,string("ReadyToSend"));
Potype_map.Add(Po_OREC ,string("OnReceive"));
Potype_map.Add(Po_OSND ,string("OnSend"));
Potype_map.Add(Po_PnIn ,string("InputPin"));
Potype_map.Add(Po_PnOu ,string("OutputPin"));
Potype_map.Add(Po_SuT  ,string("SupervisorType"));
Potype_map.Add(Po_XXXX ,string("XXXX"));
}

//------------------------------------------------------------------------------

BHelper::~BHelper()
//
{
}

//------------------------------------------------------------------------------

unsigned BHelper::BuildApp()
// Here we actually kick off the build process; we pull the elements out of the
// underlying XML, and build a stack to indicate where in the XML hierarchy we
// are
{
                                       // First, we sort out the application
                                       // name from the file name. The filename
                                       // was handed into the constructcor
                                       // Can we open the thing?
FILE * fi = fopen(filename.c_str(),"r");
if (fi==0){                            // No - so bugger off anyway
  fprintf(fo,"Cannot open %s\n",filename.c_str());
  return par->Post(238,filename);
}                                      // Here if we can at least open it

assert(context_v.empty());             // Context stack must be empty
pTos = new context(this);              // New initial frame
context_v.push_back(pTos);             // Shove the first frame in
context_v.back()->tag = Po_FILE;       // It's a file
Parse(fi);                             // Kick it
unsigned ecnt = ExposeErrs();
delete context_v.back();               // Kill the final "file" frame
context_v.pop_back();
assert(context_v.empty());
fclose(fi);
return ecnt;

// The data structure is now all there, but in two unconnected parts:
// instance graph(s) and type tree(s). For this to count as success, we have to
// (successfully) type-link *every* task.

}

//------------------------------------------------------------------------------

void BHelper::Dump(FILE * fp)
{
fprintf(fp,"BHelper+++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"Context stack size: %u\n",context_v.size());
for(unsigned i=0;i<context_v.size();i++) {
  fprintf(fp,"Frame: %u\n",i);
  context_v[i]->Dump();
}
fprintf(fp,"BHelper-----------------------------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

bool BHelper::CDATA(const void * p,const unsigned & type,const string & s)
// Found a chunk of C - where it goes depends on the context
// *Overwrites are silently ignored* (Validator should have caught them)
// Everything returns true because it's a fast way of getting out of the routine
// The return value was used in the Validator, but we wouldn't be here if that
// hadn't already blessed everything.
{
if (pTos->broken) return true;
pTos->pCFrag = new CFrag(s);           // Space for the new bit of C
                                       // Last two levels of XML:
Potype ptag = pTos->ptag;              // Parent context
Potype tag = pTos->tag;                // Current
CFrag ** ppC;                          // Pointer to where we may/may not put it

switch (ptag) {
  case Po_GrT  : switch (tag) {        // Graph type
                   case Po_CdSh : pTos->pGraphT->ShCd_v.push_back(pTos->pCFrag);
                                  return true;
                   case Po_CdP  : return CDATAfix(&(pTos->pGraphT->pPropsD));
                   case Po_MsT  : return CDATAfix(&(pTos->pMsgT->pPropsD));
                   default      : return true;
                 }
  case Po_DeT  : switch (tag) {        // Device type
                   case Po_CdSh : pTos->pDevT->ShCd_v.push_back(pTos->pCFrag);
                                  return true;
                   case Po_CdP  : return CDATAfix(&(pTos->pDevT->pPropsD));
                   case Po_CdS  : return CDATAfix(&(pTos->pDevT->pStateD));
                   case Po_ORTS : return CDATAfix(&(pTos->pDevT->pOnRTS));
                   case Po_OCTL : return CDATAfix(&(pTos->pDevT->pOnCtl));
                   case Po_OHW  : return CDATAfix(&(pTos->pDevT->pOnHW));
                   case Po_OIDL : return CDATAfix(&(pTos->pDevT->pOnIdle));
                   default      : return true;
                 }
  case Po_ExT  : switch (tag) {        // External type
                   case Po_CdP  : return CDATAfix(&(pTos->pDevT->pPropsD));
                   default      : return true;
                 }
  case Po_PnIn :                       // Input pin type
  case Po_PnOu : switch (tag) {        // Output pin type
                   case Po_OSND :
                   case Po_OREC : return CDATAfix(&(pTos->pPinT->pHandl));
                   case Po_CdP  : return CDATAfix(&(pTos->pPinT->pPropsD));
                   case Po_CdS  : return CDATAfix(&(pTos->pPinT->pStateD));
                   default      : return true;
                 }
  case Po_MsT  : switch (tag) {        // Message type
                   case Po_Msg  : return CDATAfix(&(pTos->pMsgT->pPropsD));
                   default      : return true;
                 }
  case Po_SuT  : switch (tag) {        // Supervisor type
                   case Po_CdSh : pTos->pSupT->ShCd_v.push_back(pTos->pCFrag);
                                  return true;
                   case Po_CdP  : return CDATAfix(&(pTos->pSupT->pPropsD));
                   case Po_CdS  : return CDATAfix(&(pTos->pSupT->pStateD));
                   case Po_OPKT : return CDATAfix(&(pTos->pSupT->pOnPkt));
                   case Po_ORTCL: return CDATAfix(&(pTos->pSupT->pOnRTCL));
                   case Po_OSTOP: return CDATAfix(&(pTos->pSupT->pOnStop));
                   case Po_OCTL : return CDATAfix(&(pTos->pSupT->pOnCtl));
                   default      : return true;
                 }
  default      : par->Post(211,Potype_map[tag],Lx.GetLCs());      // NFI
                 delete pTos->pCFrag;                 // But I don't want it
                 pTos->pCFrag = 0;
                 pTos->broken = true;                 // Stop further loads
                 return true;
}
//gpar->Tr.Push("CDATA");
//fprintf(fo,"\nBHelper::CDATA derived from...");
//return xmlParser::CDATA(p,type,s);
}

//------------------------------------------------------------------------------

bool BHelper::CDATAfix(CFrag ** ppC)
// Here we look at the point in the main datastructure where we want to store
// the CFrag, and see if it's already defined.
// If it's empty, we store the new one. If it's already occupied, we chuck away
// - silently - the incoming new version.
{
if (*ppC==0) *ppC = pTos->pCFrag;
else delete pTos->pCFrag;
return true;
}

//------------------------------------------------------------------------------

bool BHelper::Comments(const void * p,const string & s)
{
//gpar->Tr.Push("Comment");
//fprintf(fo,"\nBHelper::Comments derived from...");
//return xmlParser::Comments(p,s);
return true;
}

//------------------------------------------------------------------------------

bool BHelper::EndDocument(const void * p)
// The final frame of the context stack is NOT popped here, but in the caller
{
//gpar->Tr.Push("End_document");
//WALKMAP(string,Apps_t *,Apps_t::Apps_m,i) (*i).second->Dump();
//Dump();
return true;
}

//------------------------------------------------------------------------------

bool BHelper::EndElement(const void * p,const string & s)
// Make the parent of the current node the new start point
{
//fprintf(fo,"\nBHelper::EndElement derived from...");
if (!context_v.empty()) {
  delete context_v.back();
  context_v.pop_back();
}
//gpar->Tr.Push("Context_stack_size",context_v.size());
//return xmlParser::EndElement(p,s);
return true;
}

//------------------------------------------------------------------------------

bool BHelper::Error(const void * p,const unsigned & e,const unsigned & r,
                        const unsigned & c,const string & s)
{
//gpar->Tr.Push("BHelper_error");
fprintf(fo,"\nBHelper::Error derived from...");
return xmlParser::Error(p,e,r,c,s);
}

//------------------------------------------------------------------------------

void BHelper::IntegGraph(GraphI_t * pG)
// All the graphs in the file should have been type-linked. Here we scan
// everything, and look for
// 1. def/ref mismatches in the graph
// 2. Undefined types referenced
// 3. Undefined fields
{
string Gs0 = "Graph";
string Ds0 = "Device";
string Ps0 = "Pin";
                                       // The graph itself
par->Post(213,pG->par->Name(),pG->par->filename,pG->Name());
if (pG->tyId.empty())  par->Post(214,Gs0,pG->Name());
if (pG->pT==0)         par->Post(215,pG->Name(),Gs0,pG->tyId);
if (pG->pPropsI==0)    par->Post(216,Gs0,pG->Name()," "," ","properties");

                                      // Walk the graph device instances
WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,P_link *,unsigned,PinI_t *,
                  pG->G,i) {
  DevI_t * pD = pG->G.NodeData(i);    // Device instances
  string Ds = pD->Name();             // Device name
  if (pD->tyId.empty())   par->Post(214,Ds0,Ds);
  if (pD->pT==0)          par->Post(215,pG->Name(),Ds0,pD->tyId);
  if (pD->pPropsI==0)     par->Post(216,Ds0,Ds," "," ","properties");
  if (pD->pStateI==0)     par->Post(216,Ds0,Ds," "," ","state");
  if (pD->Kdev==0)        par->Post(216,Ds0,Ds," "," ","graph key");
  if (pD->Def()==0)       par->Post(217,Ds0,Ds,"defined");
  if (pD->Ref()==0)       par->Post(217,Ds0,Ds,"referenced");
  if (pG->G.SizeInPins(pD->Kdev)==0) par->Post(218,Ds0,Ds,"packet input pins");
  if (pG->G.SizeOutPins(pD->Kdev)==0)par->Post(218,Ds0,Ds,"packet output pins");

                                       // Walk input pins on device
  WALKPDIGRAPHINPINS(unsigned,DevI_t *,unsigned,P_link *,unsigned,PinI_t *,
                     pG->G,pG->G.NodeKey(i),j){
    PinI_t * pP = pG->G.PinData(j);    // Get the pin from the iterator
    string Ps = pP->Name();            // Pin name
    if (pP->pPropsI==0)   par->Post(216,Ds0,Ds,Ps0,Ps,"properties");
    if (pP->pStateI==0)   par->Post(216,Ds0,Ds,Ps0,Ps,"state");
    if (pP->pT==0)        par->Post(216,Ds0,Ds,Ps0,"","type");
  }
                                       // And again for the output pins
  WALKPDIGRAPHOUTPINS(unsigned,DevI_t *,unsigned,P_link *,unsigned,PinI_t *,
                     pG->G,pG->G.NodeKey(i),j){
    PinI_t * pP = pG->G.PinData(j);
    string Ps = pP->Name();            // Pin name
    if (pP->pPropsI==0)   par->Post(216,Ds0,Ds,Ps0,Ps,"properties");
    if (pP->pStateI==0)   par->Post(216,Ds0,Ds,Ps0,Ps,"state");
    if (pP->pT==0)        par->Post(216,Ds0,Ds,Ps0," ","type");
  }
}

// Here endeth the graph typelink check


}
 //------------------------------------------------------------------------------

void BHelper::IntegType(GraphT_t * pT)
// All the graphs in the file should have been type-linked. Here we the type
// tree
{
string Gs0 = "GraphType";
string Ds0 = "DeviceType";
string Ps0 = "PinType";
string Ms0 = "Messages";
string Ts = pT->Name();
                                       // The tree itself
par->Post(230,pT->Name(),pT->par->Name());
par->Post(231);
if (pT->DevT_v.empty())     par->Post(218,Gs0,Ts,"device types");
if (pT->MsgT_v.empty())     par->Post(218,Gs0,Ts,"message types");
if (pT->GraphI_v.empty())   par->Post(218,Gs0,Ts,"linked graph instances");
if (pT->ShCd_v.empty())     par->Post(218,Gs0,Ts,"shared code");
if (pT->pPropsD==0)         par->Post(218,Gs0,Ts,"properties");

                                       // Walk the device types
WALKVECTOR(DevT_t *,pT->DevT_v,i) {
  string Dts = (*i)->Name();           // Device type name
  if ((*i)->PinTI_v.empty())      par->Post(218,Ds0,Dts,"input pins");
  if ((*i)->PinTO_v.empty())      par->Post(218,Ds0,Dts,"output pins");
  if ((*i)->ShCd_v.empty())       par->Post(218,Ds0,Dts,"shared code");
  if ((*i)->pOnIdle==0)           par->Post(218,Ds0,Dts,"OnIdle handler");
  if ((*i)->pOnRTS==0)            par->Post(218,Ds0,Dts,"OnRTS handler");
  if ((*i)->pOnCtl==0)            par->Post(218,Ds0,Dts,"OnCtl handler");
//  if ((*i)->pPropsI==0)           par->Post(218,Ds0,Dts,"properties initialiser");
//  if ((*i)->pStateI==0)           par->Post(218,Ds0,Dts,"state initialiser");
  if ((*i)->pPropsD==0)           par->Post(218,Ds0,Dts,"properties");
  if ((*i)->pStateD==0)           par->Post(218,Ds0,Dts,"state");
  if ((*i)->Def()==0)             par->Post(217,Ds0,Dts,"defined");
  if ((*i)->Ref()==0)             par->Post(217,Ds0,Dts,"referenced");
                                       // Walk input pin types
  WALKVECTOR(PinT_t *,(*i)->PinTI_v,j) {
    string Pts = (*j)->Name();         // Pin type name
//    if ((*j)->idx > (*i)->PinTI_v.size())gpar->Post(232,Ps0,Pts,"pin index");
//    if ((*i)->P_pintypIv[(*j)->idx] != (*i))gpar->Post(233,Ps0,Pts,"pin index");
//    if ((*i)->pPropsI==0)         par->Post(218,Ps0,Pts,"properties initialiser");
//    if ((*i)->pStateI==0)         par->Post(218,Ps0,Pts,"state initialiser");
    if ((*i)->pPropsD==0)         par->Post(218,Ps0,Pts,"properties");
    if ((*i)->pStateD==0)         par->Post(218,Ps0,Pts,"state");
    if ((*i)->Def()==0)           par->Post(217,Ps0,Pts,"defined");
    if ((*i)->Ref()==0)           par->Post(217,Ps0,Pts,"referenced");
  }
                                       // And again for the output pin types
  WALKVECTOR(PinT_t *,(*i)->PinTO_v,j) {
    string Pts = (*j)->Name();         // Pin type name
//    if ((*j)->idx > (*i)->PinTO_v.size())gpar->Post(232,Ps0,Pts,"pin index");
//    if ((*i)->P_pintypOv[(*j)->idx] != (*i))gpar->Post(233,Ps0,Pts,"pin index");
//    if ((*i)->pPropsI==0)         par->Post(218,Ps0,Pts,"properties initialiser");
//    if ((*i)->pStateI==0)         par->Post(218,Ps0,Pts,"state initialiser");
    if ((*i)->pPropsD==0)         par->Post(218,Ps0,Pts,"properties");
    if ((*i)->pStateD==0)         par->Post(218,Ps0,Pts,"state");
    if ((*i)->Def()==0)           par->Post(217,Ps0,Pts,"defined");
    if ((*i)->Ref()==0)           par->Post(217,Ps0,Pts,"referenced");
  }
}
                                       // And the messages
WALKVECTOR(MsgT_t *,pT->MsgT_v,i) {
  string Ms = (*i)->Name();            // Message name
  if ((*i)->pPropsD==0)           par->Post(218,Ds0," ",Ms0,Ms,"properties");
}

// Here endeth the type tree typelink check


}

//------------------------------------------------------------------------------

void BHelper::MkAtMp(vector<pair<string,string> > & vps)
// Turn the vector of string pairs into the attribute map
{
attrMap.clear();
for(vector<pair<string,string> >::iterator i=vps.begin();i!=vps.end();i++)
  attrMap[(*i).first] = (*i).second;
}

//------------------------------------------------------------------------------

void BHelper::New_Po_CdP()
// Properties code: where it goes depends on the parent, but there's nothing to
// do here because the element contains nothing but the CFrag
{
}

//------------------------------------------------------------------------------

void BHelper::New_Po_CdS()
// State code : where it goes depends on the parent, but there's nothing to
// do here because the element contains nothing but the CFrag
{
}

//------------------------------------------------------------------------------

void BHelper::New_Po_CdSh()
// Shared code: where it goes in the tree depends on the parent, but there's
// nothing to do here because the element contains nothing but the CFrag
{
}

//------------------------------------------------------------------------------

void BHelper::New_Po_DeI()
// Device instance. This only appears in one place in the language tree, so we
// don't need to muck about with nested case statements
{
string id = attrMap["id"];
                                       // Save some typing
map<string,unsigned> & rm = context_v.back()->pGraphI->Dmap;
                                       // Find out if it's already there
if (rm.find(id)!=rm.end()) {           // It's there
  DevI_t ** ppD = context_v.back()->pGraphI->G.FindNode(rm[id]);
                                       // Is it already defined? Yes - bleat
  if ((*ppD)->Def()!=0) par->Post(209,(*ppD)->Name());
  else (*ppD)->Def(1);                 // No - mark it defined
  pTos->pDevI = *ppD;                  // Store it the context frame
  return;
}
DevI_t * p = PushNewD(id);             // Not there; create it
p->Def(1);                             // Mark it defined
p->tyId = attrMap["type"];
pTos->pDevI = p;                       // Store it the context frame
}

//------------------------------------------------------------------------------

void BHelper::New_Po_DeIs()
// Device instances - nothing to do
{
}

//------------------------------------------------------------------------------

void BHelper::New_Po_DeT()
// DeviceType. Position is unambiguous
{
string id = attrMap["id"];
DevT_t * p = new DevT_t(pTos->pGraphT,id);
p->devTyp = 'D';                       // Normal device
pTos->pDevT = p;
pTos->pGraphT->DevT_v.push_back(p);
}

//------------------------------------------------------------------------------

void BHelper::New_Po_DeTs()
// DeviceTypes - nothing to do
{
}

//------------------------------------------------------------------------------

// Two flat entities to help the  WALKINPINS and WALKOUTPINS called in the
// next function. Alas, scoping rules.....

struct Z_t {                           // Carrier structure for the graph walker
  string   s;                          // In: name string to match
  unsigned k;                          // Out: key value of match
  PinI_t * p;                          // Out: Pin object of match
};

// WALKxxPINS applies IsPinThere repeatedly to each pin. If the name does not
// match, it returns early. If there is a match, the pin data is overwritten
// to the carrier structure.

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void IsPinThere(void * p,const unsigned & rP,PinI_t *& pP)
{
Z_t * pZ = (Z_t *)p;                   // Force the carrier structure type
if (pP->Name()!=pZ->s) return;         // If not a match, go early
pZ->k = rP;                            // Yes, there was, save the key
pZ->p = pP;                            // ...and the data
}

//------------------------------------------------------------------------------

void BHelper::New_Po_EgI()
// Edge instance. Unambiguous
{
//HandleEdgeI(attrMap["path"]);
string path = attrMap["path"];

string Dtos,Ptos,Dfrs,Pfrs;            // Path device and pin names
if (PathDecode(path,Dtos,Ptos,Dfrs,Pfrs)!=0) {
  par->Post(924,path);
  return;
}
                                       // If it's a supervisor edge, ignore it
if (Dtos.empty()||Dfrs.empty()) return;
                                       // Save some typing
GraphI_t * pDG = pTos->pGraphI;
map<string,unsigned> & rm = pDG->Dmap;
DevI_t ** ppDto=0;                     // "To" node there?
DevI_t *  pDto=0;                      // Entry in the graph is the pointer
if (rm.find(Dtos)!=rm.end())           // It's there already
  ppDto = pDG->G.FindNode(rm[Dtos]);   // So go get it
else pDto = PushNewD(Dtos);            // Otherwise insert a new one
if (ppDto!=0) pDto = *ppDto;           // One way or another, we have "To"

DevI_t ** ppDfr=0;                     // "Fr" node there?
DevI_t *  pDfr=0;
if (rm.find(Dfrs)!=rm.end())           // It's there already
  ppDfr = pDG->G.FindNode(rm[Dfrs]);
else pDfr = PushNewD(Dfrs);            // Otherwise insert a new one
if (ppDfr!=0) pDfr = *ppDfr;           // One way or another, we have "Fr"
                                       // We now have two nodes, in the graph.

                                       // Force (another?) edge in
unsigned KDto = pDto->Kdev;            // Extract key of 'to' device
unsigned KDfr = pDfr->Kdev;            // Extract key of 'from' device

Z_t Zfr;                               // Carrier structure for name search
Zfr.s = Pfrs;                          // Name we're looking for
Zfr.p = 0;                             // Not found it yet
pDG->G.WALKOUTPINS(&Zfr,KDfr,IsPinThere);  // Scan output pins looking for match
if (Zfr.p==0) Zfr.p=new PinI_t(pDG,Pfrs);  // It's not there - create it
                                       // So, either way, it's in the graph now
Z_t Zto;                               // And again for the input pins
Zto.s = Ptos;                          // Name we're looking for
Zto.p = 0;                             // Not found it yet
pDG->G.WALKINPINS(&Zto,KDto,IsPinThere);   // Scan input pins
if (Zto.p==0) Zto.p=new PinI_t(pDG,Ptos);  // It's not there - create it
                                       // So either way it's here now
P_link * pL = new P_link(string());    // New edge data object
pL->key = UniU();                      // New key value for it
unsigned Kpfr= UniU();                 // New key for 'from' pin
Zfr.p->AddKey(Kpfr);                   // Add to pin
unsigned Kpto = UniU();                // New key for 'to' pin
Zto.p->AddKey(Kpto);                   // Add to pin
                                       // Insert the edge into the graph
                                       //(from-to)
pDG->G.InsertArc(pL->key,KDfr,KDto,pL,Kpfr,Zfr.p,Kpto,Zto.p);
pTos->pEdgI = pL;                      // Add edge to context stack

/*

// Look at - well, everything, really
pDto->Dump();
pDfr->Dump();
Zto.p->Dump();
Zfr.p->Dump();
pM->Dump();
pDG->Dump();
WALKPDIGRAPHNODES(unsigned,P_device *,unsigned,P_message *,unsigned,P_pin *,
                  pDG->G,i) {
  pDG->G.NodeData(i)->Dump();
  WALKPDIGRAPHINPINS(unsigned,P_device *,unsigned,P_message *,unsigned,P_pin *,
                     pDG->G,pDG->G.NodeKey(i),j) pDG->G.PinData(j)->Dump();
  WALKPDIGRAPHOUTPINS(unsigned,P_device *,unsigned,P_message *,unsigned,P_pin *,
                     pDG->G,pDG->G.NodeKey(i),j) pDG->G.PinData(j)->Dump();
}
*/
}

//------------------------------------------------------------------------------

void BHelper::New_Po_EgIs()
// Edge instances - nothing to do
{
}

//------------------------------------------------------------------------------

void BHelper::New_Po_ExI()
// External instance is treated exactly the same as a device instance
{
New_Po_DeI();
}

//------------------------------------------------------------------------------

void BHelper::New_Po_ExT()
{
string id = attrMap["id"];
DevT_t * p = new DevT_t(pTos->pGraphT,id);
p->devTyp = 'E';                       // External device
pTos->pDevT = p;
pTos->pGraphT->DevT_v.push_back(p);
}

//------------------------------------------------------------------------------

void BHelper::New_Po_GrI()
// Graph instance. Position unique
{
string id = attrMap["id"];
GraphI_t * p = new GraphI_t(pTos->pApps,id);
pTos->pGraphI = p;
pTos->pApps->GraphI_v.push_back(p);
p->tyId = attrMap["graphTypeId"];      // Target in type tree from file
p->tyId2 = p->tyId;                    // Actual target (monkey mutable)
}

//------------------------------------------------------------------------------

void BHelper::New_Po_GrS()
// Found Graphs element. Create a new application object in OrchBase
{
FileName Fn(filename);
string apName = Fn.FNBase();
pTos->pApps = new Apps_t(par,apName);
pTos->pApps->filename = filename;      // These names are getting clunky
}

//------------------------------------------------------------------------------

void BHelper::New_Po_GrT()
// Found a GraphType element
{
string id = attrMap["id"];
GraphT_t * p = new GraphT_t(pTos->pApps,id);
pTos->pGraphT = p;
pTos->pApps->GraphT_v.push_back(p);    // Could be more than one
}

//------------------------------------------------------------------------------

void BHelper::New_Po_Meta(const vector<pair<string,string> > & vps)
// Found some metadata. This can go pretty much everywhere, and we know nothing
// about it.
// StartElement turns the XML attrtibute pair vector into a string:string map
// (attrMap) for us, but here - uniquely - we don't want that, so we *could*
// just loop through attrMap and generate an attribute string pair list from it,
// (i.e. reversing the processing done by MkAtMp, or we can short out the
// entire process and just whack it in as an argument.
{
if (pTos->broken) return;

WALKMAP(string,string,attrMap,i)
  printf("%s : %s\n",(*i).first.c_str(),(*i).second.c_str());

pTos->pMeta = new Meta_t(vps);         // Space for the pair list
Potype tag  = pTos->tag;               // Current tag;
Potype ptag = pTos->ptag;              // Current parent tag

switch (ptag) {
  case Po_GrT  : pTos->pGraphT->Meta_v.push_back(pTos->pMeta);       return;
  case Po_GrI  : pTos->pGraphI->Meta_v.push_back(pTos->pMeta);       return;
  case Po_DeT  : pTos->pDevT->Meta_v.push_back(pTos->pMeta);         return;
  case Po_SuT  : pTos->pSupT->Meta_v.push_back(pTos->pMeta);         return;
  case Po_ExT  : pTos->pDevT->Meta_v.push_back(pTos->pMeta);         return;
  case Po_ExI  :
  case Po_DeI  : pTos->pDevI->Meta_v.push_back(pTos->pMeta);         return;
  case Po_EgI  : if (pTos->pEdgI==0) { // A supervisor edge.....
                   delete pTos->pMeta;
                   pTos->pMeta = 0;
                 } else pTos->pEdgI->Meta_v.push_back(pTos->pMeta);  return;
  default      : par->Post(211,Potype_map[tag],Lx.GetLCs());      // NFI
                 delete pTos->pMeta;   // But I don't want it
                 pTos->pMeta = 0;
                 pTos->broken = true;  // Stop further loads after this frame
                 return;
}

}

//------------------------------------------------------------------------------

void BHelper::New_Po_Msg()
// Message itself - belongs to exactly one MessageType, and may/may not contain
// a CFrag - so do nothing
{
}

//------------------------------------------------------------------------------

void BHelper::New_Po_MsT()
// MessageType - there can be lots of these
{
string id = attrMap["id"];
MsgT_t * p = new MsgT_t(pTos->pGraphT,id);
pTos->pMsgT = p;
pTos->pGraphT->MsgT_v.push_back(p);    // May be more than one
}

//------------------------------------------------------------------------------

void BHelper::New_Po_MsTs()
// MessageTypes - nothing to do
{
}

//------------------------------------------------------------------------------

void BHelper::New_Po_OCTL()
{
}

//------------------------------------------------------------------------------

void BHelper::New_Po_OHW()
{
}

//------------------------------------------------------------------------------

void BHelper::New_Po_OIDL()
// OnIdle handler
{
CFrag * p = new CFrag();
pTos->pCFrag = p;
pTos->pDevT->pOnIdle = p;
}

//------------------------------------------------------------------------------

void BHelper::New_Po_OREC()
// OnReceive handler
{
CFrag * p = new CFrag();
pTos->pCFrag = p;
((pTos->pDevT->PinTI_v.back()))->pHandl = p;
}

//------------------------------------------------------------------------------

void BHelper::New_Po_ORTS()
// RTS handler
{
CFrag * p = new CFrag();
pTos->pCFrag = p;
pTos->pDevT->pOnRTS = p;
}

//------------------------------------------------------------------------------

void BHelper::New_Po_OSND()
// Onsend handler
{
CFrag * p = new CFrag();
pTos->pCFrag = p;
((pTos->pDevT->PinTO_v.back()))->pHandl = p;
}

//------------------------------------------------------------------------------

void BHelper::New_Po_PnIn()
// Input pin type
/*{
string id = attrMap["name"];
PinT_t * p = new PinT_t(pTos->pDevT,id);
pTos->pPinT = p;
pTos->pDevT->PinTI_v.push_back(p);
//pTos->pPinT->idx = pTos->pP_devtyp->P_pintypIv.size();
pTos->pPinT->par = pTos->pDevT;
p->tyIdM = attrMap["messageTypeId"];
} */
{
string id = attrMap["name"];
pTos->pPinT = new PinT_t(pTos->pDevT,id);
pTos->pPinT->tyIdM = attrMap["messageTypeId"];
                                       // Last two levels of XML:
Potype ptag = pTos->ptag;              // Parent to current
Potype tag = pTos->tag;                // Current
CFrag ** ppC;                          // Pointer to where we may/may not put it

switch (ptag) {
  case Po_DeT  : switch (tag) {        // Device type
                   case Po_PnIn : pTos->pDevT->PinTI_v.push_back(pTos->pPinT);
                                  return;
                   case Po_PnOu : pTos->pDevT->PinTO_v.push_back(pTos->pPinT);
                                  return;
                   default      : return;
                 }
  case Po_ExT  : switch (tag) {        // External type
                   case Po_PnIn : pTos->pDevT->PinTI_v.push_back(pTos->pPinT);
                                  return;
                   case Po_PnOu : pTos->pDevT->PinTO_v.push_back(pTos->pPinT);
                                  return;
                   default      : return;
                 }
  default      : par->Post(211,Potype_map[tag],Lx.GetLCs());   // NFI
                 delete pTos->pPinT;
                 pTos->pPinT = 0;
                 pTos->broken = true;                 // Stop future builds
                 return;
}
//gpar->Tr.Push("CDATA");
//fprintf(fo,"\nBHelper::CDATA derived from...");
//return xmlParser::CDATA(p,type,s);
}


//------------------------------------------------------------------------------

void BHelper::New_Po_PnOu()
// Output pin type
/*{
string id = attrMap["name"];
PinT_t * p = new PinT_t(pTos->pDevT,id);
pTos->pPinT = p;
pTos->pDevT->PinTO_v.push_back(p);
pTos->pPinT->par = pTos->pDevT;
p->tyIdM = attrMap["messageTypeId"];
} */
{
New_Po_PnIn();
}

//------------------------------------------------------------------------------

void BHelper::New_Po_SuT()
// Supervisor device type found (there is no corresponding instance)
{
string id = attrMap["id"];
SupT_t * p = new SupT_t(pTos->pGraphT,id);
p->devTyp = 'S';                       // Supervisor device
pTos->pSupT = p;
pTos->pGraphT->pSup = p;
}

//------------------------------------------------------------------------------

unsigned BHelper::PathDecode(string path,string & rDto,string & rPto,
                                     string & rDfr,string & rPfr)
// Decode the edge string. Should be of the form
// to_device:to_pin:from_device:from_pin, but either (or both) of the device
// fields may be empty (signifying supervisor connection) which is legitimate.
// When called from within BHelper, the return (error) code bails the whole
// thing, because we shouldn't have got this far. PathDecode (which is static)
// is *also* called from the Validator, where any dud path strings will cause
// a meaningful error message. Which is why this routine is state pure.
{
Lex Lx;
Lex::tokdat Td;
Lx.SetFile(path);                      // Point the lexer at the input
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3} toktyp;

struct duple {int ns,ac;} next;
duple table[7][t3+1] =
// Incident symbol               // Next
//    0      1      2      3     // state
{{{1, 1},{2, 0},{X, 2},{X, 2}},  // 0
 {{X, 2},{2, 0},{X, 2},{X, 2}},  // 1
 {{3, 3},{X, 2},{X, 2},{X, 2}},  // 2
 {{X, 2},{X, 2},{4, 0},{X, 2}},  // 3
 {{5, 4},{6, 0},{X, 2},{X, 2}},  // 2
 {{X, 2},{6, 0},{X, 2},{X, 2}},  // 3
 {{R, 5},{X, 2},{X, 2},{X, 2}}}; // 4

for(int state=0;;) {
  Lx.GetTok(Td);                       // Get the next token...
//  if (Td.t==Lex::Sy_EOR) continue;     // Chuck away any newlines
  if (Lx.IsError(Td)) return 3;
  switch (Td.t) {                      // Map to array index
    case Lex::Sy_col  : toktyp = t1;  break;
    case Lex::Sy_sub  : toktyp = t2;  break;
    default           : toktyp = t3;  break;
  }
  if (Lex::IsStr(Td.t))   toktyp = t0; // Reduction functions
                                       // Override the reduction functions ?
  next = table[state][toktyp];         // Make the transition
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  :               break;     // No action
    case 1  : rDto = Td.s;  break;     // Save "to" device
    case 2  : return 1;                // Fallen off syntax graph
    case 3  : rPto = Td.s;  break;     // Save "to" pin
    case 4  : rDfr = Td.s;  break;     // Save "from" device
    case 5  : rPfr = Td.s;  break;     // Save "from" pin
    default : return 2;                // Never here?
  }
  switch (state=next.ns) {
    case X  : return 1;                // Exit - should never be here - act code
    case R  : return 0;                // Clean return
    default : break;
  }
}

}

//------------------------------------------------------------------------------

DevI_t * BHelper::PushNewD(string id)
// Insert a new device into the graph
{
                                       // Save some typing
map<string,unsigned> & rm = context_v.back()->pGraphI->Dmap;
DevI_t * pD = new DevI_t(context_v.back()->pGraphI,id);
unsigned Kdev = UniU();                // Create a unique key
rm[pD->Name()] = Kdev;                 // Insert into graph map
pD->Kdev = Kdev;                       // Tell the device
context_v.back()->pGraphI->G.InsertNode(Kdev,pD);     //...and insert it
return pD;
}

//------------------------------------------------------------------------------

bool BHelper::StartDocument(const void * p,const string & s,
                            const vector<pair<string,string> > & vps)
// Breaking symmetry, and all that, but the first context frame has already
// been pushed in by the caller, because it's got the file details in it
{
//gpar->Tr.Push("Start_document");
//fprintf(fo,"\nBHelper::StartDocument derived from...");
//return xmlParser::StartDocument(p,s,vps);
return true;
}

//------------------------------------------------------------------------------

bool BHelper::StartElement(const void * p,const string & stag,
                               const vector<pair<string,string> > & vps)
// Add a new frame to the context stack, and build the relevant datastructure
// fragment.
{
context_v.push_back(pTos = new context(this,*context_v.back()));
//gpar->Tr.Push("Context_stack_size",context_v.size());
//printf("Context stack size = %u\n",context_v.size());
//fflush(stdout);
MkAtMp(const_cast<vector<pair<string,string> > &>(vps));               // Turn the attribute pair vector -> map
Potype tag = Potype_map[stag];         // Turn the element tag into an enum
pTos->tag = tag;                       // ... and save it
//WALKVECTOR(context *,context_v,i)(*i)->Dump();
//printf("Handling tag %s %d\n",stag.c_str(),int(tag));
//fflush(stdout);
if (pTos->broken) return true;
switch (tag) {
  case Po_CdP  : New_Po_CdP();     break; // Property code
  case Po_CdS  : New_Po_CdS();     break; // State code
  case Po_CdSh : New_Po_CdSh();    break; // Shared code
  case Po_DeI  : New_Po_DeI();     break; // Device instance
  case Po_DeIs : New_Po_DeIs();    break; // Device instances
  case Po_DeT  : New_Po_DeT();     break; // DeviceType
  case Po_DeTs : New_Po_DeTs();    break; // DeviceTypes
  case Po_EgI  : New_Po_EgI();     break; // Edge instance
  case Po_EgIs : New_Po_EgIs();    break; // Edge instances
  case Po_ExI  : New_Po_ExI();     break; // External device instance
  case Po_ExT  : New_Po_ExT();     break; // External device type
  case Po_GrI  : New_Po_GrI();     break; // Graph instance
  case Po_GrS  : New_Po_GrS();     break; // Graphs
  case Po_GrT  : New_Po_GrT();     break; // GraphType
  case Po_Meta : New_Po_Meta(vps); break; // Metadata
  case Po_Msg  : New_Po_Msg();     break; // Message
  case Po_MsT  : New_Po_MsT();     break; // MessageType
  case Po_MsTs : New_Po_MsTs();    break; // MessageTypes
  case Po_OCTL : New_Po_OCTL();    break; // OnControl handler
  case Po_OHW  : New_Po_OHW();     break; // On hardware idle handler
  case Po_OIDL : New_Po_OIDL();    break; // OnIdle handler
  case Po_OREC : New_Po_OREC();    break; // Recieve handler
  case Po_ORTS : New_Po_ORTS();    break; // Ready to send handler
  case Po_OSND : New_Po_OSND();    break; // Send handler
  case Po_PnIn : New_Po_PnIn();    break; // Input pin type
  case Po_PnOu : New_Po_PnOu();    break; // Output pin type
  case Po_SuT  : New_Po_SuT();     break; // Supervisor type
  default      : par->Post(210,stag,Lx.GetLCs());  break;
}

if (dflag!=0)fprintf(fo,"\nBHelper::StartElement derived from...");
xmlParser::StartElement(p,stag,vps);
return true;
}

//==============================================================================
// SUBCLASS node METHODS

BHelper::context::context(BHelper * p)
{
par     = p;
tag     = P_0;
pApps   = 0;
pGraphT = 0;
pDevT   = 0;
pPinT   = 0;
pSupT   = 0;
pGraphI = 0;
pDevI   = 0;
pPinI   = 0;
pEdgI   = 0;
pMsgT   = 0;
pCFrag  = 0;
pMeta   = 0;
broken  = false;
}

//------------------------------------------------------------------------------

BHelper::context::context(BHelper * p,context & c)
{
par     = p;                           // Class parent (Bhelper)
ptag    = c.tag;                       // Parent tag (previous context frame)
tag     = P_0;                         // New tag (this context frame)
pApps   = c.pApps;                     // Previous context values.....
pGraphT = c.pGraphT;
pDevT   = c.pDevT;
pPinT   = c.pPinT;
pSupT   = c.pSupT;
pGraphI = c.pGraphI;
pDevI   = c.pDevI;
pPinI   = c.pPinI;
pEdgI   = c.pEdgI;
pMsgT   = c.pMsgT;
pCFrag  = c.pCFrag;
pMeta   = c.pMeta;
broken  = c.broken;                    // Semantic error found?
}

//------------------------------------------------------------------------------

BHelper::context::~context(void)
{

}

//------------------------------------------------------------------------------

void BHelper::context::Dump(FILE * fp,int off)
{
fprintf(fp,"BHelper::context frame dump++++++++++++++++++++++\n");
fprintf(fp,"Context tag          : %s %d\n",
           par->Potype_map[tag].c_str(),int(tag));
fprintf(fp,"Previous context tag : %s %d\n",
           par->Potype_map[ptag].c_str(),int(ptag));
if (pApps   ==0) fprintf(fp,"pApps   :    0\n");
else {
  fprintf(fp,"pApps   :   %s\n",pApps->Name().c_str());
  pApps->DefRef::Dump(fp);
}
if (pGraphT ==0) fprintf(fp,"pGraphT :    0\n");
else {
  fprintf(fp,"pGraphT : %s\n",pGraphT->Name().c_str());
  pGraphT->DefRef::Dump(fp);
}
if (pDevT   ==0) fprintf(fp,"pDevT   :    0\n");
else {
  fprintf(fp,"pDevT   : %s\n",pDevT->Name().c_str());
  pDevT->DefRef::Dump(fp);
}
if (pPinT   ==0) fprintf(fp,"pPinT   :    0\n");
else {
  fprintf(fp,"pPinT   : %s\n",pPinT->Name().c_str());
  pPinT->DefRef::Dump(fp);
}
if (pSupT   ==0) fprintf(fp,"pSupT   :    0\n");
else {
  fprintf(fp,"pSupT   : %s\n",pSupT->Name().c_str());
  pSupT->DefRef::Dump(fp);
}
if (pGraphI ==0) fprintf(fp,"pGraphI :    0\n");
else {
  fprintf(fp,"pGraphI : %s\n",pGraphI->Name().c_str());
  pGraphI->DefRef::Dump(fp);
}
if (pDevI   ==0) fprintf(fp,"pDevI   :    0\n");
else {
  fprintf(fp,"pDevI   : %s\n",pDevI->Name().c_str());
  pDevI->DefRef::Dump(fp);
}
if (pPinI   ==0) fprintf(fp,"pPinI   :    0\n");
else {
  fprintf(fp,"pPinI   : %s\n",pPinI->Name().c_str());
  pPinI->DefRef::Dump(fp);
}
if (pEdgI   ==0) fprintf(fp,"pEdgI   :    0\n");
else {
  fprintf(fp,"pEdgI   : %s\n",pEdgI->Name().c_str());
  pEdgI->DefRef::Dump(fp);
}
if (pMsgT   ==0) fprintf(fp,"pMsgT   :    0\n");
else {
  fprintf(fp,"pMsgT   : %s\n",pMsgT->Name().c_str());
  pMsgT->DefRef::Dump(fp);
}
if (pCFrag  ==0) fprintf(fp,"pCFrag  :    0\n");
//else {
//  fprintf(fp,"pCFrag  : %s\n",pCFrag->Name().c_str());
//  pCFrag->DefRef::Dump(fp);
//}
if (pMeta  ==0) fprintf(fp,"pMeta   :    0\n");
//else {
//  fprintf(fp,"pMeta  : %s\n",pMeta->Name().c_str());
//  pMeta->DefRef::Dump(fp);
//}
fprintf(fp,"BHelper::context frame dump----------------------\n");
}

//==============================================================================


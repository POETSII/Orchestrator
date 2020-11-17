//---------------------------------------------------------------------------

#include "DS_integ.h"
#include "Apps_t.h"
#include "SupT_t.h"
#include "PinI_t.h"
#include "PinT_t.h"
#include "CFrag.h"
#include "FileName.h"
#include "Pglobals.h"
#include "OrchBase.h"
#include "GraphT_t.h"
#include "MsgT_t.h"

//==============================================================================
/*

Temporary code. This relies on visual inspection of the output, which is tedious
and error-prone. Better is to build an inversion structure - see notes 20/7/20.
But we go with this for now because it's here and now and I want to get this
out of the door.

This ISN'T derived off the XML parser. The datastructure is intact, in the
sense that there are no structural errors in the definition, but this is an
attempt to make sure it makes sense at the highest level: are all the vertices
in the graph instance graph actually defined, do the entity type definitions
have non-blank fields, and so on. There are different sorts of checks:

BUILD (itype = 'B') where we do sanity checks on the device instance graph
and the type tree separately, and then the typelinks, if they exist
PLACE (itype = 'P') where we check the placement and all associated data
These (the above) are all focussed on user data, so the output goes to the
detail file
SYSTEM (itype = 'S') we check for undiscovered unrecoverables; these errors
go to the LogServer
TREE (itype = 'T') we check the integrity of the type trees in isolation
ALL (itype = '*') where we do everything

There is no management in this class: the very instantiation triggers the
checks, so it's hardly flexible or interactive. In fact, it's all rather Noddy,
but BigEars had gone to the pub.

This is not a Dump or a Show; we just look for things that are structurally
and unambiguously wrong. Weirdness we tolerate, in the main, silently; but
sometimes trivial omissions (lack of datestamp, for example) can lead to the
discovery of bigger cockups.
*/
//==============================================================================

DS_integ::DS_integ(char _itype,Apps_t * _pA,OrchBase * p)
{
pA       = _pA;
if (pA==0) {
  par->Post(929,"Corrupt application name");   // Unrecoverable
  return;                              // One more error....
}

apName   = pA->FullName();
itype    = _itype;                     // Integrity check type
par      = p;                          // My parent
ecnt     = 0;                          // Accumulated errors
wcnt     = 0;                          // Accumulated warnings
fd       = par->fd;                    // Detail outout stream
ReportIntegStart();                    // Let the user know what's going on
CheckApps();                           // Application anchors
switch (itype) {                       // So do it :
  case 'B' : CheckInstanceGraph();     // Device instance graphs
             CheckTLink();             // Typelinks if any
             break;
  case 'P' : CheckPlace();             // Placement linkages if any
             break;
  case 'S' : CheckSystem();            // Infrastructure
             break;
  case 'T' : CheckTypeTree();          // Type trees in isolation
             break;
  case '*' : CheckSystem();            // Everything
             CheckInstanceGraph();
             CheckTLink();
             CheckTypeTree();
             CheckPlace();
             break;
  default  : break;
}
ReportIntegEnd();
}

//------------------------------------------------------------------------------

DS_integ::~DS_integ()
//
{
}

//------------------------------------------------------------------------------

void DS_integ::CheckApps()
// Check the Apps_t objects
{
fprintf(fd,"*** DS_integ::CheckApps\n");
fflush(fd);
if (pA==0) return;
unsigned ld = pA->Def();
const char * Ns = pA->FullName().c_str();
if (pA->par==0) fprintf(fd,"E%3u. (%3u) "
  "Application %s has no parent defined\n",++ecnt,ld,Ns);
if (pA->filename.empty()) fprintf(fd,"E%3u. (%3u) "
  "Application %s has no filename defined\n",++ecnt,ld,Ns);
if (pA->sTime.empty()) fprintf(fd,"E%3u. (%3u) "
  "Application %s has no timestamp defined\n",++ecnt,ld,Ns);
if (pA->sDate.empty()) fprintf(fd,"E%3u. (%3u) "
  "Application %s has no datestamp defined\n",++ecnt,ld,Ns);
if (pA->pPoL==0) return;
if (pA->pPoL->type.empty()) fprintf(fd,"E%3u. (%3u) "
  "Application %s is PoL, but it has no type\n",++ecnt,ld,Ns);
fprintf(fd,"\n");
fflush(fd);
}

//------------------------------------------------------------------------------

void DS_integ::CheckInstanceGraph()
// Check all the graphs INTERNALLY (i.e. not the Tlinks) in the application pA
{
fprintf(fd,"*** DS_integ::CheckInstanceGraph\n");
WALKVECTOR(GraphI_t *,pA->GraphI_v,i) {// Walk graph instances
  GraphI_t * pG = *i;
  string Gs = pG->FullName();
  const char * Gcs = Gs.c_str();
  unsigned ld = pG->Def();
  if (pG->par==0) fprintf(fd,"E%3u. (%3u) "
    "Graph %s has no parent defined\n",++ecnt,ld,Gcs);
  else
    if (pG->Name().empty()) fprintf(fd,"E%3u. "
      "Graph (filename %s) has no name\n",++ecnt,pG->par->filename.c_str());
  if (pG->Def()==0) fprintf(fd,"E%3u. Graph %s not defined\n",++ecnt,Gcs);
  if (pG->tyId.empty()) fprintf(fd,"E%3u. (%3u) "
    "Graph instance %s has no default type defined\n",++ecnt,ld,Gcs);
  if (pG->tyId2.empty())
    fprintf(fd,"E%3u. (%3u) "
      "Graph instance %s has no actual type defined\n",++ecnt,ld,Gcs);
  if (pG->pPropsI==0) fprintf(fd,"I   . (%3u) "
    "Graph instance %s has no specified properties initialiser\n",ld,Gcs);
  if (pG->Dmap.empty())
    fprintf(fd,"E%3u. (%3u) "
      "Graph instance %s has no device map entries\n",++ecnt,ld,Gcs);
                                       // Walk the graph nodes
  WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,
                  pG->G,i) {
    DevI_t * pD = pG->G.NodeData(i);   // Device instance
    string Ds = pD->FullName();        // Stack movements. Think about it.
    const char * Dcs = Ds.c_str();     // Device name
    const char * Dis = "Device instance";  // Save some typing
    unsigned l = pD->Def();            // Definition line
    if (pD->par!=pG) fprintf(fd,"E%3u. (%3u) "
      "%s %s parent pointer corrupt\n",++ecnt,l,Dis,Dcs);
    if (l==0) fprintf(fd,"E%3u. (%3u) "
      "%s %s is not defined\n",++ecnt,l,Dis,Dcs);
    if (pD->Ref()==0) fprintf(fd,"W%3u. (%3u) "
      "%s %s is not referenced\n",++wcnt,l,Dis,Dcs);
    if (pD->devTyp=='U')
      fprintf(fd,"E%3u. (%3u) "
        "%s %s has no specified type\n",++ecnt,l,Dis,Dcs);
    if (pD->Key==0) fprintf(fd,"W%3u. (%3u) "
      "%s %s has no specified graph key\n",++wcnt,l,Dis,Dcs);
    if (pD->pPropsI==0) fprintf(fd,"I   . (%3u) "
      "%s %s has no specified properties initialiser\n",l,Dis,Dcs);
    if (pD->pStateI==0) fprintf(fd,"I   . (%3u) "
      "%s %s has no specified state initialiser\n",l,Dis,Dcs);
    if (pG->G.SizeInPins(pD->Key)==0) fprintf(fd,"W%3u. (%3u) "
       "%s %s has no packet input pins\n",++wcnt,l,Dis,Dcs);
    if (pG->G.SizeOutPins(pD->Key)==0) fprintf(fd,"W%3u. (%3u) "
      "%s %s has no packet output pins\n",++wcnt,l,Dis,Dcs);
                                       // Now walk input pins on device
    WALKPDIGRAPHINPINS(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,
                       pG->G,pG->G.NodeKey(i),j){
      PinI_t * pP = pG->G.PinData(j);  // Get the pin from the iterator
      string Ps = pP->FullName().c_str();
      const char * Pcs = Ps.c_str();   // Pin name
      unsigned lp = pP->Def();
      const char * Dps = "Device input pin"; // Save some typing
      if (pP->par!=pG) fprintf(fd,"E%3u. (%3u) "
        "%s %s parent pointer corrupt\n",++ecnt,lp,Dis,Dcs);
      if (lp==0) fprintf(fd,"E%3u. (%3u) "
        "%s %s is not defined\n",++ecnt,lp,Dps,Pcs);
      if (pP->Ref()==0) fprintf(fd,"W%3u. (%3u) "
        "%s %s is not referenced\n",++wcnt,lp,Dps,Pcs);
      fprintf(fd,"I   . (%3u) %s %s has %lu pin keys:\n",
                  l,Dps,Pcs,pP->Key_v.size());
      WALKVECTOR(unsigned,pP->Key_v,k) fprintf(fd,"      ... %5u\n",*k);
    } // End WALKPDIGRAPHINPINS
                                       // And again for the output pins
    WALKPDIGRAPHOUTPINS(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,
                       pG->G,pG->G.NodeKey(i),j){
      PinI_t * pP = pG->G.PinData(j);  // Get the pin from the iterator
      string Ps = pP->FullName().c_str();
      const char * Pcs = Ps.c_str();   // Pin name
      unsigned lp = pP->Def();
      const char * Dps = "Device output pin"; // Save some typing
      if (pP->par!=pG) fprintf(fd,"E%3u. (%3u) "
        "%s %s parent pointer corrupt\n",++ecnt,lp,Dis,Dcs);
      if (lp==0) fprintf(fd,"E%3u. (%3u) "
        "%s %s is not defined\n",++ecnt,lp,Dps,Pcs);
      if (pP->Ref()==0) fprintf(fd,"W%3u. (%3u) "
        "%s %s is not referenced\n",++wcnt,lp,Dps,Pcs);
      fprintf(fd,"I   . (%3u) %s %s has %lu pin keys:\n",
                  l,Dps,Pcs,pP->Key_v.size());
      WALKVECTOR(unsigned,pP->Key_v,k) fprintf(fd,"      ... %5u\n",*k);
    } // End WALKPDIGRAPHOUTPINS
    unsigned pcnt = pG->G.SizeInPins(pG->G.NodeKey(i)) +
                    pG->G.SizeInPins(pG->G.NodeKey(i));
    if (pcnt!=pD->Pmap.size()) fprintf(fd,"E%3u. (%3u) "
      "%s %s has pin map inconsistent with digraph\n",++ecnt,l,Dis,Dcs);
  } // End WALKPDIGRAPHNODES
} // End WALKVECTOR
fprintf(fd,"\n");
fflush(fd);
}

//------------------------------------------------------------------------------

void DS_integ::CheckPlace()
{
fprintf(fd,"*** DS_integ::CheckPlace() stub\nTo do ...\n");
fflush(fd);
if (pA==0) return;
fprintf(fd,"\n");
fflush(fd);
}

//------------------------------------------------------------------------------

void DS_integ::CheckSystem()
{
fprintf(fd,"*** DS_integ::CheckSystem() stub\nTo do ...\n");
fprintf(fd,"\n");
fflush(fd);
if (pA==0) return;
}

//------------------------------------------------------------------------------

void DS_integ::CheckTLink()
// Check the integrity of the typelinks. The existance of the link *from*
// GraphI_t *to* GraphT_t defines that the links should exist and be complete.
// "TLinkedness" is a property of the device instance graph, NOT the type tree
{
fprintf(fd,"*** DS_integ::CheckTLink()\n");
fflush(fd);
if (pA==0) return;

WALKVECTOR(GraphI_t *,pA->GraphI_v,i) {
  GraphI_t * pG = *i;
  string Gs = pG->FullName();
  const char * Gcs = Gs.c_str();
  unsigned l = pG->Def();              // Definition line
  if (!pG->TLink()) {
    fprintf(fd,"I   . (%u) Graph %s is not typelinked\n",l,Gcs);
                                       // So check the devices and pins
    WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,
                      pG->G,i) {
      DevI_t * pD = pG->G.NodeData(i); // Device instance
      string Ds = pD->FullName();      // Stack movements. Think about it.
      const char * Dcs = Ds.c_str();   // Device name
      const char * Dis = "Device instance"; // Save some typing
      unsigned ld = pD->Def();         // Definition line
      if (pD->pT!=0) fprintf(fd,"E%3u. (%u) : "
        "...but %s %s IS typelinked to %s\n",
        ++ecnt,ld,Dis,Dcs,pD->pT->FullName().c_str());
                                       // Now walk input pins on device
      WALKPDIGRAPHINPINS(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,
                         pG->G,pG->G.NodeKey(i),j){
        PinI_t * pP = pG->G.PinData(j);// Get the pin from the iterator
        string Ps = pP->FullName().c_str();
        const char * Pcs = Ps.c_str(); // Pin name
        const char * Dps = "Device input pin";// Save some typing
        unsigned lp = pP->Def();
        if (pP->pT!=0) fprintf(fd,"E%3u. (%3u) "
          "...but %s %s IS typelinked to %s \n",
          ++ecnt,lp,Dps,Pcs,pP->pT->FullName().c_str());
      } // End WALKPDIGRAPHINPINS
                                       // And again for the output pins
      WALKPDIGRAPHOUTPINS(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,
                         pG->G,pG->G.NodeKey(i),j){
        PinI_t * pP = pG->G.PinData(j);// Get the pin from the iterator
        string Ps = pP->FullName().c_str();
        const char * Pcs = Ps.c_str(); // Pin name
        const char * Dps = "Device output pin";// Save some typing
        unsigned lp = pP->Def();
        if (pP->pT!=0) fprintf(fd,"E%3u. (%3u) "
          "...but %s %s IS typelinked to %s \n",
          ++ecnt,lp,Dps,Pcs,pP->pT->FullName().c_str());
      } // End WALKPDIGRAPHOUTPINS
    } // End WALKPDIGRAPHNODES
    continue;                          // End of ...graph is NOT typelinked
  }                                    // ... or else it IS typelinked:
                                       // OK, let's walk the graph instance
  WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,
                  pG->G,i) {
    DevI_t * pD = pG->G.NodeData(i);   // Device instance
    string Ds = pD->FullName();        // Stack movements. Think about it.
    const char * Dcs = Ds.c_str();     // Device name
    const char * Dis = "Device instance";    // Save some typing
    unsigned ld = pD->Def();           // Definition line
    if (pD->pT==0)
      fprintf(fd,"E%3u. (%3u) "
        "%s %s NOT typelinked, but the parent graph \n           "
        "(%s) IS (to %s)\n",
        ++ecnt,ld,Dis,Dcs,Gcs,pG->pT->FullName().c_str());
                                       // Now walk input pins on device
    WALKPDIGRAPHINPINS(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,
                       pG->G,pG->G.NodeKey(i),j){
      PinI_t * pP = pG->G.PinData(j);  // Get the pin from the iterator
      string Ps = pP->FullName().c_str();
      const char * Pcs = Ps.c_str();   // Pin name
      const char * Dps = "Device input pin"; // Save some typing
      unsigned lp = pP->Def();
      if (pP->pT==0) fprintf(fd,"E%3u. (%3u) "
        "%s %s is NOT typelinked, but the parent graph \n           "
        "(%s) IS (to %s)\n",
        ++ecnt,lp,Dps,Pcs,Gcs,pG->pT->FullName().c_str());
    } // End WALKPDIGRAPHINPINS
                                       // And again for the output pins
    WALKPDIGRAPHOUTPINS(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,
                       pG->G,pG->G.NodeKey(i),j){
      PinI_t * pP = pG->G.PinData(j);  // Get the pin from the iterator
      string Ps = pP->FullName().c_str();
      const char * Pcs = Ps.c_str();   // Pin name
      const char * Dps = "Device output pin";// Save some typing
      unsigned lp = pP->Def();
      if (pP->pT==0) fprintf(fd,"E%3u. (%3u) "
        "%s %s is NOT typelinked, but the parent graph \n           "
        "(%s) IS (to %s)\n",
        ++ecnt,lp,Dps,Pcs,Gcs,pG->pT->FullName().c_str());
    } // End WALKPDIGRAPHOUTPINS
  } // End WALKPDIGRAPHNODES
                                       // Make sure the type tree knows
  GraphT_t * pGT = pG->pT;             // Putative type tree
  bool backlink = false;
  WALKVECTOR(GraphI_t *,pGT->GraphI_v,j) if ((*j)==pG) backlink = true;
  if (backlink==false) fprintf(fd,"E%3u. (%3u) "
    "Graph instance %s is typelinked to %s,\n            but the target type "
    "tree (%s) has no corresponding backpointer\n",
    ++ecnt,l,Gcs,pGT->FullName().c_str(),pG->pT->FullName().c_str());
} // End WALKVECTOR
fprintf(fd,"\n");
fflush(fd);
}

//------------------------------------------------------------------------------

void DS_integ::CheckTypeTree()
// Check the internals of the type tree
{
fprintf(fd,"*** DS_integ::CheckTypeTree()\n");
fflush(fd);
if (pA==0) return;

WALKVECTOR(GraphT_t *,pA->GraphT_v,i) {
  GraphT_t * pG = *i;
  string Gs = pG->FullName();
  const char * Gcs = Gs.c_str();
  const char * Gts = "Graph type tree";// Save some typing
  unsigned lg = pG->Def();             // Definition line
  if (lg==0) fprintf(fd,"E%3u. %s %s not defined\n",++ecnt,Gts,Gcs);
  if (pG->Ref()==0) fprintf(fd,"W%3u. (%3u) "
        "%s %s is not referenced\n",++wcnt,lg,Gts,Gcs);
  if (pG->pShCd==0) fprintf(fd,"W%3u : "
    "%s %s has no shared code defined\n",++wcnt,Gts,Gcs);
  else
    if (pG->pShCd->Size()==0) fprintf(fd,"W%3u. (%3u) : "
      "%s %s has zero-size shared code defined\n",++wcnt,lg,Gts,Gcs);
  if (pG->pPropsD==0) fprintf(fd,"W%3u. (%3u) : "
    "%s %s has no properties code defined\n",++wcnt,lg,Gts,Gcs);
  else
    if (pG->pPropsD->Size()==0) fprintf(fd,"W%3u. (%3u) : "
      "%s %s has zero-size properties code defined\n",++wcnt,lg,Gts,Gcs);
  SupT_t * pS = pG->pSup;
  if (pS==0)
    fprintf(fd,"W%3u. (%3u) : "
      "%s %s has no supervisor device type defined\n",++wcnt,lg,Gts,Gcs);
  else SubCheckSup(pS);
                                       // OK, let's walk the device types
  WALKVECTOR(DevT_t *,pG->DevT_v,j) {
    DevT_t * pD = *j;                  // Device type instance
    string Ds = pD->FullName();        // Stack movements. Think about it.
    const char * Dcs = Ds.c_str();     // Device type name
    const char * Dis = "Device type";  // Save some typing
                                       // Now the def/ref stuff
    unsigned ld = pD->Def();           // Definition line
    if (ld==0) fprintf(fd,"E%3u. %s %s not defined\n",++ecnt,Dis,Dcs);
    if (pD->Ref()==0) fprintf(fd,"W%3u. (%3u) "
      "%s %s is not referenced\n",++wcnt,ld,Dis,Dcs);
    SubCheckDevType(pD);               // Check all the static stuff
                                       // Walk input pin types
    WALKVECTOR(PinT_t *,pD->PinTI_v,k) SubCheckInPin(*k);
    if (pD->pPinTSI==0) fprintf(fd,"E%3u. (%3u) "
      "%s %s has no supervisor input pin defined\n",++ecnt,ld,Dis,Dcs);
    else SubCheckInPin(pD->pPinTSI);
                                       // Walk ouput pin types
    WALKVECTOR(PinT_t *,pD->PinTI_v,k) SubCheckOutPin(*k);
    if (pD->pPinTSO==0) fprintf(fd,"E%3u. (%3u) "
      "%s %s has no supervisor output pin defined\n",++ecnt,ld,Dis,Dcs);
    else SubCheckInPin(pD->pPinTSO);
  } // End WALKVECTOR
} // End WALKVECTOR
fprintf(fd,"\n");
fflush(fd);
}

//------------------------------------------------------------------------------

void DS_integ::Dump(unsigned off,FILE * fp)
{
fprintf(fp,"DS_integ+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

fprintf(fp,"DS_integ-----------------------------------------------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void DS_integ::SubCheckInPin(PinT_t * pP)
{
string Ps = pP->FullName();            // Stack movements. Think about it.
const char * Pcs = Ps.c_str();         // Pin type name
static const char * Pis = "Input pin type";  // Save some typing
unsigned lp = pP->Def();               // Definition line
if (lp==0) fprintf(fd,"E%3u. %s %s not defined\n",++ecnt,Pis,Pcs);
if (pP->pPropsD==0) fprintf(fd,"W%3u. (%3u) "
  "%s %s has no specified properties declaration\n",++wcnt,lp,Pcs,Pis);
else
  if (pP->pPropsD->Size()==0) fprintf(fd,"W%3u. (%3u) "
    "%s %s has zero-sized properties declaration\n",++wcnt,lp,Pcs,Pis);
if (pP->pStateD==0) fprintf(fd,"W%3u. (%3u) "
  "%s %s has no specified state declaration\n",++wcnt,lp,Pcs,Pis);
else
  if (pP->pStateD->Size()==0) fprintf(fd,"W%3u. (%3u) "
    "%s %s has zero-sized state declaration\n",++wcnt,lp,Pcs,Pis);
if (pP->pHandl==0) fprintf(fd,"E%3u. (%3u) "
  "%s %s has no specified OnRecv handler\n",++ecnt,lp,Pcs,Pis);
else
  if (pP->pHandl->Size()==0) fprintf(fd,"E%3u. (%3u) "
    "%s %s has zero-sized OnRecv handler\n",++ecnt,lp,Pcs,Pis);
if (pP->tyId.empty()) fprintf(fd,"E%3u. (%3u) "
  "%s %s has no message name\n",++ecnt,lp,Pcs,Pis);
if (pP->pMsg==0) fprintf(fd,"E%3u. (%3u) "
  "%s %s has no message pointer\n",++ecnt,lp,Pcs,Pis);
else
  if (pP->pMsg->Name()!=pP->tyId) fprintf(fd,"E%3u. (%3u) "
    "%s %s has inconsistent message name binding\n",++ecnt,lp,Pcs,Pis);
}

//------------------------------------------------------------------------------

void DS_integ::SubCheckOutPin(PinT_t * pP)
{
string Ps = pP->FullName();      // Stack movements. Think about it.
const char * Pcs = Ps.c_str();   // Pin type name
static const char * Pis = "Output pin type";   // Save some typing
unsigned lp = pP->Def();         // Definition line
if (lp==0) fprintf(fd,"E%3u. %s %s not defined\n",++ecnt,Pis,Pcs);
if (pP->pPropsD!=0) fprintf(fd,"E%3u. (%3u) "
  "%s %s has specified properties declaration\n",++ecnt,lp,Pcs,Pis);
if (pP->pStateD==0) fprintf(fd,"W%3u. (%3u) "
  "%s %s has specified state declaration\n",++wcnt,lp,Pcs,Pis);
if (pP->pHandl==0) fprintf(fd,"E%3u. (%3u) "
  "%s %s has no specified OnSend handler\n",++ecnt,lp,Pcs,Pis);
else
  if (pP->pHandl->Size()==0) fprintf(fd,"E%3u. (%3u) "
    "%s %s has zero-sized OnSend handler\n",++ecnt,lp,Pcs,Pis);
if (pP->tyId.empty()) fprintf(fd,"E%3u. (%3u) "
  "%s %s has no message name\n",++ecnt,lp,Pcs,Pis);
if (pP->pMsg==0) fprintf(fd,"E%3u. (%3u) "
  "%s %s has no message pointer\n",++ecnt,lp,Pcs,Pis);
else
  if (pP->pMsg->Name()!=pP->tyId) fprintf(fd,"E%3u. (%3u) "
    "%s %s has inconsistent message name binding\n",++ecnt,lp,Pcs,Pis);
}

//------------------------------------------------------------------------------

void DS_integ::ReportIntegEnd()
// Integrity check epilogue
{
fprintf(fd,"\nApplication integrity check:\n");
fprintf(fd,"...%s exhibits %u(%u) errors (warnings) in %ld msecs\n\n",
           apName.c_str(),ecnt,wcnt,mTimer(t0));
fprintf(fd,"%s %s\n%s\n",GetDate(),GetTime(),Q::sline.c_str());
fflush(fd);
}

//------------------------------------------------------------------------------

void DS_integ::ReportIntegStart()
// Integrity check prologue
{
t0 = mTimer();
fprintf(fd,"%s\n%s %s\n\n",Q::pline.c_str(),GetDate(),GetTime());
fprintf(fd,"Application integrity check %s\n\n",apName.c_str());
fprintf(fd,"(lin) refers to defining line number in client file\n\n");
fprintf(fd,"Integrity check:\n\n");
fflush(fd);
}

//------------------------------------------------------------------------------

void DS_integ::SubCheckDevType(DevT_t * pD)
// Static checks on a device type
{
string Ds = pD->FullName();            // Stack movements. Think about it.
const char * Dcs = Ds.c_str();         // Device type name
static const char * Dis = "Device type";// Save some typing
unsigned l = pD->Def();                // Definition line

if (pD->pShCd==0) fprintf(fd,"W%3u. (%3u) "
  "%s %s has no specified shared code\n",++wcnt,l,Dis,Dcs);
else
  if (pD->pShCd->Size()==0) fprintf(fd,"W%3u. (%3u) "
    "%s %s has zero-sized specified shared code\n",++wcnt,l,Dis,Dcs);

if (pD->pOnDeId==0) fprintf(fd,"W%3u. (%3u) "
  "%s %s has no specified OnIdle handler\n",++wcnt,l,Dis,Dcs);
else if (pD->pOnDeId->Size()==0)
  fprintf(fd,"W%3u. (%3u) "
    "%s %s has zero-sized OnIdle handler\n",++wcnt,l,Dis,Dcs);

if (pD->pOnHWId==0) fprintf(fd,"W%3u. (%3u) "
  "%s %s has no specified OnHardwareIdle handler\n",++wcnt,l,Dis,Dcs);
else
  if (pD->pOnHWId->Size()==0)fprintf(fd,"W%3u. (%3u) "
    "%s %s has zero-sized OnHarwareIdle handler\n",++wcnt,l,Dis,Dcs);

if (pD->pOnRTS==0)fprintf(fd,"W%3u. (%3u) "
  "%s %s has no specified OnReadyToSend handler\n",++wcnt,l,Dis,Dcs);
else
  if (pD->pOnRTS->Size()==0)fprintf(fd,"W%3u. (%3u) "
    "%s %s has zero-sized OnReadyToSend handler\n",++wcnt,l,Dis,Dcs);

if (pD->pOnInit==0) fprintf(fd,"W%3u. (%3u) "
  "%s %s has no specified OnInitialisation handler\n",++wcnt,l,Dis,Dcs);
else
  if (pD->pOnInit->Size()==0) fprintf(fd,"W%3u. (%3u) "
    "%s %s has zero-sized OnInitialisation handler\n",++wcnt,l,Dis,Dcs);

if (pD->pPropsD==0) fprintf(fd,"W%3u. (%3u) "
  "%s %s has no specified properties declaration\n",++wcnt,l,Dis,Dcs);
else
  if (pD->pPropsD->Size()==0) fprintf(fd,"W%3u. (%3u) "
    "%s %s has zero-sized properties declaration\n",++wcnt,l,Dis,Dcs);

if (pD->pStateD==0) fprintf(fd,"W%3u. (%3u) "
  "%s %s has no specified state declaration\n",++wcnt,l,Dis,Dcs);
else
  if (pD->pStateD->Size()==0)fprintf(fd,"W%3u. (%3u) "
    "%s %s has zero-sized state declaration\n",++wcnt,l,Dis,Dcs);

if (pD->devTyp=='U') fprintf(fd,"E%3u. (%3u) "
  "%s %s is of undefined type\n",++ecnt,l,Dis,Dcs);

if (pD->pPinTSI==0) fprintf(fd,"W%3u. (%3u) "
  "%s %s has no specified pin for input from supervisor\n",++wcnt,l,Dis,Dcs);
else SubCheckInPin(pD->pPinTSI);

if (pD->pPinTSO==0) fprintf(fd,"W%3u. (%3u) "
  "%s %s has no specified pin for output to supervisor\n",++wcnt,l,Dis,Dcs);
else SubCheckOutPin(pD->pPinTSO);

fflush(fd);
}

//------------------------------------------------------------------------------

void DS_integ::SubCheckSup(SupT_t * pS)
// Static checks on Supervisor device type
{
string Ss = pS->FullName();            // Stack movements. Think about it.
const char * Scs = Ss.c_str();         // Device type name
static const char * Sis = "Supervisor type"; // Save some typing
unsigned ls = pS->Def();                // Definition line
                                       // Supervisor-specific stuff
if (pS->pOnPkt==0) fprintf(fd,"W%3u. (%3u) "
  "%s %s has no specified packet handler\n",++wcnt,ls,Scs,Sis);
else
  if (pS->pOnPkt->Size()==0) fprintf(fd,"W%3u. (%3u) "
    "%s %s has zero-sized packet handler\n",++wcnt,ls,Scs,Sis);
if (pS->pOnRTCL==0) fprintf(fd,"W%3u. (%3u) "
  "%s %s has no specified RTCL handler\n",++wcnt,ls,Scs,Sis);
else
  if (pS->pOnRTCL->Size()==0) fprintf(fd,"W%3u. (%3u) "
    "%s %s has zero-sized RTCL handler\n",++wcnt,ls,Scs,Sis);
if (pS->pOnStop==0) fprintf(fd,"W%3u. (%3u) "
  "%s %s has no specified OnStop handlert\n",++wcnt,ls,Scs,Sis);
else
  if (pS->pOnStop->Size()==0)fprintf(fd,"W%3u. (%3u) "
    "%s %s has zero-sized OnStop handler\n",++wcnt,ls,Scs,Sis);
if (pS->pOnCTL==0) fprintf(fd,"W%3u. (%3u) "
  "%s %s has no specified OnCTL handlert\n",++wcnt,ls,Scs,Sis);
else
  if (pS->pOnCTL->Size()==0)fprintf(fd,"W%3u. (%3u) "
    "%s %s has zero-sized OnCTL handler\n",++wcnt,ls,Scs,Sis);
                                       // Base class device type stuff
DevT_t * pD = dynamic_cast<DevT_t *>(pS);
if (pD==0) fprintf(fd,"E%3u. (%3u) "
    "%s %s ... dynamic cast to SupT_t failed?????\n",++ecnt,ls,Scs,Sis);
else SubCheckDevType(pD);
fflush(fd);
}

//==============================================================================

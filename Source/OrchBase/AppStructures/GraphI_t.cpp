
//------------------------------------------------------------------------------

#include "GraphI_t.h"
#include "GraphT_t.h"
#include "Apps_t.h"
#include "PinI_t.h"
#include "PinT_t.h"
#include "EdgeI_t.h"
#include "CFrag.h"
#include "DevI_t.h"
#include "Meta_t.h"
#include "OrchBase.h"
#include "Pglobals.h"
#include <set>

//==============================================================================

GraphI_t::GraphI_t(Apps_t * _p,string _s):
    par(_p),deployed(false),pT(0),pPropsI(0),pSup(0),built(0)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent

G.SetPD_CB(PinI_t::PinDat_cb);         // Debug callbacks for the graph build
G.SetPK_CB(PinI_t::PinKey_cb);
G.SetND_CB(DevI_t::DevDat_cb);
G.SetNK_CB(DevI_t::DevKey_cb);
G.SetAD_CB(EdgeI_t::EdgDat_cb);
G.SetAK_CB(EdgeI_t::EdgKey_cb);
}

//------------------------------------------------------------------------------

GraphI_t::~GraphI_t()
{
                                       // Delete all the devices in the graph
WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,G,i)
  delete G.NodeData(i);
                                       // Delete all the edges in the graph
WALKPDIGRAPHARCS(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,G,i)
  delete G.ArcData(i);

set<PinI_t *> DelSet;                  // "To be deleted" set
                                       // Walk all the pins, adding to the set
WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,G,i) {
  WALKPDIGRAPHINPINS(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,
                     G,G.NodeKey(i),j) DelSet.insert(G.PinData(j));
  WALKPDIGRAPHOUTPINS(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,
                     G,G.NodeKey(i),j) DelSet.insert(G.PinData(j));
}
WALKSET(PinI_t *,DelSet,i) delete *i;  // Kill the "to be deleted" set
                                       // Lose any metadata
WALKVECTOR(Meta_t *,Meta_v,i) delete *i;

if (pPropsI!=0) delete pPropsI;

/* Freeing supervisors is a litle complicated, as they can be deleted when
 * their GraphI_t object is deleted, and can also be deleted when placement is
 * destroyed, so beware. */
if (pSup != PNULL) delete pSup;
}

//------------------------------------------------------------------------------

void GraphI_t::DevicesOfType(DevT_t * type,vector<DevI_t *>& output)
// Populate `output` with all devices in this graph instance that have a device
// type matching `type`.
{
output.clear();
WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,G,i)
  if (i!=G.NodeEnd()) {
    DevI_t * pD = G.NodeData(i);
    if (pD->pT==type) output.push_back(pD);
  }
}

//------------------------------------------------------------------------------

void GraphI_t::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n%sGraphI_t +++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sNameBase       %s\n",os,FullName().c_str());
fprintf(fp,"%sMe,Parent      0x%#018lx,0x%#018lx\n",
           os,(uint64_t)this,(uint64_t)par);
if (par!=0) fprintf(fp,"%s...%s\n",os,par->FullName().c_str());
fprintf(fp,"%sDEVICE GRAPH + + + + + + + + + + + + + + + + + + + + + + +\n",os);
G.DumpChan(fp);                        // OP channel for callbacks
G.Dump();
fprintf(fp,"%sDEVICE GRAPH - - - - - - - - - - - - - - - - - - - - - - -\n",os);
fprintf(fp,"%sDEVICE GRAPH VERTICES (P_device) + + + + + + + + + + + + +\n",os);
WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,G,i)
  if (i!=G.NodeEnd()) {
    DevI_t * pD = G.NodeData(i);
    if (pD!=0) pD->Dump(off+2,fp);
    else fprintf(fp,"%sNo P_device data\n",os);
  }
fprintf(fp,"%sDEVICE GRAPH VERTICES (P_device) - - - - - - - - - - - - -\n",os);
fprintf(fp,"%sDEVICE GRAPH ARCS (P_message) + + + + + + + + + + + + + + \n",os);
WALKPDIGRAPHARCS(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,G,i)
  if (i!=G.ArcEnd()) {
    EdgeI_t * pL = G.ArcData(i);
    if (pL!=0) pL->Dump(off+2,fp);
    else fprintf(fp,"%sNo EdgeI_t data\n",os);
  }
fprintf(fp,"%sDEVICE GRAPH ARCS (P_message) - - - - - - - - - - - - - - \n",os);
fprintf(fp,"%sTarget type tree from file %s\n",os,tyId.c_str());
fprintf(fp,"%sMonkey mutable type tree   %s\n",os,tyId2.c_str());
fprintf(fp,"%sType cross-link :  %#018lx\n",os,(uint64_t)pT);
if (pT!=0) fprintf(fp,"%s... -> %s\n",os,pT->FullName().c_str());
fprintf(fp,"%sProperties initialiser %#018lx\n",os,(uint64_t)pPropsI);
if (pPropsI!=0) pPropsI->Dump(off+2,fp);
fprintf(fp,"%sDevice name : key map has %lu entries :\n",os,Dmap.size());
WALKMAP(string,unsigned,Dmap,i)
  fprintf(fp,"%s%s : %u\n",os,(*i).first.c_str(),(*i).second);
fprintf(fp,"%sMetadata vector has %lu entries:\n",os,Meta_v.size());
WALKVECTOR(Meta_t *,Meta_v,i) (*i)->Dump(off+2,fp);
NameBase::Dump(off+2,fp);
fprintf(fp,"%sGraphI_t dump ------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

DevI_t * GraphI_t::GetDevice(string & rname)
// Search the graph *by name* for a device.
// If there isn't one there already, make one.
{
unsigned key = Dmap[rname];            // Device name
DevI_t ** ppD = G.FindNode(key);       // Device key
DevI_t * pD = 0;                       // RAII
if (ppD!=0) {                          // Not in graph?
  pD = *ppD;                           // Yes - get device address
  if (pD!=0) return pD;                // And hand it back
  FILE * fd = par->par->fd;            // Unrecoverable: detail output file
  fprintf(fd,"U. Device %s has *entry* in GraphI_t::G, but zero address\n",
          rname.c_str());
}
pD = new DevI_t(this,rname);           // No sign of it - make one
pD->devTyp = 'U';                      // As yet....
return pD;
}

//------------------------------------------------------------------------------

unsigned GraphI_t::TypeLink()
// Typelink a graph. If it's already typelinked, kill the links and carry on.
// Note that if *this* attempted link fails, the TLinks will be consequently
// cleared and the whole thing left unlinked.
{
FILE * fd = par->par->fd;              // Detail output file
fprintf(fd,"%s\nTypelinking graph instance %s (defined file %s line %u)\n\n",
           Q::dline.c_str(),FullName().c_str(),par->filename.c_str(),Def());
unsigned lecnt = 0;                    // Local error count
if (TLink()) UnTLink();                // If it is, unlink it
pT = par->FindTree(tyId);              // Go find type tree
if (pT==0) {                           // It *should* be there?
  fprintf(fd," %3u. Type tree %s not found\n",++lecnt,tyId.c_str());
  return lecnt;
}                                      // So far, so good
pT->GraphI_v.push_back(this);          // Link at graph level
pT->Ref(Def());
                                       // Walk device instances
WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,G,i) {
  DevI_t * pD = G.NodeData(i);         // Device instance
  pD->pT = pT->FindDev(pD->tyId);
  if (pD->pT==0)
    fprintf(fd,"E%3u. No device type definition \"%s\" found"
               " for device \"%s\" (defined line %u)\n",
               ++lecnt,pD->tyId.c_str(),pD->FullName().c_str(),pD->Def());
  else pD->pT->Ref(pD->Def());
                                       // Now walk input pins on device
  WALKPDIGRAPHINPINS(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,
                     G,G.NodeKey(i),j){
    PinI_t * pP = G.PinData(j);        // Get the pin from the iterator
    pP->pT = pT->FindPin(pD->pT,pP->Name());
    if (pP->pT==0)
      fprintf(fd,"E%3u. No pin name definition \"%s\" found"
              " for device pin \"%s\" (defined line %u)\n",
              ++lecnt,pP->Name().c_str(),pP->FullName().c_str(),pP->Def());
    else pP->pT->Ref(pP->Def());
  } // End WALKPDIGRAPHINPINS
                                       // And again for the output pins
  WALKPDIGRAPHOUTPINS(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,
                      G,G.NodeKey(i),j) {
    PinI_t * pP = G.PinData(j);        // Get the pin from the iterator
    pP->pT = pT->FindPin(pD->pT,pP->Name());
    if (pP->pT==0)
      fprintf(fd,"E%3u. No pin name definition \"%s\" found"
              " for device pin \"%s\" (defined line %u)\n",
              ++lecnt,pP->Name().c_str(),pP->FullName().c_str(),pP->Def());
    else pP->pT->Ref(pP->Def());
  } // End WALKPDIGRAPHOUTPINS
} // End WALKPDIGRAPHNODES
if (lecnt!=0) UnTLink();
fprintf(fd,"\n%s\n",Q::dline.c_str());
fflush(fd);
return lecnt;
}

//------------------------------------------------------------------------------

void GraphI_t::UndefDevs()
// Called at the END of a GraphInstance definition. There are three distinct
// things done here:
// Walk through the graph, and
// 1. Bleat if there are any devices referenced but not defined. User error.
// 2. Check that the sizes of the scaffold maps are consistent with the digraph
// 3. Delete the scaffold maps (they just contain a *copy* of the pin names, so
// they can add up to quite a bit).
{
FILE * fd = par->par->fd;              // Detail output file
unsigned cnt = 0;                      // (User) error count
                                       // Walk the nodes
WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,G,i) {
  DevI_t * pD = G.NodeData(i);
  if (pD->Def()==0) {                  // Is it defined?
    fprintf(fd,"E%3u. Device %s is not defined but referenced on line(s)\n",
                ++cnt,pD->FullName().c_str());
    vector<unsigned> v = *pRef();
    unsigned l = 0;
    WALKVECTOR(unsigned,v,j) fprintf(fd,"       %u.%u : %u\n",cnt,++l,*j);
  }
  unsigned pins = 0;                   // Walk the scaffold map
  WALKMAP(string,PinI_t *,pD->Pmap,j)
    pins += (*j).second->Key_v.size(); // Integrate pin count
  unsigned kD = G.NodeKey(i);
  if (pins != G.SizeInPins(kD) + G.SizeOutPins(kD))
    fprintf(fd,"U. Device %s has scaffolding for %u pins "
               "but only %u input pins and %u output pins in the graph\n",
               pD->FullName().c_str(),pins,G.SizeInPins(kD),G.SizeOutPins(kD));
                                       // Either way, we're done with scaffold
  pD->Pmap.clear();
}
}

//------------------------------------------------------------------------------

void GraphI_t::UnTLink()
// Mindlessly kill all the typelinks for this graph
{
if (pT!=0)                             // Graph-level link
  WALKVECTOR(GraphI_t *,pT->GraphI_v,i)// Walk backlink vector in graphT
    if ((*i)==this) {                  // If it's to me....
      pT->GraphI_v.erase(i);           // Kill it and.....
      pT->clRef1(pT->Def());           // Remove reference
      break;                           // bail - loop iterators are now invalid
    }
pT = 0;                                // Reset GraphInstance->Type tree pointer

WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,G,i) {
  DevI_t * pD = G.NodeData(i);         // Device address
  if (pD->pT != 0)
  {
    pD->pT->clRef1(pD->Def());
    pD->pT = 0;                        // Device instance type
  }
                                       // Input pin type
  WALKPDIGRAPHINPINS(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,
                     G,G.NodeKey(i),j) {
    PinI_t * pP =G.PinData(j);
    if (pP->pT!=0) pP->pT->clRef1(pP->Def());
    pP->pT = 0;
  }
                                       // And again for the output pins
  WALKPDIGRAPHOUTPINS(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,
                      G,G.NodeKey(i),j) {
    PinI_t * pP =G.PinData(j);
    if (pP->pT!=0) pP->pT->clRef1(pP->Def());
    pP->pT = 0;
  }
} // End WALKPDIGRAPHNODES
}

//==============================================================================
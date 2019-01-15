//------------------------------------------------------------------------------

#include "PoetsEngine.h"
#include "Pglobals.h"
#include "Cli.h"
#include "OrchBase.h"
#include "Config_t.h"
#include <climits>

//==============================================================================

PoetsEngine::P_graph(OrchBase * _p,string _s):par(_p)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent

Config_t * pC = new Config_t(this,"ConfigXXX");
pConfigl.push_back(pC);                   // TEMP: box configuration stuff

G.SetPD_CB(P_port::PrtDat_cb);          // Debug callbacks for the graph build
G.SetPK_CB(P_port::PrtKey_cb);
G.SetND_CB(PoetsBox::BoxDat_cb);
G.SetNK_CB(PoetsBox::BoxKey_cb);
G.SetAD_CB(P_link::LnkDat_cb);
G.SetAK_CB(P_link::LnkKey_cb);
}

//------------------------------------------------------------------------------

PoetsEngine::~P_graph()
{
Clear();
WALKLIST(Config_t *,pConfigl,i) delete *i;
}


//------------------------------------------------------------------------------

void PoetsEngine::Clear()
// Remove all the data - dynamic and topological - from the hardware graph
{
par->UnlinkAll();                      // Disconnect any/all tasks
                                       // Delete all the boxes in the task
WALKPDIGRAPHNODES(unsigned,PoetsBox *,unsigned,P_link *,unsigned,P_port *,G,i)
  if (i!=G.NodeEnd()) delete G.NodeData(i);
                                       // Delete all the links in the graph
WALKPDIGRAPHARCS(unsigned,PoetsBox *,unsigned,P_link *,unsigned,P_port *,G,i)
  if (i!=G.ArcEnd()) delete G.ArcData(i);

G.Clear();                             // Destroy the topology itself

}

//------------------------------------------------------------------------------

void PoetsEngine::Cm(Cli *)
{
}

//------------------------------------------------------------------------------

void PoetsEngine::Dump(FILE * fp)
{
string s = FullName();
fprintf(fp,"PoetsEngine dump %35s++++++++++++++++++++++++++++++++\n",s.c_str());
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      0x%#08p,0x%#08p\n",this,par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
fprintf(fp,"BOX CONFIGURATION OBJECTS %35s+++++++++++++++++++\n",s.c_str());
WALKLIST(Config_t *,pConfigl,i) (*i)->Dump();
fprintf(fp,"BOX CONFIGURATION OBJECTS %35s-------------------\n",s.c_str());
fprintf(fp,"BOX GRAPH %35s+++++++++++++++++++++++++++++++++++\n",s.c_str());
G.DumpChan(fp);
G.Dump();
fprintf(fp,"BOX GRAPH %35s-----------------------------------\n",s.c_str());
fprintf(fp,"BOX GRAPH VERTICES (PoetsBox) %35s++++++++++++++++++\n",s.c_str());
WALKPDIGRAPHNODES(unsigned,PoetsBox *,unsigned,P_link *,unsigned,P_port *,G,i)
  if (i!=G.NodeEnd()) {
    PoetsBox * pB = G.NodeData(i);
    if (pB!=0) pB->Dump(fp);
    else fprintf(fp,"No PoetsBox data\n");
  }
fprintf(fp,"BOX GRAPH VERTICES (PoetsBox) %35s------------------\n",s.c_str());
fprintf(fp,"BOX GRAPH ARCS (P_link) %35s+++++++++++++++++++++\n",s.c_str());
WALKPDIGRAPHARCS(unsigned,PoetsBox *,unsigned,P_link *,unsigned,P_port *,G,i)
  if (i!=G.ArcEnd()) {
    P_link * pL = G.ArcData(i);
    if (pL!=0) pL->Dump(fp);
    else fprintf(fp,"No P_link data\n");
  }
fprintf(fp,"BOX GRAPH ARCS (P_link) %35s---------------------\n",s.c_str());
NameBase::Dump(fp);
fprintf(fp,"PoetsEngine dump %35s--------------------------------\n",s.c_str());
fflush(fp);
}

//------------------------------------------------------------------------------

bool PoetsEngine::IsEmpty()
{
return G.SizeNodes()==0 ? true : false;
}

//------------------------------------------------------------------------------

void PoetsEngine::SetN(unsigned numBoxes, bool vSys)
// Reset the topology graph, and recreate it with exactly and only one node
// (box)
{
Clear();                               // Lose any existing structure
// no effective memory limit on a virtual board
// note that an unsigned int may be inadequate to express the memory limit
// even in real systems!
if (vSys) (*pConfigl.begin())->SetBMem(UINT_MAX);
for (unsigned B = 0; B < numBoxes; B++)
{
PoetsBox * pB = new P_box(this);          // New box
pB->Npar(this);                        // Namebase parent
pB->AutoName("Bx");                    // Derive the name off the uid
pB->addr.SetBox(B);                  // Ordinal address number
pB->vBox=vSys;                         // Is this box real hardware?
pB->Build(*pConfigl.begin());          // Assemble the hardware (model)
G.InsertNode(pB->Id(),pB);             // Insert the box
}
par->pPlace->Init();                   // Initialise the placement system
}

//------------------------------------------------------------------------------

//==============================================================================




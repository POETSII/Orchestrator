//------------------------------------------------------------------------------

#include "P_graph.h"
#include "Pglobals.h"
#include "Cli.h"
#include "OrchBase.h"
#include "Config_t.h"
#include "stdint.h"
#include <climits>

//==============================================================================

P_graph::P_graph(OrchBase * _p,string _s):par(_p)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent

Config_t * pC = new Config_t(this,"ConfigXXX");
pConfigl.push_back(pC);                   // TEMP: box configuration stuff

G.SetPD_CB(P_port::PrtDat_cb);          // Debug callbacks for the graph build
G.SetPK_CB(P_port::PrtKey_cb);
G.SetND_CB(P_box::BoxDat_cb);
G.SetNK_CB(P_box::BoxKey_cb);
G.SetAD_CB(P_link::LnkDat_cb);
G.SetAK_CB(P_link::LnkKey_cb);
}

//------------------------------------------------------------------------------

P_graph::~P_graph()
{
Clear();
WALKLIST(Config_t *,pConfigl,i) delete *i;
}


//------------------------------------------------------------------------------

void P_graph::Clear()
// Remove all the data - dynamic and topological - from the hardware graph
{
par->UnlinkAll();                      // Disconnect any/all tasks
                                       // Delete all the boxes in the task
WALKPDIGRAPHNODES(unsigned,P_box *,unsigned,P_link *,unsigned,P_port *,G,i)
  if (i!=G.NodeEnd()) delete G.NodeData(i);
                                       // Delete all the links in the graph
WALKPDIGRAPHARCS(unsigned,P_box *,unsigned,P_link *,unsigned,P_port *,G,i)
  if (i!=G.ArcEnd()) delete G.ArcData(i);

G.Clear();                             // Destroy the topology itself

}

//------------------------------------------------------------------------------

void P_graph::Cm(Cli *)
{
}

//------------------------------------------------------------------------------

void P_graph::Dump(FILE * fp)
{
string s = FullName();
fprintf(fp,"P_graph dump %35s++++++++++++++++++++++++++++++++\n",s.c_str());
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      %#018lx,%#018lx\n",
        (uint64_t) this, (uint64_t) par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
fprintf(fp,"BOX CONFIGURATION OBJECTS %35s+++++++++++++++++++\n",s.c_str());
WALKLIST(Config_t *,pConfigl,i) (*i)->Dump();
fprintf(fp,"BOX CONFIGURATION OBJECTS %35s-------------------\n",s.c_str());
fprintf(fp,"BOX GRAPH %35s+++++++++++++++++++++++++++++++++++\n",s.c_str());
G.DumpChan(fp);
G.Dump();
fprintf(fp,"BOX GRAPH %35s-----------------------------------\n",s.c_str());
fprintf(fp,"BOX GRAPH VERTICES (P_box) %35s++++++++++++++++++\n",s.c_str());
WALKPDIGRAPHNODES(unsigned,P_box *,unsigned,P_link *,unsigned,P_port *,G,i)
  if (i!=G.NodeEnd()) {
    P_box * pB = G.NodeData(i);
    if (pB!=0) pB->Dump(fp);
    else fprintf(fp,"No P_box data\n");
  }
fprintf(fp,"BOX GRAPH VERTICES (P_box) %35s------------------\n",s.c_str());
fprintf(fp,"BOX GRAPH ARCS (P_link) %35s+++++++++++++++++++++\n",s.c_str());
WALKPDIGRAPHARCS(unsigned,P_box *,unsigned,P_link *,unsigned,P_port *,G,i)
  if (i!=G.ArcEnd()) {
    P_link * pL = G.ArcData(i);
    if (pL!=0) pL->Dump(fp);
    else fprintf(fp,"No P_link data\n");
  }
fprintf(fp,"BOX GRAPH ARCS (P_link) %35s---------------------\n",s.c_str());
NameBase::Dump(fp);
fprintf(fp,"P_graph dump %35s--------------------------------\n",s.c_str());
fflush(fp);
}

//------------------------------------------------------------------------------

bool P_graph::IsEmpty()
{
return G.SizeNodes()==0 ? true : false;
}

//------------------------------------------------------------------------------

void P_graph::SetN(unsigned numBoxes, bool vSys)
// Reset the topology graph, and recreate it with exactly and only one node
// (box)
{
Clear();                               // Lose any existing structure
// no effective memory limit on a virtual board
if (vSys) (*pConfigl.begin())->SetBMem(ULLONG_MAX);
for (unsigned B = 0; B < numBoxes; B++)
{
P_box * pB = new P_box(this);          // New box
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




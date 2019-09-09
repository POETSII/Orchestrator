//------------------------------------------------------------------------------

#include "GraphI_t.h"
#include "Apps_t.h"
#include "PinI_t.h"
#include "DevI_t.h"

//==============================================================================

GraphI_t::GraphI_t(Apps_t * _p,string _s):par(_p),pT(0),pPropsI(0)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent

G.SetPD_CB(PinI_t::PinDat_cb);         // Debug callbacks for the graph build
G.SetPK_CB(PinI_t::PinKey_cb);
G.SetND_CB(DevI_t::DevDat_cb);
G.SetNK_CB(DevI_t::DevKey_cb);
G.SetAD_CB(P_link::LnkDat_cb);
G.SetAK_CB(P_link::LnkKey_cb);
}

//------------------------------------------------------------------------------

GraphI_t::~GraphI_t()
{
                                       // Delete all the devices in the graph
WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,P_link *,unsigned,PinI_t *,G,i)
//  if (i!=G.NodeEnd())
  delete G.NodeData(i);

WALKPDIGRAPHARCS(unsigned,DevI_t *,unsigned,P_link *,unsigned,PinI_t *,G,i)
//  if (i!=G.NodeEnd())
  delete G.ArcData(i);

WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,P_link *,unsigned,PinI_t *,G,i) {
  WALKPDIGRAPHINPINS(unsigned,DevI_t *,unsigned,P_link *,unsigned,PinI_t *,
//                     G,G.NodeKey(i),j) if (G.PinData(j)!=0) delete G.PinData(j);
                     G,G.NodeKey(i),j) PinI_t::DelSet.insert(G.PinData(j));
  WALKPDIGRAPHOUTPINS(unsigned,DevI_t *,unsigned,P_link *,unsigned,PinI_t *,
//                     G,G.NodeKey(i),j) if (G.PinData(j)!=0) delete G.PinData(j);
                     G,G.NodeKey(i),j) PinI_t::DelSet.insert(G.PinData(j));
}

PinI_t::Delete();
WALKVECTOR(Meta_t *,Meta_v,i) delete *i;














                                       // Delete all the links


                                       // Delete all the messages in the graph
//WALKPDIGRAPHARCS(unsigned,P_device *,unsigned,P_message *,unsigned,PinI_t *,G,i)
//  if (i!=G.ArcEnd()) delete G.ArcData(i);

// Note that this does not touch the graph topology, just the POETS data
// hanging off it. The graph itself - G - is automatic in the class.
/*intptr_t msg_ptr;
map<intptr_t, P_message*> unique_msgs;
// crashes here - if this is removed the application exits.
WALKPDIGRAPHARCS(unsigned,P_device *,unsigned,P_message *,unsigned,PinI_t *,G,i)
{
  if (i!=G.ArcEnd())
  {
      msg_ptr = reinterpret_cast<intptr_t>(G.ArcData(i));
      if (unique_msgs.find(msg_ptr) == unique_msgs.end()) unique_msgs[msg_ptr] = G.ArcData(i);
  }
}
//   if (i!=G.ArcEnd()) delete G.ArcData(i);
for (map<intptr_t, P_message*>::iterator del_msg = unique_msgs.begin(); del_msg != unique_msgs.end(); del_msg++) delete (*del_msg).second;
*/
}

//------------------------------------------------------------------------------

void GraphI_t::Dump(FILE * fp)
{
string s = FullName();
fprintf(fp,"GraphI_t %35s+++++++++++++++++++++++++++++++++++++\n",s.c_str());
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      0x%#08p,0x%#08p\n",this,par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
//fprintf(fp,"P_graph shortcut %#08p\n",pP);
//if (pP!=0) fprintf(fp,"...%s\n",pP->FullName().c_str());
fprintf(fp,"C Initialiser %#08p\n",pPropsI);
if (pPropsI!=0) pPropsI->Dump(fp);
fprintf(fp,"Target type tree from file %s\n",tyId.c_str());
fprintf(fp,"Monkey mutable type tree   %s\n",tyId2.c_str());
fprintf(fp,"DEVICE GRAPH %35s++++++++++++++++++++++++++++++++\n",s.c_str());
G.DumpChan(fp);                        // OP channel for callbacks
G.Dump();
fprintf(fp,"DEVICE GRAPH %35s--------------------------------\n",s.c_str());
fprintf(fp,"DEVICE GRAPH VERTICES (P_device) %35s++++++++++++\n",s.c_str());
WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,P_link *,unsigned,PinI_t *,G,i)
  if (i!=G.NodeEnd()) {
    DevI_t * pD = G.NodeData(i);
    if (pD!=0) pD->Dump(fp);
    else fprintf(fp,"No P_device data\n");
  }
fprintf(fp,"DEVICE GRAPH VERTICES (P_device) %35s------------\n",s.c_str());
fprintf(fp,"DEVICE GRAPH ARCS (P_message) %35s+++++++++++++++\n",s.c_str());
WALKPDIGRAPHARCS(unsigned,DevI_t *,unsigned,P_link *,unsigned,PinI_t *,G,i)
  if (i!=G.ArcEnd()) {
    P_link * pL = G.ArcData(i);
    if (pL!=0) pL->Dump(fp);
    else fprintf(fp,"No P_link data\n");
  }
fprintf(fp,"DEVICE GRAPH ARCS (P_message) %35s---------------\n",s.c_str());
NameBase::Dump(fp);
fprintf(fp,"GraphI_t dump %35s--------------------------------\n",s.c_str());
fflush(fp);
}

//------------------------------------------------------------------------------

vector<DevI_t *> GraphI_t::DevicesOfType(const DevT_t * d_type)
// ADB:: Isn't this going to shove a *massive* vector onto the stack?
{
     vector<DevI_t *> devices;
     WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,P_link *,unsigned,PinI_t *,G,i)
     {
        if (i!=G.NodeEnd() && (G.NodeData(i)->pT == d_type)) devices.push_back(G.NodeData(i));
     }
     return devices;
}

//==============================================================================




//------------------------------------------------------------------------------

#include "D_graph.h"
#include "P_task.h"
#include "P_pin.h"
#include "P_device.h"
//#include <cinttypes>

//==============================================================================

D_graph::D_graph(P_task * _p,string _s):par(_p),pP(0),pD(0),pPropsI(0)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent

G.SetPD_CB(P_pin::PinDat_cb);          // Debug callbacks for the graph build
G.SetPK_CB(P_pin::PinKey_cb);
G.SetND_CB(P_device::DevDat_cb);
G.SetNK_CB(P_device::DevKey_cb);
G.SetAD_CB(P_message::MsgDat_cb);
G.SetAK_CB(P_message::MsgKey_cb);
}

//------------------------------------------------------------------------------

D_graph::~D_graph()
{
                                       // Delete all the devices in the task
WALKPDIGRAPHNODES(unsigned,P_device *,unsigned,P_message *,unsigned,P_pin *,G,i)
  if (i!=G.NodeEnd()) delete G.NodeData(i);
                                       // Delete all the messages in the graph
//WALKPDIGRAPHARCS(unsigned,P_device *,unsigned,P_message *,unsigned,P_pin *,G,i)
//  if (i!=G.ArcEnd()) delete G.ArcData(i);

// Note that this does not touch the graph topology, just the POETS data
// hanging off it. The graph itself - G - is automatic in the class.
/*intptr_t msg_ptr;
map<intptr_t, P_message*> unique_msgs;
// crashes here - if this is removed the application exits.
WALKPDIGRAPHARCS(unsigned,P_device *,unsigned,P_message *,unsigned,P_pin *,G,i)
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

void D_graph::Dump(FILE * fp)
{
string s = FullName();
fprintf(fp,"D_graph %35s+++++++++++++++++++++++++++++++++++++\n",s.c_str());
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      %#018lx,%#018lx\n",
        (uint64_t) this, (uint64_t) par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
fprintf(fp,"P_graph shortcut %#018lx\n", (uint64_t) pP);
if (pP!=0) fprintf(fp,"...%s\n",pP->FullName().c_str());
fprintf(fp,"C Initialiser %#018lx\n", (uint64_t) pPropsI);
if (pPropsI!=0) pPropsI->Dump(fp);
fprintf(fp,"DEVICE GRAPH %35s++++++++++++++++++++++++++++++++\n",s.c_str());
G.DumpChan(fp);                        // OP channel for callbacks
G.Dump();
fprintf(fp,"DEVICE GRAPH %35s--------------------------------\n",s.c_str());
fprintf(fp,"DEVICE GRAPH VERTICES (P_device) %35s++++++++++++\n",s.c_str());
WALKPDIGRAPHNODES(unsigned,P_device *,unsigned,P_message *,unsigned,P_pin *,G,i)
  if (i!=G.NodeEnd()) {
    P_device * pD = G.NodeData(i);
    if (pD!=0) pD->Dump(fp);
    else fprintf(fp,"No P_device data\n");
  }
fprintf(fp,"DEVICE GRAPH VERTICES (P_device) %35s------------\n",s.c_str());
fprintf(fp,"DEVICE GRAPH ARCS (P_message) %35s+++++++++++++++\n",s.c_str());
WALKPDIGRAPHARCS(unsigned,P_device *,unsigned,P_message *,unsigned,P_pin *,G,i)
  if (i!=G.ArcEnd()) {
    P_message * pM = G.ArcData(i);
    if (pM!=0) pM->Dump(fp);
    else fprintf(fp,"No P_message data\n");
  }
fprintf(fp,"DEVICE GRAPH ARCS (P_message) %35s---------------\n",s.c_str());
NameBase::Dump(fp);
fprintf(fp,"D_graph dump %35s--------------------------------\n",s.c_str());
fflush(fp);
}

//------------------------------------------------------------------------------

vector<P_device*> D_graph::DevicesOfType(const P_devtyp* d_type)
{
     vector<P_device*> devices;
     WALKPDIGRAPHNODES(unsigned,P_device *,unsigned,P_message *,unsigned,P_pin *,G,i)
     {
        if (i!=G.NodeEnd() && (G.NodeData(i)->pP_devtyp == d_type)) devices.push_back(G.NodeData(i));
     }
     return devices;
}

//==============================================================================




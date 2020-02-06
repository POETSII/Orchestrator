//==============================================================================

#include "macros.h"

//==============================================================================
  /*
PDIGRAPH_ bool _PDIGRAPH::ArcNodePin(AKT key_a,NKT key_n,PKT & key_p, bool & dir)
// Routine to return the pin joining arc 'key_a' to node 'key_n' (in 'key_p'),
// and the direction of the arc ( in 'dir'==false -> arc goes FROM node).
// We can't return the result in the normal way, because it might not exist,
// and there's no 'null' value for iterators. So success/failure is indicated
// by the return bool, and the answers written into 'key_p' and 'dir'.
// If the evaluation fails for any reason, the function returns 'false' and the
// value of 'key_p' and 'dir' are left alone.
{
TPa_it a = index_a.find(key_a);        // Find the arc
if (a==index_a.end()) return false;    // If not there ...
TPn_it n = index_n.find(key_n);        // Find the node
if (n==index_n.end()) return false;    // If not there ...
                                       // Are they connected?
if (((*a)._2.fr_n!=n)&&((*a)._2.to_n!=n)) return false;
dir = (*a)._2.fr_n == n;           // Yes; is it a 'from' arc?
                                       // Go get the pin key
key_p = dir ? (*((*a)._2.fr_p))._1 : (*((*a)._2.to_p))._1;
return true;
}   */

//------------------------------------------------------------------------------

PDIGRAPH_ _PDIGRAPH::pdigraph(void)
{
cb_struct x = {0,0,0,0,0,0};
cb          = x;                       // No callbacks to start
N_dirty     = true;                    // No random access iterators valid
dfp         = stdout;                  // Pretty-print default output stream
}

//------------------------------------------------------------------------------

PDIGRAPH_ _PDIGRAPH::pdigraph(_PDIGRAPH & rG)
{
cb          = rG.cb;                   // Callbacks
N_dirty     = true;                    // No random access iterators valid
dfp         = rG.dfp;                  // Pretty-print output stream

WALKPDIGRAPHNODES(NKT,NT,AKT,AT,PKT,PT,rG,i)
  InsertNode(rG.NodeKey(i),rG.NodeData(i));
WALKPDIGRAPHARCS(NKT,NT,AKT,AT,PKT,PT,rG,i) {
  AKT ak = rG.ArcKey(i);
//  AT  ad = rG.ArcData(i);
  NKT nk0,nk1;
  rG.FindNodes(ak,nk0,nk1);
//  NT * pnd0 = rG.FindNode(nk0);
//  NT * pnd1 = rG.FindNode(nk1);
  PKT pk0,pk1;
  rG.FindPins(ak,pk0,pk1);
  PT * ppd0 = rG.FindPin(nk0,pk0);
  PT * ppd1 = rG.FindPin(nk1,pk1);
  InsertArc(ak,nk0,nk1,rG.ArcData(i),pk0,*ppd0,pk1,*ppd1);
}

}

//------------------------------------------------------------------------------

PDIGRAPH_ void _PDIGRAPH::Clear()
// Hose the thing
{
index_n.clear();                       // Node list
index_a.clear();                       // Arc list
N_dirty = true;                        // Node map has changed
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::DrivenNodes(const NKT & key_n,vector<NKT> & vN)
// Return a vector of node keys (vN) that are *driven* by the node key_n
{
vN.clear();                            // Clear the output vector
TPn_it n_it = index_n.find(key_n);     // Find the source node key
if(n_it==index_n.end()) return false;  // Is it there?

                                       // Fanout multimap iterator limits
TPp_it begin_it =  (*n_it)._2.fano.begin();
TPp_it end_it   =  (*n_it)._2.fano.end();
for (TPp_it i=begin_it; i!=end_it; i++) {
  TPa_it a_it  = (*i)._2.iArc;         // Driving arc
  TPn_it dn_it = (*a_it)._2.to_n;      // Driven node
  vN.push_back((*dn_it)._1);           // Driven node key
}
return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::DrivenNodes(const NKT & key_n,vector<PKT> & vP_fr,
         vector<AKT> & vA,vector<PKT> & vP_to,vector<NKT> & vN)
// Returns:
// A vector of node keys (vN) that are *driven by* the node key_n.
// A vector of pin keys (vP_fr) in the nodes key_n
// A vector of arc keys (vA) ....
// A vector of pin keys (vP_to) in the nodes in vN
{
vP_fr.clear();                            // Clear the output vectors
vA.clear();
vP_to.clear();
vN.clear();
TPn_it n_it = index_n.find(key_n);     // Find the source node key
if(n_it==index_n.end()) return false;  // Is it there?

                                       // Fanout multimap iterator limits
TPp_it begin_it =  (*n_it)._2.fano.begin();
TPp_it end_it   =  (*n_it)._2.fano.end();
for (TPp_it i=begin_it; i!=end_it; i++) {
  TPa_it a_it  = (*i)._2.iArc;         // Driven arc
  vA.push_back((*a_it)._1);            // Driven arc key
  TPn_it dr_it = (*a_it)._2.to_n;      // Driving node
  vN.push_back((*dr_it)._1);           // Driving node key
  TPp_it to_it = (*a_it)._2.to_p;      // Pin on driving node (key_n)
  vP_to.push_back((*to_it)._1);
  TPp_it fr_it = (*a_it)._2.fr_p;      // Pin on driven node
  vP_fr.push_back((*fr_it)._1);
}
return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::DrivingNodes(const NKT & key_n,vector<NKT> & vN)
// Return a vector of node keys (vN) that are *driving* the node key_n
{
vN.clear();                            // Clear the output vector
TPn_it n_it = index_n.find(key_n);     // Find the source node key
if(n_it==index_n.end()) return false;  // Is it there?

                                       // Fanout multimap iterator limits
TPp_it begin_it =  (*n_it)._2.fani.begin();
TPp_it end_it   =  (*n_it)._2.fani.end();
for (TPp_it i=begin_it; i!=end_it; i++) {
  TPa_it a_it  = (*i)._2.iArc;         // Driving arc
  TPn_it dn_it = (*a_it)._2.fr_n;      // Driven node
  vN.push_back((*dn_it)._1);           // Driven node key
}
return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::DrivingNodes(const NKT & key_n,vector<PKT> & vP_fr,
     vector<AKT> & vA,vector<PKT> & vP_to,vector<NKT> & vN)
// Returns:
// A vector of node keys (vN) that are *driving* the node key_n.
// A vector of pin keys (vP_fr) in the nodes in vN
// A vector of arc keys (vA) driving....
// A vector of pin keys (vP_to) in the node key_n
{
vP_fr.clear();                         // Clear the output vectors
vA.clear();
vP_to.clear();
vN.clear();
TPn_it n_it = index_n.find(key_n);     // Find the source node key
if(n_it==index_n.end()) return false;  // Is it there?

                                       // Fanout multimap iterator limits
TPp_it begin_it =  (*n_it)._2.fani.begin();
TPp_it end_it   =  (*n_it)._2.fani.end();
for (TPp_it i=begin_it; i!=end_it; i++) {
  TPa_it a_it  = (*i)._2.iArc;         // Driving arc
  vA.push_back((*a_it)._1);            // Driving arc key
  TPn_it dn_it = (*a_it)._2.fr_n;      // Driven node
  vN.push_back((*dn_it)._1);           // Driven node key
  TPp_it fr_it = (*a_it)._2.fr_p;      // Pin on driven node (key_n)
  vP_fr.push_back((*fr_it)._1);
  TPp_it to_it = (*a_it)._2.to_p;      // Pin on driving node
  vP_to.push_back((*to_it)._1);
}
return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ void _PDIGRAPH::Dump()
{
std::string s(35,'=');
fprintf(dfp,"Pdigraph topological dump %35s++++++++++++++++++++\n",s.c_str());
fprintf(dfp,"Node index (%u entries):\n",static_cast<unsigned>(index_n.size()));
int c1 = 1;
int c2;
WALKMAP(NKT,node,index_n,i) {
  const NKT & r_nk = (*i)._1;
  fprintf(dfp,"%4d (",c1++);
  DoNK_CB(r_nk);
  fprintf(dfp,":");
  DoND_CB((*i)._2.data);
  fprintf(dfp,")\n");
  fprintf(dfp,"  Fanin arcs (%u entries)\n",static_cast<unsigned>(((*i)._2).fani.size()));
  c2 = 1;
  WALKMULTIMAP(PKT,pin,((*i)._2).fani,j) {
    fprintf(dfp,"  %4d ^|",c2++);
    const AKT & r_ak = (*(*j)._2.iArc)._1;
    DoAK_CB(r_ak);
    fprintf(dfp,"~");
    DoAD_CB((*(*j)._2.iArc)._2.data);
    fprintf(dfp,"|^ \t-> ((");
    const PKT & r_pk = (*j)._1;
    DoPK_CB(r_pk);
    fprintf(dfp,"~");
    DoPD_CB((*j)._2.data);
    fprintf(dfp,"))\n");
  }
  fprintf(dfp,"  Fanout arcs (%u entries)\n",static_cast<unsigned>(((*i)._2).fano.size()));
  c2 = 1;
  WALKMULTIMAP(PKT,pin,((*i)._2).fano,j) {
   fprintf(dfp,"  %4d ((",c2++);
   DoPK_CB(PKT((*j)._1));
   fprintf(dfp,"~");
   DoPD_CB((*j)._2.data);
   fprintf(dfp,")) \t-> ^|");
   DoAK_CB(AKT((*(*j)._2.iArc)._1));
   fprintf(dfp,"~");
   DoAD_CB((*(*j)._2.iArc)._2.data);
   fprintf(dfp,"|^\n");
  }
  fprintf(dfp,"===\n\n");
}
c1 = 1;
fprintf(dfp,"Arc index (%u entries):\n",static_cast<unsigned>(index_a.size()));
WALKMAP(AKT,arc,index_a,i) {
  fprintf(dfp,"%4d (",c1++);     DoNK_CB(NKT((*(((*i)._2).fr_n))._1));
  fprintf(dfp,"~");              DoND_CB((*(((*i)._2).fr_n))._2.data);
  fprintf(dfp,")((");            DoPK_CB(PKT((*(((*i)._2).fr_p))._1));
  fprintf(dfp,"~");              DoPD_CB((*(((*i)._2).fr_p))._2.data);
  fprintf(dfp,"))\t->\n ^|");    DoAK_CB(AKT((*i)._1));
  fprintf(dfp,"~");              DoAD_CB(((*i)._2).data);
  fprintf(dfp,"|^ \t->\n ((");   DoPK_CB(PKT((*(((*i)._2).to_p))._1));
  fprintf(dfp,"~");              DoPD_CB((*(((*i)._2).to_p))._2.data);
  fprintf(dfp,"))(");            DoNK_CB(NKT((*(((*i)._2).to_n))._1));
  fprintf(dfp,"~");              DoND_CB((*(((*i)._2).to_n))._2.data);
  fprintf(dfp,")\n\n");
}
fprintf(dfp,"Pdigraph topological dump %35s--------------------\n",s.c_str());
}

//------------------------------------------------------------------------------

PDIGRAPH_ AT * _PDIGRAPH::FindArc(const AKT & key_a)
// Given the arc key, return the arc data address
{
typename map<AKT,arc>::iterator i = index_a.find(key_a);
return (i==index_a.end()) ? (AT *)0 : &((*i)._2.data);
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool  _PDIGRAPH::FindArcs(const NKT & n_k,const PKT & p_k,
                                   vector<AKT> & vI,vector<AKT> & vO)
// Given a node and a pin key, return any/all arc keys
{
vI.clear();
vO.clear();

if (FindNode(n_k)==0) return false;    // Node not there
if (FindPin(n_k,p_k)==0) return false; // Pin not there

                                       // Walk right there...
pair<TPp_it,TPp_it> pi = index_n[n_k].fano.equal_range(p_k);
for(typename multimap<PKT,pin>::iterator i=pi.first;i!=pi.second;i++)
  vO.push_back((*(*i).second.iArc).first);

pi = index_n[n_k].fani.equal_range(p_k);
for(typename multimap<PKT,pin>::iterator i=pi.first;i!=pi.second;i++)
  vI.push_back((*(*i).second.iArc).first);

return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::FindArcs(const NKT & n_k,
                                   vector<AKT> & vI,vector<AKT> & vO)
// Given the node key, overwrite references to vectors of the input and output
// arc keys
{
vI.clear();                            // Reset the output vectors
vO.clear();

TPn_it n_it = NodeKey2It(n_k);         // Key -> iterator
if (n_it==index_n.end()) return false; // Node not there
                                       // Fanin multimap iterator limits
TPp_it begin_it =  (*n_it)._2.fani.begin();
TPp_it end_it   =  (*n_it)._2.fani.end();
for (TPp_it i=begin_it; i!=end_it; i++) vI.push_back(ArcKey((*i)._2.iArc));

                                       // Fanout multimap iterator limits
begin_it =  (*n_it)._2.fano.begin();
end_it   =  (*n_it)._2.fano.end();
for (TPp_it i=begin_it; i!=end_it; i++) vO.push_back(ArcKey((*i)._2.iArc));
return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ NT * _PDIGRAPH::FindNode(const NKT & key_n)
// Given the node key, return the node data address
{
typename map<NKT,node>::iterator i = index_n.find(key_n);
return (i==index_n.end()) ? (NT *)0 : &((*i)._2.data);
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::FindNodes(const AKT & a_k,NKT & n_fr_k,NKT & n_to_k)
// Given the arc key, overwrite references to the two node keys
{
TPa_it a_it = ArcKey2It(a_k);          // Key -> iterator
if (a_it==index_a.end()) return false; // Arc not there

n_fr_k = NodeKey((*a_it)._2.fr_n);
n_to_k = NodeKey((*a_it)._2.to_n);
return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::FindNodes(const NKT & key_n,vector<NKT> & vN)
// Return a vector of node keys (vN) that are *attached to* the node key_n.
// This is a conflation of DrivenNodes(...) and DrivingNodes(...), with the
// additional feature that a node driving itself will only have one entry in
// the output vectors, not two.
{
vN.clear();                            // Clear the output vector
vector<NKT> vA,vB;
DrivenNodes(key_n,vN);                 // Collect all nodes
DrivingNodes(key_n,vA);
                                       // Get rid of any duplicate
WALKVECTOR(NKT,vA,i) if (*i!=key_n) vB.push_back(*i);
                                       // Merge the two vectors
vN.insert(vN.begin(),vB.begin(),vB.end());
return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::FindNodes(const NKT & key_n,vector<PKT> & vP_me,
         vector<AKT> & vA,vector<PKT> & vP_N,vector<NKT> & vN)
// See comments for FindNodes(...) above and Driven/DrivingNodes
{
vP_me.clear();                         // Clear the output vectors
vA.clear();
vP_N.clear();
vN.clear();
vector<PKT> vA_PKT,vB_PKT;
vector<AKT> vA_AKT,vB_AKT;
vector<PKT> vA_N_PKT,vB_N_PKT;
vector<NKT> vA_NKT,vB_NKT;
DrivenNodes(key_n,vP_me,vA,vP_N,vN);
DrivingNodes(key_n,vA_PKT,vA_AKT,vA_N_PKT,vA_NKT);
for(unsigned i=0;i<vA_NKT.size();i++) {
  if (vA_NKT[i]!=key_n) {
    vB_PKT.push_back(vA_PKT[i]);
    vB_AKT.push_back(vA_AKT[i]);
    vB_N_PKT.push_back(vA_N_PKT[i]);
    vB_NKT.push_back(vA_NKT[i]);
  }
}
vP_me.insert(vP_me.begin(),vB_PKT.begin(),vB_PKT.end());
vA.insert(vA.begin(),vB_AKT.begin(),vB_AKT.end());
vP_N.insert(vP_N.begin(),vB_N_PKT.begin(),vB_N_PKT.end());
vN.insert(vN.begin(),vB_NKT.begin(),vB_NKT.end());
return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ PT * _PDIGRAPH::FindPin(const NKT & key_n,const PKT & key_p)
// Find a pin, given the node and pin keys
{
                                       // Is the node there?
typename std::map<NKT,node>::iterator i = index_n.find(key_n);
if (i==index_n.end()) return (PT *)0;  // No, so there can't be a pin

                                       // Look in the fanin pin multimap
TPp_it pinIt = InPinKey2It(key_n,key_p);
                                       // Yup, it's there
if (pinIt!=index_n[key_n].fani.end()) return &((*pinIt)._2.data);
                                       // Nope - try the fanout
pinIt = OutPinKey2It(key_n,key_p);
                                       // Yup - it's there
if (pinIt!=index_n[key_n].fano.end()) return &((*pinIt)._2.data);

return (PT *)0;                        // Nope
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::FindPins(const AKT & a_k,PKT & p_fr_k,PKT & p_to_k)
// Given the arc key, overwrite references to the two pin keys
{
TPa_it a_it = ArcKey2It(a_k);          // Arc key -> iterator
if (a_it==index_a.end()) return false; // Arc not there

p_fr_k = PinKey((*a_it)._2.fr_p);      // (Inpins)
p_to_k = PinKey((*a_it)._2.to_p);      // (Outpins)
return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::FindPins(const NKT & n_k,
                                   vector<PKT> & vIP,vector<PKT> & vOP)
// Given a node key, overwrite reference vectors with pin keys
{
if (FindNode(n_k)==0) return false;    // Is it there?
vIP.clear();
vOP.clear();
WALKMULTIMAP(PKT,pin,index_n[n_k].fani,i) vIP.push_back(PinKey(i));
WALKMULTIMAP(PKT,pin,index_n[n_k].fano,i) vOP.push_back(PinKey(i));
return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::FlipArc(const AKT & key_a)
// Invert the connectivity of the arc 'key_a'. The pins flip WITH the arc.
{
TPa_it a = index_a.find(key_a);        // Find the arc
if (a==index_a.end()) return false;    // If not there...

NKT fr_k = (*(*a)._2.fr_n)._1;
NKT to_k = (*(*a)._2.to_n)._1;

MoveArcFrom(key_a,to_k);
MoveArcTo  (key_a,fr_k);

return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::InsertArc(const AKT & a_key,const NKT & fr,
                                    const NKT & to,const AT & a_data,
                                    const PKT & fr_p_key,const PT & fr_p_data,
                                    const PKT & to_p_key,const PT & to_p_data)
{
TPn_it ifr = index_n.find(fr);         // Find the end nodes; both there?
TPn_it ito = index_n.find(to);
if (ifr==index_n.end()) return false;
if (ito==index_n.end()) return false;

TPa_it ia = index_a.find(a_key);       // Find the arc; already there?
if (ia!=index_a.end()) return false;
                                       // Join them up :
                                       // Create the pins
TPp_it p_fr = (*ifr)._2.fano.insert(pair<PKT,PT>(fr_p_key,fr_p_data));
TPp_it p_to = (*ito)._2.fani.insert(pair<PKT,PT>(to_p_key,to_p_data));
                                       // Create the arc
pair<TPa_it,bool> ans1 = index_a.insert(pair<AKT,AT>(a_key,a_data));
                                       // Already exists?
if (ans1._2==false) return false;
                                       // Link the pins to the arc
(*p_fr)._2.iArc = ans1._1;
(*p_to)._2.iArc = ans1._1;
                                       // And the arc to the nodes
(*(ans1._1))._2.fr_n = ifr;
(*(ans1._1))._2.to_n = ito;
                                       // And the arc to the pins
(*(ans1._1))._2.fr_p = p_fr;
(*(ans1._1))._2.to_p = p_to;
return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::InsertNode(const NKT & key_n,const NT & data_n)
{
                                       // Is it already in the map?
if (index_n.find(key_n)!=index_n.end()) return false;
index_n[key_n] = node(data_n);         // Nope - shove it in
N_dirty = true;                        // Node map has changed
return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::MoveArcFrom(const AKT & key_a,const NKT & key_n)
// Move the 'from' end of arc 'key_a' from its current position and attach it
// to node 'key_n'. The pin stays with the arc.
{
TPa_it a = index_a.find(key_a);        // Find the arc
if (a==index_a.end()) return false;    // If not there...
TPn_it n = index_n.find(key_n);        // Find the target node
if (n==index_n.end()) return false;    // If not there...

                                       // Save the old pin <key,data>
TPp_it pI = (*a)._2.fr_p;
                                       // Add new pin to target node
TPp_it nP = (*n)._2.fano.insert(*pI);
                                       // Move arc 'from' pin
(*a)._2.fr_p = nP;
                                       // Trash the old pin
(*(*a)._2.fr_n)._2.fano.erase(pI);
                                       // Move arc 'from' node
(*a)._2.fr_n = n;
return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::MoveArcTo(const AKT & key_a,const NKT & key_n)
// Move the 'to' end of arc 'key_a' from its current position and attach it
// to node 'key_n'. The pin stays with the arc.
{
TPa_it a = index_a.find(key_a);        // Find the arc
if (a==index_a.end()) return false;    // If not there...
TPn_it n = index_n.find(key_n);        // Find the target node
if (n==index_n.end()) return false;    // If not there...

                                       // Save the old pin <key,data>
TPp_it pI = (*a)._2.to_p;
                                       // Add new pin to target node
TPp_it nP = (*n)._2.fani.insert(*pI);
                                       // Move arc 'from' pin
(*a)._2.to_p = nP;
                                       // Trash the old pin
(*(*a)._2.to_n)._2.fani.erase(pI);
                                       // Move arc 'from' node
(*a)._2.to_n = n;
return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ void _PDIGRAPH::RandomNode(NKT & rNK,NT & rND)
// Routine to deliver a random node
{
if (N_dirty) RConfig();                // If the node map has changed, rebuild
TPn_it n_it = RandN[RNG()];            // Hit the RNG
rNK = (*n_it)._1;                      // Unload the answers
rND = *(*n_it)._2;
}

//------------------------------------------------------------------------------

PDIGRAPH_ void _PDIGRAPH::RConfig()
// Routine to (re)build the random node access structure
{
if (!N_dirty) return;                  // If node map has changed, rebuild
RandN.clear();                         // Lose the past
                                       // Walk node map, and store the iterators
WALKMAP(NKT,node,index_n,i) RandN.push_back(i);
RNG.box(SizeNodes());                  // Ensure RNG distrib matches node list
N_dirty = false;                       // All up to date
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::RemoveArc(const AKT & key_a)
// Removing an arc requires searching arc link maps of the nodes to which it
// (the arc) is connecterd.
{
TPa_it a = index_a.find(key_a);        // Find it
if (a==index_a.end()) return false;    // If not there....
                                       // Modify the fanout of the 'from' node
(*(*a)._2.fr_n)._2.fano.erase((*a)._2.fr_p);
                                       // Modify the fanin of the 'to' node
(*(*a)._2.to_n)._2.fani.erase((*a)._2.to_p);
index_a.erase(a);                      // Remove from the arc index
return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::RemoveNode(const NKT & key_n)
// Routine to remove a node from the graph. We have to (in order)
// 1) Remove the connecting arcs from the graph, so the node is isolated
// 2) Remove the node from the graph.
// The pins will go automatically with the node.
// We can't just walk the fani/fano multimaps and kill stuff, because we'll
// be incrementing a map iterator for something we've just deleted, so we have
// to copy the iterators into a local vector and kill the whole lot in a 
// seperate pass
{
TPn_it n = index_n.find(key_n);        // Find it
                                       // Is it there?
if (n==index_n.end()) return false;    // Nope
                                       // (1a) Walk node fanin/out arc multimaps
vector<AKT> vak;
WALKMULTIMAP(PKT,pin,(*n)._2.fani,i) vak.push_back((*((*i)._2.iArc))._1);
WALKMULTIMAP(PKT,pin,(*n)._2.fano,i) vak.push_back((*((*i)._2.iArc))._1);
WALKVECTOR(AKT,vak,i) RemoveArc(*i);   // (1b) Kill them
                                       // Now the node is isolated
index_n.erase(n);                      // (2) Remove the node itself
N_dirty = true;                 // Node map has changed
return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ unsigned _PDIGRAPH::SizeInPins(const NKT & key_n)
{
if (index_n.find(key_n)==index_n.end()) return 0;
return index_n[key_n].fani.size();
}

//------------------------------------------------------------------------------

PDIGRAPH_ unsigned  _PDIGRAPH::SizeOutPins(const NKT & key_n)
{
if (index_n.find(key_n)==index_n.end()) return 0;
return index_n[key_n].fano.size();
}

//------------------------------------------------------------------------------

PDIGRAPH_ void _PDIGRAPH::WALKARCS(void * p,void (*f)(void *,const AKT &,AT &))
// The u$soft compiler is not capable of generating the temporaries automatically
{
WALKMAP(AKT,arc,index_a,i) if (f!=0) {
  AKT tmp_k =  (*i)._1;
  AT  tmp_d = *(*i)._2;
  f(p,tmp_k,tmp_d);
}
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::WALKINPINS(void * p,const NKT & iNode,
                                     void (*f)(void *,const PKT &,PT &))
{
if (index_n.find(iNode)==index_n.end()) return false;
WALKMULTIMAP(PKT,pin,index_n[iNode].fani,i) if (f!=0) {
  PKT tmp_k =  (*i)._1;
  PT  tmp_d = *(*i)._2;
  f(p,tmp_k,tmp_d);
}
return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ bool _PDIGRAPH::WALKOUTPINS(void * p,const NKT & iNode,
                                      void (*f)(void *,const PKT &,PT &))
{
if (index_n.find(iNode)==index_n.end()) return false;
WALKMULTIMAP(PKT,pin,index_n[iNode].fano,i) if (f!=0) {
  PKT tmp_k =  (*i)._1;
  PT  tmp_d = *(*i)._2;
  f(p,tmp_k,tmp_d);
}
return true;
}

//------------------------------------------------------------------------------

PDIGRAPH_ void _PDIGRAPH::WALKNODES(void * p,void (*f)(void *,const NKT &,NT &))
{
WALKMAP(NKT,node,index_n,i) if (f!=0) {
  NKT tmp_k =  (*i)._1;
  NT  tmp_d = *(*i)._2;
  f(p,tmp_k,tmp_d);
}
}

//------------------------------------------------------------------------------


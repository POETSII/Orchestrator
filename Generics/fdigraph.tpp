//==============================================================================

#include "macros.h"

//==============================================================================

FDIGRAPH_ void _FDIGRAPH::Dump()
{
printf("Ze fdigraph dumpings...\n");
printf("Node index:\n");
int c1 = 1;
int c2;
WALKMAP(NKT,node,index_n,i) {
  printf("%4d (",c1++);      NK_CB((*i).first);         printf(":");
                             ND_CB((*i).second.data);   printf(")\n");
  printf("  Fanin arcs\n");
  c2 = 1;
  WALKVECTOR(TPa_it,((*i).second).fani,j) {
   printf("  %4d ",c2++);    AK_CB((**j).first);        printf(":");
                             AD_CB((**j).second.data);  printf(" ->\n");
  }
  printf("  Fanout arcs\n");
  c2 = 1;
  WALKVECTOR(TPa_it,((*i).second).fano,j) {
   printf("  %4d -> ",c2++); AK_CB((**j).first);        printf(":");
                             AD_CB((**j).second.data);  printf("\n");
  }
  printf("===\n");
}
c1 = 1;
printf("Arc index:\n");
WALKMAP(AKT,arc,index_a,i) {
  printf("%4d (",c1++);     NK_CB((*(((*i).second).fr)).first);
  printf(":");              ND_CB((*(((*i).second).fr)).second.data);
  printf(") \t-> ^|");      AK_CB((*i).first);
  printf(":");              AD_CB(((*i).second).data);
  printf("|^ \t-> (");      NK_CB((*(((*i).second).to)).first);
  printf(":");              ND_CB((*(((*i).second).to)).second.data);
  printf(")\n");
}
printf("+------------------------+\n");
}

//------------------------------------------------------------------------------

FDIGRAPH_ AT * _FDIGRAPH::FindArc(AKT key_a)
{
map<AKT,arc>::iterator i = index_a.find(key_a);
return (i==index_a.end()) ? (AT *)0 : &((*i).second.data);
}

//------------------------------------------------------------------------------

FDIGRAPH_ NT * _FDIGRAPH::FindNode(NKT key_n)
{
map<NKT,node>::iterator i = index_n.find(key_n);
return (i==index_n.end()) ? (NT *)0 : &((*i).second.data);
}

//------------------------------------------------------------------------------

FDIGRAPH_ bool _FDIGRAPH::InsertArc(AKT key_a,NKT fr,NKT to,AT data_a)
{
                                       // Find the end nodes; both there?
TPn_it ifr = index_n.find(fr);
TPn_it ito = index_n.find(to);
if (ifr==index_n.end()) return false;
if (ito==index_n.end()) return false;
                                       // Join them up :
                                       // Link the nodes to the arcs
pair<TPa_it,bool> ans1 = index_a.insert(pair<AKT,AT>(key_a,data_a));
(*ifr).second.fano.push_back(ans1.first);
(*ito).second.fani.push_back(ans1.first);
                                       // And the arcs to the nodes
(*(ans1.first)).second.fr = ifr;
(*(ans1.first)).second.to = ito;
return true;
}

//------------------------------------------------------------------------------

FDIGRAPH_ bool _FDIGRAPH::InsertNode(NKT key_n,NT data_n)
{
                                       // Is it already in the map?
if (index_n.find(key_n)!=index_n.end()) return false;
index_n[key_n] = node(data_n);         // Nope - shove it in
return true;
}

//------------------------------------------------------------------------------

FDIGRAPH_ bool _FDIGRAPH::RemoveArc(AKT key_a)
// Removing an arc requires searching arc link vectors of the nodes to which it
// (the arc) is connecterd. These two searches are *not* fast. The vectors
// could/should be replaced with maps, so the delete time := O(log N)
{
TPa_it a = index_a.find(key_a);        // Find it
                                       // Is it there?
if (a==index_a.end()) return false;    // Nope
                                       // Detach pointers from node index ...
//     *a                              : pair in the index_a
//    (*a).second                      : arc structure
//    (*a).second.fr                   : iterator into the node index
//  *((*a).second.fr)                  : node pair
// (*((*a).second.fr)).second          : node structure
// (*((*a).second.fr)).second.fani     : vector
                                       // Modify the fanin of the 'from' node
vector<TPa_it> * pv = &((*((*a).second.fr)).second.fano);
vector<TPa_it>::iterator it = Seek(pv,a);
if (it!=(*pv).end())(*pv).erase(it);
                                       // Modify the fanout of the 'to' node
pv = &((*((*a).second.to)).second.fani);
it = Seek(pv,a);
if (it!=(*pv).end())(*pv).erase(it);

index_a.erase(a);                      // Remove from the arc index
return true;
}

//------------------------------------------------------------------------------

FDIGRAPH_ bool _FDIGRAPH::RemoveNode(NKT key_n)
// Routine to remove a node from the graph. We have to (in order)
// 1) Remove the connecting arcs from the graph, so the node is isolated
// 2) Remove the node from the graph
{
TPn_it n = index_n.find(key_n);        // Find it
                                       // Is it there?
if (n==index_n.end()) return false;    // Nope
//   n                                 : node iterator
//  *n                                 : pair in the index_n
// (*n).second                         : node structure
// (*n).second.fani                    : vector
                                       // (1) Walk the node fanin arc vector
WALKVECTOR(TPa_it,(*n).second.fani,i) {
//       *i                            : 'from' arc iterator
//      **i                            : pair in the index_a
//     (**i).second                    : 'from' arc structure
//     (**i).second.fr                 : iterator to 'from' node pair
//   *((**i).second.fr)                : 'from' node pair
//  (*((**i).second.fr)).second        : 'from' node structure
//  (*((**i).second.fr)).second.fano   : 'from' vector of arc iterators
// &(*((**i).second.fr)).second.fano   : address of ...
  vector<TPa_it> * pv = &(*((**i).second.fr)).second.fano;
  WALKVECTOR(TPa_it,(*pv),j) if ((*j==*i)&&(*j!=n)) {
    (*pv).erase(j);                    // Kill the 'other' link to the arc
    break;
  }
  index_a.erase(*i);                   // Kill the arc itself
}
                                       // (1) Walk the node fanout arc vector
WALKVECTOR(TPa_it,(*n).second.fano,i) {
  vector<TPa_it> * pv = &(*((**i).second.to)).second.fani;
  WALKVECTOR(TPa_it,(*pv),j) if ((*j==*i)&&(*j!=n)) {
    (*pv).erase(j);                    // Kill the 'other' link to the arc
    break;
  }
  index_a.erase(*i);                   // Kill the arc itself
}
                                       // (2) Remove the node itself
index_n.erase(n);
return true;
}

//------------------------------------------------------------------------------

FDIGRAPH_ void _FDIGRAPH::WALKARCS(void (*f)(AKT,AT))
{
WALKMAP(AKT,arc,index_a,i) f((*i).first,*(*i).second);
}

//------------------------------------------------------------------------------

FDIGRAPH_ void _FDIGRAPH::WALKNODES(void (*f)(NKT,NT))
{
WALKMAP(NKT,node,index_n,i) f((*i).first,*(*i).second);
}

//------------------------------------------------------------------------------

FDIGRAPH_ vector<fdigraph::TPa_it>::iterator
_FDIGRAPH::Seek(vector<fdigraph::TPa_it> * pv,TPa_it a)
{
WALKVECTOR(TPa_it,(*pv),i) if (*i==a) return i;
return (*pv).end();
}

//------------------------------------------------------------------------------


#ifndef __pdigraph__H
#define __pdigraph__H

#include "rand.h"
#include <map>
#include <vector>
#include <utility>
using namespace std;
#include "DumpChan.h"

//==============================================================================
          /*
class DumpChan
// DumpChan is a base class of the digraph, and intended to be the base of all
// the classes stored in it. This is so that the pretty-printer can find an
// output stream. G.DumpChan() sets the static channel value, and the static
// callbacks in the stored classes pick it up, and write to it when G.Dump()
// is called.
{
public:
              DumpChan(){}
virtual ~     DumpChan(){}
static FILE * dfp;                     // Dumpfile channel
};
            */
//FILE * DumpChan::dfp = stdout;

//==============================================================================

#define PDIGRAPH_ template<class NKT,class NT,class AKT,class AT,class PKT,class PT>
#define _PDIGRAPH pdigraph<NKT,NT,AKT,AT,PKT,PT>
#define _1 first
#define _2 second

PDIGRAPH_ class pdigraph : protected DumpChan
{
public:
             pdigraph(void);
             pdigraph(_PDIGRAPH &);
virtual ~    pdigraph(void){}

void         Clear();
bool         DrivenNodes(const NKT &,vector<NKT> &);
bool         DrivenNodes(const NKT &,vector<PKT> &,vector<AKT> &,
                         vector<PKT> &,vector<NKT> &);
bool         DrivingNodes(const NKT &,vector<NKT> &);
bool         DrivingNodes(const NKT &,vector<PKT> &,vector<AKT> &,
                          vector<PKT> &,vector<NKT> &);
void         Dump();
void         DumpChan(FILE * fp = stdout)  { dfp = fp;              }
AT *         FindArc(const AKT &);
bool         FindArcs(const NKT &,const PKT &,vector<AKT> &,vector<AKT> &);
bool         FindArcs(const NKT &,vector<AKT> &,vector<AKT> &);
NT *         FindNode(const NKT &);
bool         FindNodes(const AKT &,NKT &,NKT &);
bool         FindNodes(const NKT &,vector<NKT> &);
bool         FindNodes(const NKT &,vector<PKT> &,vector<AKT> &,
                       vector<PKT> &,vector<NKT> &);
PT *         FindPin(const NKT &,const PKT &);
bool         FindPins(const AKT &,PKT &,PKT &);
bool         FindPins(const NKT &,vector<PKT> &,vector<PKT> &);
bool         FlipArc(const AKT &);
bool         InsertArc(const AKT &,const NKT &,const NKT &,const AT & =AT(),
                       const PKT & =PKT(),const PT & =PT(),
                       const PKT & =PKT(),const PT & =PT());
bool         InsertNode(const NKT &,const NT & =NT());
bool         MoveArcFrom(const AKT &,const NKT &);
bool         MoveArcTo(const AKT &,const NKT &);
void         RandomNode(NKT &,NT &);
void         RandomSeed(unsigned s) { RNG.seed(s); }
bool         RemoveArc(const AKT &);
bool         RemoveNode(const NKT &);
unsigned     SizeArcs()                    { return index_a.size(); }
unsigned     SizeNodes()                   { return index_n.size(); }
unsigned     SizeInPins(const NKT &);
unsigned     SizeOutPins(const NKT &);

struct  arc;                           // Forward declare is public, so walking
struct  node;                          // macros below work
struct  pin;
typedef typename map<AKT,arc >::iterator TPa_it;
typedef typename map<NKT,node>::iterator TPn_it;
typedef typename multimap<PKT,pin >::iterator TPp_it;

// The guts of the thing. There appears to be no alternative to making the
// whole thing public or the walking macros won't work.

//FILE * dfp;                            // Dumpfile channel
struct node {
  node(){}
  node(NT d):data(d),tag(0){}          // Required for internal STL machinations
  NT data;                             // Node data
  multimap<PKT,pin> fani;              // Fanin pins
  multimap<PKT,pin> fano;              // Fanout pins
  void * tag;                          // For derived class use
  NT operator*(void) {return data;}    // Access the user data
};
struct arc {
  arc(AT d):data(d),tag(0){}           // Required for internal STL machinations
  AT data;                             // Arc data
  TPn_it fr_n;                         // Iterator to 'from' node
  TPp_it fr_p;                         // Iterator to 'from' pin
  TPn_it to_n;                         // Iterator to 'to' node
  TPp_it to_p;                         // Iterator to 'to' pin
  void * tag;                          // For the bits I forgot.....
  AT operator*(void) {return data;}    // Access the user data
};
struct pin {
  pin(PT d):data(d),tag(0){}           // Required for internal STL machinations
  PT data;                             // Pin data
  TPa_it iArc;                         // Iterator to connecting arc
  void * tag;
  PT operator*(void) {return data;}    // Access the user data
};

map<NKT,node> index_n;                 // This is the data, the whole data, and
map<AKT,arc > index_a;                 // nothing but the data

inline TPa_it ArcKey2It(AKT  key_a) { return index_a.find(key_a); }
inline TPn_it NodeKey2It(NKT  key_n){ return index_n.find(key_n); }
inline TPp_it InPinKey2It(NKT  key_n,PKT key_p)
                                     { return index_n[key_n].fani.find(key_p); }
inline TPp_it OutPinKey2It(NKT  key_n,PKT key_p)
                                     { return index_n[key_n].fano.find(key_p); }

public: // Routines to do with dumping, pretty printing and walking in general

#if (defined  _MSC_VER || defined __BORLANDC__ )
#define WALKPDIGRAPHNODES(NKT,NT,AKT,AT,PKT,PT,g,i) \
  for (map<NKT,pdigraph<NKT,NT,AKT,AT,PKT,PT>::node>::iterator \
       i = g.index_n.begin(); i!=g.index_n.end();i++)

#define WALKPDIGRAPHARCS(NKT,NT,AKT,AT,PKT,PT,g,i) \
  for (map<AKT,pdigraph<NKT,NT,AKT,AT,PKT,PT>::arc>::iterator \
       i = g.index_a.begin(); i!=g.index_a.end();i++)
#else
#define WALKPDIGRAPHNODES(NKT,NT,AKT,AT,PKT,PT,g,i) \
  for (typename map<NKT,pdigraph<NKT,NT,AKT,AT,PKT,PT>::node>::iterator \
       i = g.index_n.begin(); i!=g.index_n.end();i++)

#define WALKPDIGRAPHARCS(NKT,NT,AKT,AT,PKT,PT,g,i) \
  for (typename map<AKT,pdigraph<NKT,NT,AKT,AT,PKT,PT>::arc>::iterator \
       i = g.index_a.begin(); i!=g.index_a.end();i++)
#endif

#define WALKPDIGRAPHINPINS(NKT,NT,AKT,AT,PKT,PT,g,iNode,i) \
  for(multimap<NKT,pdigraph<NKT,NT,AKT,AT,PKT,PT>::pin>::iterator \
      i=g.index_n.find(iNode)->second.fani.begin();                  \
      i!=g.index_n.find(iNode)->second.fani.end();i++)

#define WALKPDIGRAPHOUTPINS(NKT,NT,AKT,AT,PKT,PT,g,iNode,i) \
  for(multimap<NKT,pdigraph<NKT,NT,AKT,AT,PKT,PT>::pin>::iterator \
      i=g.index_n.find(iNode)->second.fano.begin();                   \
      i!=g.index_n.find(iNode)->second.fano.end();i++)

#define PIN(i) (*i).second.data

TPa_it       ArcBegin()          { return index_a.begin(); }
AT           ArcData(TPa_it i)   { return *(*i)._2;        }
TPa_it       ArcEnd()            { return index_a.end();   }
AKT          ArcKey(TPa_it i)    { return (*i)._1;         }
TPn_it       NodeBegin()         { return index_n.begin(); }
NT           NodeData(TPn_it i)  { return *(*i)._2;        }
TPn_it       NodeEnd()           { return index_n.end();   }
NKT          NodeKey(TPn_it i)   { return (*i)._1;         }

//''''''''''''''''''''''''''''''''

//TPp_it       InPinBegin(NKT n)   { TPn_it iNode = index_n.find(n);
//                                   if (iNode==index_n.end()) return iNode;
//                                   return iNode.index_p.begin();           }
//PT           InPinData(TPp_it i) { return *(*i)._2;        }
//TPp_it       InPinEnd(NKT n)     { TPn_it iNode = index_n.find(n);
//                                   if (iNode==index_n.end()) return iNode;
//                                   return iNode.index_p.end();             }
//PKT          InPinKey(TPp_it i)  { return (*i)._1;         }
//TPp_it       OutPinBegin(NKT n)  { TPn_it iNode = index_n.find(n);
//                                   if (iNode==index_n.end()) return iNode;
//                                   return iNode.index_p.begin();           }
//PT           OutPinData(TPp_it i){ return *(*i)._2;        }
//TPp_it       OutPinEnd(NKT n)    { TPn_it iNode = index_n.find(n);
//                                   if (iNode==index_n.end()) return iNode;
//                                   return iNode.index_p.end();             }
//PKT          OutPinKey(TPp_it i) { return (*i)._1;         }
   //'''''''''''''''''''''''''''''''''
PT           PinData(TPp_it i)   { return *(*i)._2;        }
PKT          PinKey(TPp_it i)    { return (*i)._1;         }

void         WALKARCS   (void *,            void(*)(void *,const AKT &,AT &));
void         WALKNODES  (void *,            void(*)(void *,const NKT &,NT &));
bool         WALKINPINS (void *,const NKT &,void(*)(void *,const PKT &,PT &));
bool         WALKOUTPINS(void *,const NKT &,void(*)(void *,const PKT &,PT &));

// SETS the callback functions on the graph components

void SetAD_CB(void (* f)(const AT  &)=0){cb.AD_cb=f;}
void SetAK_CB(void (* f)(const AKT &)=0){cb.AK_cb=f;}
void SetND_CB(void (* f)(const NT  &)=0){cb.ND_cb=f;}
void SetNK_CB(void (* f)(const NKT &)=0){cb.NK_cb=f;}
void SetPD_CB(void (* f)(const PT  &)=0){cb.PD_cb=f;}
void SetPK_CB(void (* f)(const PKT &)=0){cb.PK_cb=f;}

private: // Dumping and pretty printing

// INVOKES the callback functions on the graph elements
// Another BORLAND compiler bug: originally I had Setxx_CB and Doxx_CB both
// called xx_CB; the signatures differed only in the argument list. BUT this
// caused the arguments to pick up the 'const' modifier from - well, I know not
// where. Changing the names made the modifier go away.

void DoAD_CB(      AT  & x) {
  if(cb.AD_cb!=0)cb.AD_cb(x);
  else fprintf(dfp,"No arc data c/b");
}
void DoAK_CB(const AKT & x) {
  if(cb.AK_cb!=0)cb.AK_cb(x);
  else fprintf(dfp,"No arc key c/b");
}
void DoND_CB(      NT  & x) {
  if(cb.ND_cb!=0)cb.ND_cb(x);
  else fprintf(dfp,"No node data c/b");
}
void DoNK_CB(const NKT & x) {
  if(cb.NK_cb!=0)cb.NK_cb(x);
  else fprintf(dfp,"No node key c/b");
}
void DoPD_CB(      PT  & x) {
  if(cb.PD_cb!=0)cb.PD_cb(x);
  else fprintf(dfp,"No pin data c/b");
}
void DoPK_CB(const PKT & x) {
  if(cb.PK_cb!=0)cb.PK_cb(x);
  else fprintf(dfp,"No pin key c/b");
}

struct cb_struct {                     // Callbacks
  void (* AD_cb)(const AT  &);         // Arc data
  void (* AK_cb)(const AKT &);         // Arc key
  void (* ND_cb)(const NT  &);         // Node data
  void (* NK_cb)(const NKT &);         // Node key
  void (* PD_cb)(const PT  &);         // Pin data
  void (* PK_cb)(const PKT &);         // Pin key
} cb;

// Random access to nodes. I initially tried to subclass this, but you'd think
// I'd have learned by now. Eventually I gave up.

vector<TPn_it>   RandN;                // Random access iterator vector
bool             N_dirty;              // Node map changed?
Urand            RNG;                  // Rectangular RNG
void             RConfig();            // Set up the random access structure
};

//==============================================================================



#include "pdigraph.tpp"
#undef PDIGRAPH_
#undef _PDIGRAPH
#undef _1
#undef _2
#endif

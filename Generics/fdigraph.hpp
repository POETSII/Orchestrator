#ifndef __fdigraph__H
#define __fdigraph__H

#include <map>
#include <vector>
using namespace std;

//==============================================================================

#define FDIGRAPH_ template<class NKT,class NT,class AKT,class AT>
#define _FDIGRAPH fdigraph<NKT,NT,AKT,AT>

FDIGRAPH_ class fdigraph
{
public:  // Creation and modification

             fdigraph(void){cb_struct x={0,0,0,0}; cb=x;}
virtual ~    fdigraph(void){}

AT *         FindArc(AKT);
NT *         FindNode(NKT);
bool         InsertArc(AKT,NKT,NKT,AT=AT());
bool         InsertNode(NKT,NT=NT());
bool         RemoveArc(AKT);
bool         RemoveNode(NKT);
unsigned int SizeArcs() {return index_a.size();}
unsigned int SizeNodes() {return index_n.size();}

struct arc;                            // Forward declare is public, so walking
struct node;                           // macros above work
typedef map<AKT,arc >::iterator TPa_it;
typedef map<NKT,node>::iterator TPn_it;

private:  // The guts of the thing

struct node {                          // Internal structure is private, so the
  node(){}                             // Bad Guys can't bugger it up
  node(NT d):data(d){}
  NT data;
  vector<TPa_it> fani;
  vector<TPa_it> fano;
  NT operator*(void) {return data;}
};
struct arc {
  arc(AT d):data(d){}
  AT data;
  TPn_it fr;
  TPn_it to;
  AT operator*(void) {return data;}
};
map<NKT,node> index_n;
map<AKT,arc > index_a;

public: // Routines to do with dumping, pretty printing and walking in general

#define WALKFDIGRAPHNODES(NKT,NT,AKT,AT,m,i) \
  for (map<NKT,fdigraph<NKT,NT,AKT,AT>::node>::iterator i = m.NodeBegin(); \
       i!=m.NodeEnd();i++)

#define WALKFDIGRAPHARCS(NKT,NT,AKT,AT,m,i) \
  for (map<AKT,fdigraph<NKT,NT,AKT,AT>::arc>::iterator i = m.ArcBegin(); \
       i!=m.ArcEnd();i++)

void         Dump();
void         AD_CB(void (* f)(AT )=0){cb.AD_cb=f;}
void         AK_CB(void (* f)(AKT)=0){cb.AK_cb=f;}
void         ND_CB(void (* f)(NT )=0){cb.ND_cb=f;}
void         NK_CB(void (* f)(NKT)=0){cb.NK_cb=f;}

TPa_it       ArcBegin()         { return index_a.begin(); }
AT           ArcData(TPa_it i)  { return  *(*i).second;   }
TPa_it       ArcEnd()           { return index_a.end();   }
AKT          ArcKey(TPa_it i)   { return (*i).first;      }
TPn_it       NodeBegin()        { return index_n.begin(); }
NT           NodeData(TPn_it i) { return *(*i).second;    }
TPn_it       NodeEnd()          { return index_n.end();   }
NKT          NodeKey(TPn_it i)  { return (*i).first;      }

void         WALKARCS(void (*)(AKT,AT));
void         WALKNODES(void (*)(NKT,NT));

private: // Dumping and pretty printing

void AD_CB(AT  x){if(cb.AD_cb!=0)cb.AD_cb(x);else printf("No arc data c/b");}
void AK_CB(AKT x){if(cb.AK_cb!=0)cb.AK_cb(x);else printf("No arc key c/b");}
void ND_CB(NT  x){if(cb.ND_cb!=0)cb.ND_cb(x);else printf("No node data c/b");}
void NK_CB(NKT x){if(cb.NK_cb!=0)cb.NK_cb(x);else printf("No node key c/b");}
vector<TPa_it>::iterator   Seek(vector<fdigraph::TPa_it> *,TPa_it);
struct cb_struct {                     // Callbacks
  void (* AD_cb)(AT);                  // Arc data
  void (* AK_cb)(AKT);                 // Arc key
  void (* ND_cb)(NT);                  // Node data
  void (* NK_cb)(NKT);                 // Node key
} cb;
};

//==============================================================================

#include "fdigraph.tpp"
#undef FDIGRAPH_
#undef _FDIGRAPH
#endif

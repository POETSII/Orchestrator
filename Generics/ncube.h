#ifndef __ncube__H
#define __ncube__H

#include "flat.h"
#include <string>
#include <vector>
using namespace std;

//==============================================================================

void PV(vector<int>);

//==============================================================================

class Ncube {
// This really should be a template, and it isn't. Get over it. Hang the data
// onto the payload fields and program down to the bare metal like a Real Man.

// And it should be a digraph, and it isn't either. And it seemed such a good
// idea to do it this way when I started out....

// Address : machine pointer to element
// Ordinal number : 0..whatever
// Coordinates : {left..right,left..right,left..right}

public:
struct _N;
struct _L;
                   Ncube(){}
                   Ncube(int,int[],int[]=0,bool[]=0);
virtual ~          Ncube(void);
void               doit(int,int[],int[],bool[]);
void               Dump(bool=true);

bool               chkvec(vector<int> &);
int                adr2ord(_N *,int &);
_N *               ord2adr(int,_N *&);
vector<int> &      ord2vec(int,vector<int> &);
int                vec2ord(vector<int>,int &);
vector<int> &      vecADDvec(vector<int>,vector<int>,vector<int> &);
vector<int> &      vecSUBvec(vector<int>,vector<int>,vector<int> &);
vector<int> &      vecADDscl(vector<int>,int,int,vector<int> &);
bool               vecADDscl2(vector<int>,int,int,vector<int> &);
vector<int> &      vecSUBscl(vector<int>,int,int,vector<int> &);

_N *               get0()               { return cell0;                   }
unsigned           getD()               { return (unsigned)extent.size(); }   // Return cube dimensions
void               getN(_N *,int,_N **,_N **); // Return two links
void               getL(_N *,int,_L **,_L **); // Return two links
string             name()               { return cname;                   }
void               name(string & rn)    { cname = rn;                     }
int                size()               { return ncells;                  }
void               xLink(int,_N *,_N *);

protected:
int                ncells;             // Total cell count
string             cname;              // Ummm...

struct _ext {                          // Extent layer
  _ext():st(1),lo(0),hi(0),wr(false),wi(0){}
  _ext(int _st,int _lo,int _hi,bool _wr,int _wi):st(_st),lo(_lo),hi(_hi),wr(_wr),wi(_wi){}
  int  lo;                             // Low index
  int  hi;                             // High index
  bool wr;                             // 'Identify edges' flag
  int  st;                             // Stride in linear memory
  int  wi;                             // Dimension width
};

vector<_ext>       extent;
_N *               cell0;
vector<_L *>       garbage;            // Link holder for destructor

public:
struct _N {                            // Node primitive
  _N():id(0),p(0),val(0){}
  int    id;                           // Ordinal number
  void * p;                            // Payload (i)
  template<class T> T * P()               { return static_cast<T *>(p);      }
//  template<class T> void setP (T * value) { p = static_cast<void *> (value); }
  void setP (void * value)                { p = static_cast<void *> (value); }
  int    val;                          // Payload (ii)
  vector<pair<_L *,_L *> > links;
  _N * L0(int i)   { return links[i].first ==0 ? 0 : links[i].first->_2;     }
  _N * L1(int i)   { return links[i].second==0 ? 0 : links[i].second->_1;    }
  _L * Lab0(int i) { return links[i].first;                                  }
  _L * Lab1(int i) { return links[i].second;                                 }
};

struct _L {                            // Link primitive
  _L(_N * c0,_N * c2):_1(c0),_2(c2),name(UniS(string("L"),3)){}
  _N * _1;
  _N * _2;
  string name;
};

};

//==============================================================================
   
#endif

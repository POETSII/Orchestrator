#ifndef __msg_p__H
#define __msg_p__H

#include "map2.h"
#include <typeinfo>
#include <map>
#include <vector>
#include <string>
using namespace std;

//==============================================================================
/* Copying these things seems to be a complete nightmare. Or, at least, I
haven't worked out how to do it yet. Copying the guts, as such, is fiddly but
tractable. What I can't do is make the copying mechanism importable into the
STL, which means, amongst other things, that putting them into vectors, maps and
so on is a non-starter. So if you want to do that sort of stuff, you have to
fall back on managing the heap yourself and shovelling pointers about. Hey-ho.
*/
//==============================================================================

class Msg_p {
public    : typedef unsigned char   byte; // Keep it simple
public    :                         Msg_p();
public    :                         Msg_p(byte *,int);
public    :                         Msg_p(Msg_p &);
public    : virtual ~               Msg_p(void);
private   : template <class Z> void AddToMap();
public    : void                    Clear();
public    : void                    Dump();
public    : template <class T> T *  Get(int,int &);
public    : void                    Get(int,string &);  // SAME ADR SPC AS CHAR
public    : template <class T> void Get(int,vector<T> &);
public    : void                    GetX(int,vector<string> &);
public    : int                     Id();
public    : void                    Id(int);
public    : int                     L(int);
public    : void                    L(int,int);
public    : int                     Length();
protected : void                    Load(byte *,int);
public    : int                     Mode();
public    : void                    Mode(int);
private   : void * operator         new(size_t, void * p) {return p;}
private   : template <class Z> Z    pullB(int &);
private   : template <class Z> void pushB(vector<byte> &,Z);
public    : template <class T> int  Put();
public    : template <class T> void Put(int,T *,int=1);
public    : void                    Put(int,string *);   // SAME ADR SPC AS CHAR
public    : template <class T> void Put(int,vector<T> *);
public    : void                    PutX(int,vector<string> *);
public    : int                     Src();
public    : void                    Src(int);
public    : byte *                  Stream();
public    : void                    Tag(int);
public    : int                     Tag();
public    : int                     Tgt();
public    : void                    Tgt(int);
public    : double                  Ztime(int);
public    : void                    Ztime(int,double);


protected :

struct SS {                            // One of these for each stored data item
  SS():p(0),c(0){}
  void Wipe(){ delete [] p; p=0; c=0; }
  byte * p;                            // Byte equivalent of typed data
  int    c;                            // Count of them
};

struct typemap {                       // One of these for each stored type
  typemap(byte t0):typelen(t0){}
  byte typelen;                        // Length of this type in bytes
  map <int,SS *> Dmap;                 // Map for this type of data item
};

map <int,typemap *> Tmap;              // Main sheetanchor map

vector<byte> vm;                       // Packed message

map2<const char *,int> t2imap;         // Type-to-integer reversible map
int typecount;

static int       counter;              // Id value generator
int              tag;                  // Message tag
static const int Z_FIELDS = 4;         // Number of identifier fields
int              level[Z_FIELDS];      // Identifier fields
static const int Z_TIMES = 4;          // Number of time fields
double           ztime[Z_TIMES];       // Holder for timestamp(s)
int              id;                   // Unique identifier
int              mode;                 // Doesn't go through the switcher
int              src;                  // Source ID
int              tgt;                  // Target ID

friend bool operator == (Msg_p &,Msg_p &);
friend bool operator != (Msg_p &,Msg_p &);
};

//==============================================================================

int Msg_p::counter = 0;

//==============================================================================

bool operator == (Msg_p &,Msg_p &);
bool operator != (Msg_p &,Msg_p &);

//==============================================================================

#include "msg_p.tpp"
#endif

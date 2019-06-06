#ifndef __Msg_p__HPP
#define __Msg_p__HPP

#include "map2.h"
#include <stdio.h>
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

// Strings are so useful they are explicitly supported, but they are immediately
// transformed into char arrays, so "string", "string &" and "char" all share
// the same user keyspace.

//------------------------------------------------------------------------------

typedef unsigned char   byte; // Keep it simple
static const unsigned BYTESPERWORD = 4;
static const unsigned BITSPERBYTE  = 8;

class Msg_p {
public:
                        Msg_p();
                        Msg_p(byte *,int);
                        Msg_p(byte *);
                        Msg_p(Msg_p &);
virtual ~               Msg_p(void);
void                    Clear();
void                    Dump(FILE * = stdout);
template <class T> void Dump(FILE * = stdout);
template <class T> T *  Get(int,int &);
void                    Get(int,string &);  // SAME ADR SPC AS CHAR
template <class T> void Get(int,vector<T> &);
void                    GetX(int,vector<string> &);    // SAME ADR SPC AS CHAR
int                     Id();
void                    Id(int);
unsigned                Key();
void                    Key(byte,byte=0x00,byte=0x00,byte=0x00);
static unsigned         KEY(byte=0x00,byte=0x00,byte=0x00,byte=0x00);
byte                    L(int);
void                    L(int,byte);
int                     Length();
int                     Mode();
void                    Mode(int);
template <class T> int  Put();
template <class T> void Put(int,T *,int=1);
void                    Put(int,string *);   // SAME ADR SPC AS CHAR
template <class T> void Put(int,vector<T> *);
void                    PutX(int,vector<string> *); // SAME ADR SPC AS CHAR
static unsigned         Sizeof(byte *);
void                    SubKey(byte * sk);
//void                    Send(int);
int                     Src();
void                    Src(int);
byte *                  Stream();
void                    Tag(int);
int                     Tag();
int                     Tgt();
void                    Tgt(int);
string                  Zname(unsigned);
void                    Zname(unsigned,string);
double                  Ztime(int);
void                    Ztime(int,double);

static const unsigned   Z_FIELDS = 4;  // Number of identifier fields
static const unsigned   Z_TIMES  = 4;  // Number of time fields
static const unsigned   Z_NAMES  = 4;  // Number of name strings

private   :
template <class Z> void AddToMap();
//void *         operator new(size_t, void * p) {return p;}
template <class Z> Z    pullB(int &);
template <class Z> void pushB(vector<byte> &,Z);
string                  pullS(int &);
void                    pushS(vector<byte> &,string);

protected :
void                    Load(byte *,int);

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

static int            counter;         // Id value generator
int                   tag;             // Message tag
byte                  subkey[Z_FIELDS];// Identifier fields
double                ztime[Z_TIMES];  // Holder for timestamp(s)
string                names[Z_NAMES];  // Holder for names
int                   id;              // Unique identifier
int                   mode;            // Doesn't go through the switcher
int                   src;             // Source ID
int                   tgt;             // Target ID

friend bool operator == (Msg_p &,Msg_p &);
friend bool operator != (Msg_p &,Msg_p &);

};

//==============================================================================

bool operator == (Msg_p &,Msg_p &);
bool operator != (Msg_p &,Msg_p &);

//==============================================================================

#include "Msg_p.tpp"
#endif

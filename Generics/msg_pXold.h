#ifndef __msg_p__H
#define __msg_p__H

#include <typeinfo>
#include <map>
#include <vector>
#include <string>
using namespace std;

//==============================================================================

class Msg_p {
public : typedef unsigned char   byte; // Keep it simple
                                       // Empty constructor
public :                         Msg_p();
                                       // From received message
public :                         Msg_p(byte *,int);
public : virtual ~               Msg_p(void);
private: template <class T> void AddToMap();
private: template <class T> void Destroy();
public : void                    Dump();
private: template <class T> void Dump();
public : template <class T> T *  Get(int,int &);
public : int                     Length();
private: template <class Z> Z    pull(int=1);
private: template <class Z> void push(vector<byte> &,Z);
public : template <class T> void Put(int,T *,int=1);
public : byte *                  Stream();
public : void                    Tag(int);
public : int                     Tag();
private: template <class Z> Z    pullB();
private: template <class Z> Z *  copyB(int);
private: template <class Z> void pushB(vector<byte> &,Z);

private:

template <class T> struct SS {         // One of these for each stored data item
  SS():p(0),c(0),a(0){}
  Wipe(){ delete [] p; p=0; delete [] a; a=0;}
  T *    p;                            // Copy of typed data
  int    c;                            // Count of them
  int    bc;                           // Count of bytes
  byte * a;                            // Bit copy of data (i.e. p) as bytes
};

struct typemap {                       // One of these for each stored type
  typemap(byte t0):typelen(t0){}
//  typemap(byte t0,int t1):typelen(t0),typeint(t1){}
  byte typelen;                        // Length of this type in bytes
//  int  typeint;                        // Each type has an integer ID
  map <int,void *> Dmap;
};

//map <const char *,typemap *> Tmap;     // Main sheetanchor map
map <int, typemap *> Tmap;

vector<byte> vm;                       // Packed output message

int typecount;
map <const char *,int> s2iMap;         // Typename -> integer ID
map <int,const char *> i2sMap;         // Integer ID -> typename

int tag;                               // MPI message tag
const static int EOM = -1;             // End of message marker

friend bool operator == (Msg_p &,Msg_p &);
friend bool operator != (Msg_p &,Msg_p &);
};

//==============================================================================

bool operator == (Msg_p &,Msg_p &);
bool operator != (Msg_p &,Msg_p &);

//==============================================================================

#include "msg_p.tpp"
#endif

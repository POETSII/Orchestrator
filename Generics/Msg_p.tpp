#ifndef __msg_p__TPP
#define __msg_p__TPP

#include "macros.h"
#include <stdio.h>
//==============================================================================
//
// Note this file contains the template function method body definitions, so it
// has to be #included into the calling source, NOT compiled separately.
//
//==============================================================================

template <class Z> void Msg_p::AddToMap()
{
//printf("Adding %s<->%d to the typemap\n",typeid(Z).name(),typecount);
t2imap.Add(typeid(Z).name(),typecount++);
}

//------------------------------------------------------------------------------

template <class T> void Msg_p::Dump(FILE * fp)
// Type-specific dump.
{
fprintf(fp,"---------------------------------------------\n");
fprintf(fp,"Type specific dumping Msg_p\n\n");
int itype;                                    // Type key
                                              // String is special case
if (typeid(T)==typeid(string)) {
  fprintf(fp,"Type \"string\" is mapped to \"char\"\n");
  itype = t2imap[typeid(char).name()];
}
else itype = t2imap[typeid(T).name()];
fprintf(fp,"Type ||%s|| has type key %d\n",typeid(T).name(),itype);
typemap * tm = Tmap[itype];                   // Get the corresponding typemap
fprintf(fp,"Typemap: Type length (in bytes) %d\n",tm->typelen);
fprintf(fp,"    User  Byte  \n");
fprintf(fp,"    key   count Data\n");
WALKMAP(int,SS *,tm->Dmap,i) {
  fprintf(fp,"%5d %5d     ",(*i).first,(*i).second->c);
  for(int j=0;j<(*i).second->c;j++) fprintf(fp,"%2x ",(*i).second->p[j]);
  fprintf(fp,"\n                ");
  for(int j=0;j<(*i).second->c;j++) fprintf(fp,"%1c  ",(*i).second->p[j]);
  fprintf(fp,"\n");
//  T value = T((*i).second->p);
//  cout << value;
}
fprintf(fp,"---------------------------------------------\n");
fflush(fp);
}

//------------------------------------------------------------------------------

template <class T> T * Msg_p::Get(int k,int & cnt)
// Can return 0, so you need to test it before you dereference it
{
cnt = 0;
int keyT = t2imap[typeid(T).name()];   // Data type key
                                       // Legitimate response - unknown type
if (Tmap.find(keyT)==Tmap.end()) return (T *)0;
typemap * pTM = Tmap[keyT];
if (pTM==0) return (T *)0;             // Bad response - unknown type
                                       // Legitimate response - unknown key
if (pTM->Dmap.find(k)==pTM->Dmap.end()) return (T *)0;

SS * pSS = pTM->Dmap[k];               // Data stepping stone
cnt = pSS->c / pTM->typelen;           // Number of things
return new(pSS->p) T;                  // Reinterpret ->p as type T
}

//------------------------------------------------------------------------------

template <class T> void Msg_p::Get(int k,vector<T> & vT)
{
int len;
T * p = Get<T>(k,len);
vT.clear();
for (int i=0;i<len;i++) vT.push_back(p[i]);
}

//------------------------------------------------------------------------------

template <class Z> Z Msg_p::pullB(int & nfc)
{
Z x = * new(&vm[nfc]) Z;
nfc += sizeof(Z);
return x;
}

//------------------------------------------------------------------------------

template <class Z> void Msg_p::pushB(vector<byte> & v,Z x)
// Routine to break up the variable x and shove it into the byte vector v
{
const int s = sizeof(Z);               // Size of x in bytes
byte buff[s];                          // Can do this 'cos s is compile-time
new(buff) Z(x);                        // Write x as a byte stream
for(int i=0;i<s;i++) v.push_back(buff[i]);  // Save it
}

//------------------------------------------------------------------------------

template <class T> int Msg_p::Put()
// Insert into datastructure the knowledge of a type T, even though we don't
// want to put one in yet. This is so that you can shove a type into an unpacked
// message and get the fields out right.
{
// Get the type key
int keyT = t2imap[typeid(T).name()];
// If the type has not been seen before, create the t2i map entry
if (keyT==0) keyT = t2imap.Add(typeid(T).name(),typecount++);
// And create the typemap entry
if (Tmap.find(keyT) == Tmap.end()) Tmap[keyT] = new typemap(sizeof(T));
return keyT;
}

//------------------------------------------------------------------------------

template <class T> void Msg_p::Put(int k,T * data,int cnt)
// Insert into datastructure a bunch of things, type T, user key k, cnt of them.
{
// Legitimate, if somewhat strange, exit
if (cnt==0) return;
// Data structure is dirty.
vm.clear();
// Insert the details of the type T, if necessary
int keyT = Put<T>();
// Either way, the typemap for T now exists; get the address
typemap * pTM = Tmap[keyT];
// If the data key has not been seen before, create one AND the stepping stone
if (pTM->Dmap.find(k)==pTM->Dmap.end()) pTM->Dmap[k] = new SS();
// If it has, hose the old data BUT NOT the stepping stone
else pTM->Dmap[k]->Wipe();
// Cannot make this definition before, 'cos of side-effect of []
SS * pSS = pTM->Dmap[k];
// Size of byte buffer
int siz = sizeof(T);
pSS->c = siz * cnt;
// Now COPY the input data; first create the storage, then copy
pSS->p = new byte[pSS->c];
byte * base = new(&data[0]) byte;
for(int i=0;i<pSS->c;i++) pSS->p[i] = base[i];
}

//------------------------------------------------------------------------------

template <class T> void Msg_p::Put(int k,vector<T> * data)
// Special case for vectors 'cos they're so useful... you could do this for
// sets, queues and lists quite easily; templates of n-tuples might require a
// bit more effort?
{
Put<T>(k,&((*data)[0]),data->size());
}

//------------------------------------------------------------------------------

#endif


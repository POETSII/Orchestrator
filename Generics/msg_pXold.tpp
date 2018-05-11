//==============================================================================
//
// Note this class *contains* template function members, so it has to be linked
// like it was a template class, i.e. the source - this file - needs to be
// #included into the calling source, NOT compiled separately.
// Note it *will* compile separately successfully, because it contains non-
// templated code, it's just that the templates won't compile ('cos they've
// not - yet - been specialised) and so the linker won't see the specialised
// object code. 'Cos it's not there.

#include "msg_p.h"
#include "macros.h"
#include <stdio.h>

// There exists a fundamental problem in trying to write a generic routine that
// accesses the entire datastructure (specifically Dump() and the destructor).
// Upfront, we don't know what types are in the message, and - even though we
// can store this information at runtime when we create the object in some form
// or another, to be useful, it needs to be available at *compile-time* so that
// Dump() et al can be specialised accordingly.
// The only real way round this is to trawl through every possible builtin type
// the hard way.
// Which is why you have to keep an eye open on the list below.......

#define THE_LIST(X)                     \
                    X<int>();           \
                    X<int *>();         \
                    X<float>();         \
                    X<double>();        \
                    X<char>();          \
                    X<long>();          \
                    X<long double>();   \
                    X<unsigned char>(); \
                    X<string>();        \
                    X<vector<int> >();

// Note that, in a given memory space, you can quite happily Get() and Put()
// arbitrarily complicated objects, because there are no nasty tricks played
// when getting stuff in and out of a local object - it's all good, high-level
// C++. Well, almost. Even the stunt with the static_cast<> is completely safe.
// The difficulty comes when you send stuff over the MPI link, 'cos a lot of
// the STL objects, for example, exist on stack and heap both, and there's no
// clean way of localising, say, a string, onto a contiguous set of bytes,
// *especially* if you don't know the internal layout. Even if you do, putting
// it back together on the other side would be a nightmare, 'cos all the heap
// pointers would be screwed, to say the very least. Thus you'd have to write
// explicit streaming methods for each class you transported. OK, so that's
// what virtual methods are for, but, hey, life's (currently) too short.
// The take-home message, then, is stick to built-in primitive types, and
// arrays thereof, and it'll all be tickety-spong.

//==============================================================================

Msg_p::Msg_p()
// Constructor from empty
{
typecount = 1;
tag = 0;
THE_LIST(AddToMap)
Dump();
}

//------------------------------------------------------------------------------

Msg_p::Msg_p(byte * tm,int len)
// Constructor from byte stream
{
typecount = 1;
tag = 0;
THE_LIST(AddToMap)
                                       // Copy the data somewhere safe
for(int i=0;i<len;i++) vm.push_back(tm[i]);

tag = pullB<int>();    // Unload the tag
int msize = pullB<int>();  // Message size in bytes
int elmapT = pullB<int>();  // Elements in Tmap
for (int i = 0;i<elmapT;i++) {
  int keyT = pullB<int>();                 // Key for this type (not seen before)
  byte typelen = pullB<byte>(); // Length of this type in bytes
  Tmap[keyT] = new typemap(typelen);
  int elmapD = pullB<int>();  // Elements in Dmap
  for (int j = 0;j<elmapD;j++) {
    int keyD = pullB<int>();   // Key for this data item
    int itcnt = pullB<int>();  // Data item count

    void * px =  Tmap[keyT]->Dmap[keyD];

    px = new SS<byte>;




    static_cast<SS<byte> *>(Tmap[keyT]->Dmap[keyD])->p = new SS<byte>[itcnt];
    Tmap[keyT]->Dmap[keyD]->c = itcnt;
    byte * data = copyB<byte>(typelen*itcnt);
    for(int k=0;k<typelen*itcnt;k++) Tmap[keyT]->Dmap[keyD]->p[k]=data[k++];
  }
}

//------------------------------------------------------------------------------

template <class Z> Z Msg_p::pullB()
{

Z x = * new(tm[nfc]) Z();
nfc += sizeof(Z);
return x;



//const int s = sizeof(Z);
//byte buff[s];
///*Z * tmp = */ new(buff) Z(x);

}

//------------------------------------------------------------------------------

template <class Z> Z * Msg::copyB(int cnt)
{

Z * p = new(tm[nfc]) Z();
nfc += cnt + sizeof(Z);
return p;
}

/*

tag = pull<int>();   // Unload tag
int size = pull<int>();       // Unload total message size
int msize;
for(;;) {
  if ((msize=pull<int>())==EOM) return;// Unload next map size
  int typeID = pull<int>();
  int itemcount = pull<int>();
  int typesize = pull<byte>();
  for(int i=0;i<itemcount;i++) {
    int key = pull<int>();
    int cnt = pull<int>();
    for(int j=0;j<cnt;j++) {
      int data = pull<byte>(typesize);
    }
  }
}
  */
}

//------------------------------------------------------------------------------

Msg_p::~Msg_p()
{
printf("~Msg_p\n");
THE_LIST(Destroy)
}

//------------------------------------------------------------------------------




template <class T> void Msg_p::AddToMap()
{
const char * name = typeid(T).name();
s2iMap[name] = typecount;
i2sMap[typecount++] = name;
}

//------------------------------------------------------------------------------

template <class T> void Msg_p::Destroy()
{
printf("Destroying Msg_p<%s>\n",typeid(T).name());
typemap * pTM = Tmap[typeid(T).name()];
if (pTM==0) printf("... no entries\n");
else WALKMAP(int,void *,pTM->Dmap,i) {
  static_cast<SS<T>*>((*i).second)->Wipe();
  delete (*i).second;
}
delete pTM;
}

//------------------------------------------------------------------------------

void Msg_p::Dump()
{
printf("---------------------------------------------\n");
printf("Dumping Msg_p\n\n");
WALKMAP(const char *,int,s2iMap,i)
  printf("s2iMap[%s] = %d\n",(*i).first,(*i).second);
WALKMAP(int,const char *,i2sMap,i)
  printf("i2sMap[%d] = %s\n",(*i).first,(*i).second);
THE_LIST(Dump);
printf("---------------------------------------------\n");
}

//------------------------------------------------------------------------------

template <class T> void Msg_p::Dump()
{
const char * name = typeid(T).name();
printf("Dumping Msg_p<%s>\n",name);
if (Tmap.find(name)==Tmap.end()) printf("... no entries\n");
else {
  typemap * pTM = Tmap[name];
  printf("typelen = %d\n",pTM->typelen);
  printf("typeint = %d\n",pTM->typeint);
  WALKMAP(int,void *,pTM->Dmap,i) {
    SS<T> * pSS = static_cast<SS<T>*>((*i).second);
    printf("key = %5d, &data = %0x x %d\n",(*i).first,pSS->p,pSS->c);
  }
}
printf("\n");
}

//------------------------------------------------------------------------------

template <class T> T * Msg_p::Get(int k,int & cnt)
{
cnt = 0;
const char * name = typeid(T).name();
                                       // Legitimate response - unknown type
if (Tmap.find(name)==Tmap.end()) return 0;
typemap * pTM = Tmap[name];
if (pTM==0) return 0;                  // Bad response - unknown type
                                       // Legitimate response - unknown key
if (pTM->Dmap.find(k)==pTM->Dmap.end()) return 0;
SS<T> * pSS = static_cast<SS<T> *>(pTM->Dmap[k]);
cnt = pSS->c;
return pSS->p;
}

//------------------------------------------------------------------------------

int Msg_p::Length()
{
return 0;
}

//------------------------------------------------------------------------------

template <class Z> Z Msg_p::pull(int len)
{

return (Z)0;

}

//------------------------------------------------------------------------------


template <class Z> void Msg_p::push(vector<byte> & v,Z x)
// Routine to break up the variable x and shove it into the byte vector v
{
const int s = sizeof(Z);
byte buff[s];
Z * tmp = new(buff) Z(x);
for(int i=0;i<s;i++) v.push_back(buff[i]);
}

//------------------------------------------------------------------------------

template <class T> void Msg_p::Put(int k,T * data,int cnt)
{
const char * name = typeid(T).name();
// If the type has not been seen before, create an entry in the main map
if (Tmap.find(name) == Tmap.end())
  Tmap[name] = new typemap(sizeof(T),s2iMap[name]);
// Either way, the typemap for T now exists; get the address
typemap * pTM = Tmap[name];
// If the data key has not been seen before, create one AND the stepping stone
if (pTM->Dmap.find(k)==pTM->Dmap.end())
  static_cast<SS<T> *>(pTM->Dmap[k]) = new SS<T>();
// If it has, hose the old data BUT NOT the stepping stone
else static_cast<SS<T> *>(pTM->Dmap[k])->Wipe();
// Cannot make this definition before, 'cos of side-effect of []
SS<T> * pSS = static_cast<SS<T> *>(pTM->Dmap[k]);
pSS->c = cnt;
// We need to store this, 'cos when we stream/unstream we don't have T easily
pSS->bc = cnt * sizeof(T);
// Legitimate, if somewhat strange, exit
if (cnt==0) return;
// Now COPY the input data, preserving the type
pSS->p = new T[cnt];
//for(int i=0;i<cnt;i++) pSS->p[i] = data[i];
// Copy it again into the streamable data buffer fragment
//int inspect = cnt*sizeof(T);
pSS->a = new byte[cnt*sizeof(T)];

//char * buff = new(pSS->a) char[cnt];
//buff[0] = '#';
//printf("%c\n",*buff);

T * buff = new(pSS->a) T[cnt];
for(int i=0;i<cnt;i++) buff[i] = pSS->p[i] =  data[i];

// Now pSS->a contains the byte stream equivalent
}

//------------------------------------------------------------------------------

Msg_p::byte * Msg_p::Stream()
// Routine to take all the byte vector fragments in the data structure and
// bolt them togther to form a single byte stream that can be piped through MPI
// We can do this without specialising the SS template because we never
// reference the p field - the only one that cares about the specialisation.
{

pushB<int>(vm,tag); // Message tag
pushB<int>(vm,0); // Placeholder for total message size
pushB<int>(vm,Tmap.size());  // Number of elements in the Pmap
WALKMAP(int,typemap *,Tmap,i) {  // Walk the Tmap
  typemap * pTm = (*i).second;
  pushB<int>(vm,(*i).first);  // Type key
  pushB<byte>(vm,pTm->typelen);   // Type length
  pushB<int>(vm,pTm->size()); // Size of Dmap
  WALKMAP(int,void *,pTm->Dmap,j) {
    ss<void> * pSS = static_cast<SS<void>*>((*j).second);
    pushB<int>(vm,(*j).first);    // Item key
    pushB<int>(vm,pSS->c);
    pushB<byte>(vm,pSS->p,(pSS->c)*(pTm->typelen));
  }
}
    /*
push<int>(vm,tag);                     // Message tag
push<int>(vm,0);                       // Placeholder for message size

WALKMAP(const char *,typemap *,Tmap,i) {
  typemap * pTM = (*i).second;
  push<int>(vm,pTM->typeint);
  push<int>(vm,pTM->Dmap.size());
  vm.push_back(pTM->typelen);
  WALKMAP(int,void *,pTM->Dmap,j) {
    SS<void> * pSS = static_cast<SS<void>*>((*j).second);
    for (int k=0;k<pSS->c;k++) printf("%d %x\n",k,pSS->a[k]);
    push<int>(vm,(*j).first);
    push<int>(vm,pSS->bc);
    for (int k=0;k<pSS->c;k++) vm.push_back(pSS->a[k]);
  }
}
*/
WALKVECTOR(byte,vm,i) printf("%x ",*i);
return &vm[0];
}

//------------------------------------------------------------------------------

void Msg_p::Tag(int t)
{
tag = t;
}

//------------------------------------------------------------------------------

int Msg_p::Tag()
{
return tag;
}
//------------------------------------------------------------------------------

template <class Z> void Msg_p::pushB(vector<byte> & v,Z x)
// Routine to break up the variable x and shove it into the byte vector v
{
const int s = sizeof(Z);            // Size of x in bytes
byte buff[s];                       // Can do this 'cos s is compile-time
/*Z * tmp = */new(buff) Z(x);      // Write x as a byte stream
for(int i=0;i<s;i++) v.push_back(buff[i]);  // Save it
}

//------------------------------------------------------------------------------

template <class Z> void Msg_p::push(vector<byte> & v,Z * px,int cnt)
// Routine to take the variable list pointed to by px (there are cnt of them)
// and shove them into the byte vector v
{
const int s = sizeof(Z);
byte buff[s];
/*Z * tmp = */ new(buff) Z(x);
for(int i=0;i<cnt;i++) {
  new(buff) Z(*px++);
  for(int j=0;j<s;j++)v.push_back(buff[j]);
}
}

//------------------------------------------------------------------------------


//==============================================================================

bool operator == (Msg_p & a,Msg_p & b)
{
if (a.vm.size()!=b.vm.size()) return false;
for (unsigned int i=0;i<a.vm.size();i++) if (a.vm[i]!=b.vm[i]) return false;
return true;
}

//------------------------------------------------------------------------------

bool operator != (Msg_p & a,Msg_p & b)
{
return !(a==b);
}

//------------------------------------------------------------------------------


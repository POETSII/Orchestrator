
#include "Msg_p.hpp"
#include "macros.h"
#include <stdio.h>

//==============================================================================
//
// Note this class *contains* template function members, even though it's not a
// template class itself. Back in the days of sanity, this wasn't a problem,
// but now (C++11) u$oft gets all picky, so I've had to split the method
// definitions into two files: the *.tpp, which is #included from the *.hpp file
// like a template class, and a *.cpp, which contains a #include *.hpp and is
// compiled as a separate translation unit. The whole lot can then be linked
// and there are no duplicate non-template method body definitions.
//
// There exists a fundamental problem in trying to write a generic routine that
// accesses the entire datastructure (specifically Dump() and the destructor).
// Upfront, we don't know what types are in the message, and - even though we
// can store this information at runtime when we create the object in some form
// or another, to be useful, it needs to be available at *compile-time* so that
// Dump() et al can be specialised accordingly.
// The only real way round this is to trawl through every possible builtin type
// the hard way.
// Which is why you have to keep an eye open on the list below.......

#define THE_LIST(X)                            \
                    X<bool>();                 \
                    X<bool *>();               \
                    X<char>();                 \
                    X<char *>();               \
                    X<double>();               \
                    X<double *>();             \
                    X<float>();                \
                    X<float *>();              \
                    X<int>();                  \
                    X<int *>();                \
                    X<long>();                 \
                    X<long *>();               \
                    X<long double>();          \
                    X<long double *>();        \
                    X<short int>();            \
                    X<short int *>();          \
                    X<unsigned char>();        \
                    X<unsigned char *>();      \
                    X<unsigned int>();         \
                    X<unsigned int *>();       \
                    X<unsigned long>();        \
                    X<unsigned long *>();      \
                    X<unsigned short int>();   \
                    X<unsigned short int *>(); \
                    X<void *>();

// Note that, in a given memory space, you can quite happily Get() and Put()
// arbitrarily complicated objects, because there are no nasty tricks played
// when getting stuff in and out of a local object - it's all good, high-level
// C++.
// The difficulty comes when you send stuff over the MPI link, 'cos a lot of
// the STL objects, for example, exist on stack and heap both, and there's no
// clean way of localising, say, a map, onto a contiguous set of bytes,
// *especially* if you don't know the internal layout. Even if you do, putting
// it back together on the other side would be a nightmare, 'cos all the heap
// pointers would be screwed, to say the very least. Thus you'd have to write
// explicit streaming methods for each class you transported. OK, so that's
// what virtual methods are for, but, hey, life's (currently) too short.
// I *have* written special purpose interfaces for vectors, strings, and vectors
// of strings. The vector code can easily be extended to cope with sets, queues
// and lists. Anything else will be a bit fiddly, but if the need arises....
// The special name for the string vector Get/Put is, I think, made necessary
// by a BORLAND BUG - the compiler should be able to disambiguate the overloads,
// but it can't.
// The saving (Stream()) and restoring (Msg_p(byte *,int)) code knows nothing of
// types - everything is handled and stored as fragments of a byte stream. The
// internal datastructure holds everything as pure bytes. When you explicitly
// load/unload stuff (Get(...) and Put(...)), the type information is overlaid
// onto the byte images by some creative use of overloaded new().
//
// It also works fine for arbitrarily complicated old-style structures that
// don't have methods - effectively, once you have sizeof(), you have it all,
// so you can carry on. I think this might be a feature - everything that isn't
// known about up front gets type key 0.....

//==============================================================================

int Msg_p::counter = 0;

//==============================================================================

Msg_p::Msg_p()
// Constructor from empty
{
tag  = 0;                              // No tag
src  = -1;                             // UNDEF in the derived class, but the
tgt  = -1;                             // base is pure....
mode = 0;                              // Mode == 0 is the default normal
id   = counter++;                       // Process-local unique id
for(unsigned i=0;i<Z_TIMES;i++)ztime[i]=0.0;// No times - yet...
for(unsigned i=0;i<Z_FIELDS;i++)subkey[i]=0x00;// No labels - yet...
typecount = int() + 1;                 // Set up the initial type table
THE_LIST(AddToMap)
}

//------------------------------------------------------------------------------

Msg_p::Msg_p(byte * tm,int len)
// Constructor from byte stream
{
typecount = int() + 1;
THE_LIST(AddToMap)
Load(tm,len);
id   = counter++;                       // Process-local unique id
}

//------------------------------------------------------------------------------

Msg_p::Msg_p(byte * tm)
// Better constructor from byte stream, which doesn't need the explicit length
// because the length is now (2018) encoded as the first four bytes of the byte
// stream
{
typecount = int() + 1;
THE_LIST(AddToMap)
unsigned len = * new(&tm[0]) unsigned;
Load(tm,len);
id   = counter++;                       // Process-local unique id
}

//------------------------------------------------------------------------------

Msg_p::Msg_p(Msg_p & m)
// Copy constructor. By far the easiest way to do this is simply stream the
// source and unstream to the target....
// This *does not* copy over any additions to the type table
{
typecount = int() + 1;
THE_LIST(AddToMap)
Load(m.Stream(),m.Length());
id   = counter++;                       // Process-local unique id
}

//------------------------------------------------------------------------------

Msg_p::~Msg_p()
{
Clear();                               // Hose the dynamic bits
}

//------------------------------------------------------------------------------

void Msg_p::Clear()
{
WALKMAP(int,typemap *,Tmap,i) {        // Kill the datastructure
  WALKMAP(int,SS *,(*i).second->Dmap,j) {
    delete [] (*j).second->p;
    delete (*j).second;
  }
  delete (*i).second;
}
Tmap.clear();
t2imap.clear();                        // Just need to remove any user-defined
typecount = int() + 1;                 // types from the typemap, but it's
THE_LIST(AddToMap)                     // easier to hose it and re-initialise
}

//------------------------------------------------------------------------------

void Msg_p::Dump(FILE * fp)
// You can't dump the data in a pretty-print, because the object has no internal
// knowledge of the data types within it. You can say "Dump all the integers",
// but not "Dump all the stuff", because the nature of "stuff" is not stored
// (or storable) in the object. You have to use Dump<T>() below.
{
fprintf(fp,"Msg_p+++++++++++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"Raw dumping Msg_p\n\n");
fprintf(fp,"Source : %4d, Target    : %4d\n",src,tgt);
fprintf(fp,"Tag    : %4d, Identifier: %4d, Mode      : %4d\n",tag,id,mode);
fprintf(fp,"Counter: %4d\n",counter);
for(unsigned i=0;i<Z_TIMES;i++) fprintf(fp,"Timestamp[%u] : %e\n",i,ztime[i]);
for(unsigned i=0;i<Z_NAMES;i++) fprintf(fp,"Names    [%u] : %s\n",i,names[i].c_str());
fprintf(fp,"Labels : %03d(0x%04x)|%03d(0x%04x)|%03d(0x%04x)|%03d(0x%04x)\n",
        L(0),L(0),L(1),L(1),L(2),L(2),L(3),L(3));
fprintf(fp,"Raw data dump:\n");
WALKMAP(int,typemap *,Tmap,i) {
  fprintf(fp,"Type key = %d (%s)\n",(*i).first,t2imap[(*i).first]);
  fprintf(fp,"Type length = %d\n",(*i).second->typelen);
  WALKMAP(int,SS *,(*i).second->Dmap,j) {
    fprintf(fp,"  User key = %d\n",(*j).first);
    fprintf(fp,"  Byte count = %d\n  ",(*j).second->c);
    for(int k=0;k<(*j).second->c;k++)
      fprintf(fp,"%02x%s",(*j).second->p[k],(k+1)%16==0?"\n  ":" ");
    fprintf(fp,"\n\n");
  }
}
for (int i=1;i<typecount;i++) fprintf(fp,"%2d <-> %-20s\n",i,t2imap[i]);

fprintf(fp,"Msg_p---------------------------------------------\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void Msg_p::Get(int k,string & s)
// Foul, I know, but I can't partially specialise a template function  (i.e.
// template <class T> T * Msg_p::Get(int k,int & cnt), if the templated class
// isn't part of the function signature; I could only change the signature.
{
int len;
char * pc = Get<char>(k,len);          // Null return is legitimate
s = string(pc,len);
}

//------------------------------------------------------------------------------

void Msg_p::GetX(int k,vector<string> & vs)
{
string sbuf;                           // Holder for packed string
Get(k,sbuf);                           // Unload the packed string
vs.clear();                            // Clear the output
unsigned int nfc=0;                    // Initialise next free cell
while (nfc+1<sbuf.size()) {            // Loop through the packed string
                                       // Pull out a null-terminated string
  string str = string(&sbuf.c_str()[nfc]);
  vs.push_back(str);
  nfc += (unsigned int)(str.size()+1); // Move to start of next string
}

}

//------------------------------------------------------------------------------

int Msg_p::Id()
{
return id;
}

//------------------------------------------------------------------------------

void Msg_p::Id(int i)
{
id = i;
vm.clear();                            // Stream vector now dirty
}

//------------------------------------------------------------------------------

unsigned Msg_p::Key()
{
unsigned ans = 0;
for (unsigned i=0;i<Z_FIELDS;i++)
  ans = (ans|subkey[i])<<(i==Z_FIELDS-1 ? 0 : BITSPERBYTE);
return ans;
}

//------------------------------------------------------------------------------

void Msg_p::Key(byte L0,byte L1,byte L2,byte L3)
{
subkey[0] = L0;
subkey[1] = L1;
subkey[2] = L2;
subkey[3] = L3;
}

//------------------------------------------------------------------------------

unsigned Msg_p::KEY(byte L0,byte L1,byte L2,byte L3)
{
unsigned ans = 0;
ans |= L0; ans <<= BITSPERBYTE;
ans |= L1; ans <<= BITSPERBYTE;
ans |= L2; ans <<= BITSPERBYTE;
ans |= L3;
return ans;
}

//------------------------------------------------------------------------------

void Msg_p::L(int index,byte val)
// Set a subkey field
{
int i = int(index);
if ((i<0)||(i>=static_cast<const int>(Z_FIELDS))) return;
vm.clear();                            // Stream vector is now dirty
subkey[i] = val;
}

//------------------------------------------------------------------------------

byte Msg_p::L(int index)
// Retrieve a subkey field
{
int i = int(index);
if ((i<0)||(i>=static_cast<const int>(Z_FIELDS))) return 0xff;
return subkey[i];
}

//------------------------------------------------------------------------------

int Msg_p::Length()
// Retrieve length of streamed message
{
if (vm.empty()) Stream();
return (int)vm.size();
}

//------------------------------------------------------------------------------

void Msg_p::Load(byte * tm, int len)
// This routine knows nothing of types; *everything* is a byte string.
{
                                       // Copy the data somewhere safe
for(int i=0;i<len;i++) vm.push_back(tm[i]);
int nfc = 0;                           // Next free cell in byte stream
int msize = pullB<int>(nfc);           // Message size in bytes
//printf("Just read message size<int> = %d\n",msize);
tag = pullB<int>(nfc);                 // Unload the tag
//printf("Just read tag<int> = %d\n",tag);
src = pullB<int>(nfc);                 // Unload the source
tgt = pullB<int>(nfc);                 // Unload the target
for(unsigned i=0;i<Z_TIMES;i++) ztime[i] = pullB<double>(nfc);  // Timestamps
id = pullB<int>(nfc);                  // Identifier
mode = pullB<int>(nfc);                // Mode
for(unsigned i=0;i<Z_FIELDS;i++) subkey[i] = pullB<int>(nfc);
for(unsigned i=0;i<Z_NAMES;i++) names[i] = pullS(nfc);
int elmapT = pullB<int>(nfc);          // Elements in Tmap
//printf("Just read Tmap size<int> = %d\n",elmapT);
for (int i = 0;i<elmapT;i++) {
  int keyT = pullB<int>(nfc);          // Key for this type (not seen before)
//  printf("Just read typekey<int> = %d\n",keyT);
  byte typelen = pullB<byte>(nfc);     // Length of this type in bytes
//  printf("Just read typelen<byte> = %x\n",typelen);
  Tmap[keyT] = new typemap(typelen);
  int elmapD = pullB<int>(nfc);        // Elements in Dmap
//  printf("Just read Dmap size<int> = %d\n",elmapD);
  for (int j = 0;j<elmapD;j++) {
    int keyD = pullB<int>(nfc);        // Key for this data item
    Tmap[keyT]->Dmap[keyD] = new SS();
//    printf("Just read data key<int> = %d\n",keyD);
    int itcnt = pullB<int>(nfc);       // Data item count
//    printf("Just read item count<int> = %d\n",itcnt);
    byte * pB = new byte[itcnt];       // Storage for data
    for (int k=0;k<itcnt;k++) {        // Pull it in
      pB[k] = pullB<byte>(nfc);
//      printf("Just read data item<byte> = %x\n",pB[k]);
    }
    Tmap[keyT]->Dmap[keyD]->p=pB;      // Attach to datastructure
    Tmap[keyT]->Dmap[keyD]->c=itcnt;   // Byte count
  }
}
assert(msize==nfc);
}

//------------------------------------------------------------------------------

int Msg_p::Mode()
{
return mode;
}

//------------------------------------------------------------------------------

void Msg_p::Mode(int m)
{
mode = m;
vm.clear();                            // Stream vector now dirty
}

//------------------------------------------------------------------------------

string Msg_p::pullS(int & nfc)
// Pull a (Hollerith-defined) string off the buffer
{
unsigned len = pullB<unsigned>(nfc);   // Get the length
string ans;
for(unsigned i=0;i<len;i++) ans += pullB<char>(nfc);
return ans;
}

//------------------------------------------------------------------------------

void Msg_p::pushS(vector<byte> & v,string x)
// Push a string onto the streaming buffer as a Hollerith literal:
// {unsigned length, byte sequence}
{
unsigned len = x.size();               // Length of string
pushB<unsigned>(v,len);                // Store it
for(unsigned i=0;i<len;i++) v.push_back(x[i]);
}

//------------------------------------------------------------------------------

void Msg_p::Put(int k,string * data)
// Special case for strings 'cos they're so useful
{
//Put<char>(k,const_cast<char *>(data->c_str()),data->size()+1);
Put<char>(k,const_cast<char *>(data->c_str()),data->size());
}

//------------------------------------------------------------------------------

void Msg_p::PutX(int k,vector<string> * data)
// Special special case for vectors of strings, 'cos.....
// This could go on all night. There has to be a better way.
// And also, to piss me off even more: BORLAND BUG..... the compiler cannot
// or will not disambiguate
// template <class T> void Put(int,vector<T> *);
// and                void Put(int,vector<string> *); - hence the silly name
{
int len=0;                             // Total size?
WALKVECTOR(string,(*data),i) len += int((*i).size());
len += int(data->size())+1;
char * buf = new char[len];
int nfc=0;
WALKVECTOR(string,(*data),i) {
  for(unsigned int j=0;j<(*i).size();j++) buf[nfc++] = (*i)[j];
  buf[nfc++] = char(0);
}
buf[nfc] = char(0);
Put<char>(k,&buf[0],len);
delete [] buf;
}

//------------------------------------------------------------------------------

unsigned Msg_p::Sizeof(byte * vm)
// Routine to establish the size of a byte stream that is a streamed Msg_p; i.e.
// we simply read the first integer for the byte vector. If it's gerbage, we get
// garbage - there is no defense.
{
if (vm==0) return 0;                   // Paranoia
int nfc = 0;                           // Length is at position 0
int len = * new(&vm[nfc]) int;
return (unsigned)len;
}

//------------------------------------------------------------------------------

int Msg_p::Src()
{
return src;
}

//------------------------------------------------------------------------------

void Msg_p::Src(int s)
{
src = s;
vm.clear();                            // Stream vector now dirty
}

//------------------------------------------------------------------------------

byte * Msg_p::Stream()
// Routine to take all the byte vector fragments in the data structure and
// bolt them together to form a single byte stream that can be piped through MPI
// Nothing in this routine needs/knows about any typing.
{
if (!vm.empty()) return &vm[0];
//printf("About to write (placeholder)0<int> = %d\n",0);
int offset = (int)vm.size();           // Placeholder offset
pushB<int>(vm,0);                      // Placeholder for total message size
//printf("About to write tag<int> = %d\n",tag);
pushB<int>(vm,tag);                    // Message tag
pushB<int>(vm,src);                    // Message source
pushB<int>(vm,tgt);                    // Message target
for(unsigned i=0;i<Z_TIMES;i++) pushB<double>(vm,ztime[i]); // Timestamps
pushB<int>(vm,id);                     // Identifier
pushB<int>(vm,mode);                   // Mode
for(unsigned i=0;i<Z_FIELDS;i++) pushB<int>(vm,subkey[i]);
for(unsigned i=0;i<Z_NAMES;i++) pushS(vm,names[i]);
//printf("About to write Tmap.size()<int> = %d\n",Tmap.size());
pushB<int>(vm,int(Tmap.size()));       // Number of elements in the Tmap
WALKMAP(int,typemap *,Tmap,i) {        // Walk the Tmap
  typemap * pTm = (*i).second;
//  printf("About to write typekey<int> = %d\n",(*i).first);
  pushB<int>(vm,(*i).first);           // Type key
//  printf("About to write type length<byte> = %x\n",pTm->typelen);
  pushB<byte>(vm,pTm->typelen);        // Type length
//  printf("About to write Dmap size<int> = %d\n",pTm->Dmap.size());
  pushB<int>(vm,int(pTm->Dmap.size()));// Size of Dmap
  WALKMAP(int,SS *,pTm->Dmap,j) {
    SS * pSS = (*j).second;
//    printf("About to write user key<int> = %d\n",(*j).first);
    pushB<int>(vm,(*j).first);         // Item key (user label}
//    printf("About to write byte count<int> = %d\n",pSS->c);
    pushB<int>(vm,pSS->c);             // Byte count
//    printf("About to write the data itself\n");
    for(int k=0;k<pSS->c;k++) {
//      printf("About to write data<byte> = %02x\n",pSS->p[k]);
      pushB<byte>(vm,pSS->p[k]);
    }
  }
}
//WALKVECTOR(byte,vm,i) printf("%02x ",*i);
//printf("\n\n");
//printf("About to overwrite message size with<int> = %d\n",vm.size());
//int offset = sizeof(int)               // The tag
//           + sizeof(double)            // The timestamp
//           + (FIELDS * sizeof(byte));  // The identifier fields
new(&vm[offset]) int(vm.size());       // Write message length into placeholder

//WALKVECTOR(byte,vm,i) printf("%02x ",*i);
//printf("\n\n");
return &vm[0];
}

//------------------------------------------------------------------------------

void Msg_p::SubKey(byte * sk)
{
sk = subkey;
}

//------------------------------------------------------------------------------

void Msg_p::Tag(int t)
// Set message tag
{
vm.clear();                            // Stream vector now dirty
tag = t;
}

//------------------------------------------------------------------------------

int Msg_p::Tag()
// Guess
{
return tag;
}

//------------------------------------------------------------------------------

int Msg_p::Tgt()
{
return tgt;
}

//------------------------------------------------------------------------------

void Msg_p::Tgt(int t)
{
tgt = t;
vm.clear();                            // Stream vector now dirty
}

//------------------------------------------------------------------------------

string Msg_p::Zname(unsigned i)
// Return name
{
return names[i];
}

//------------------------------------------------------------------------------

void Msg_p::Zname(unsigned i,string s)
// Set name
{
vm.clear();                            // Stream vector now dirty
names[i] = s;
}

//------------------------------------------------------------------------------

void Msg_p::Ztime(int i,double t)
// Set message timestamp
{
vm.clear();                            // Stream vector now dirty
ztime[i] = t;
}

//------------------------------------------------------------------------------

double Msg_p::Ztime(int i)
{
return ztime[i];
}

//==============================================================================

bool operator == (Msg_p & a,Msg_p & b)
{
if (a.vm.size()==0) a.Stream();
if (b.vm.size()==0) b.Stream();
if (a.vm.size()!=b.vm.size()) return false;
for (unsigned int i=0;i<a.vm.size();i++) if (a.vm[i]!=b.vm[i]) return false;
return true;
}

//------------------------------------------------------------------------------

bool operator != (Msg_p & a,Msg_p & b)
{
return !(a==b);
}
          
//==============================================================================


//------------------------------------------------------------------------------

#include "Ns_el.h"

//==============================================================================

Ns_el::Ns_el()
{
Put<Dindex_t>();
Put<Sindex_t>();
Put<Tindex_t>();
Put<Oindex_t>();
}

//------------------------------------------------------------------------------

Ns_el::Ns_el(byte * s,int l):PMsg_p(s,l)
{
Put<Dindex_t>();
Put<Sindex_t>();
Put<Tindex_t>();
Put<Oindex_t>();
}

//------------------------------------------------------------------------------

Ns_el::Ns_el(PMsg_p * p):PMsg_p(*p)
{
Put<Dindex_t>();
Put<Sindex_t>();
Put<Tindex_t>();
Put<Oindex_t>();
Dump();
}

//------------------------------------------------------------------------------

Ns_el::~Ns_el()
{
}

//------------------------------------------------------------------------------

vector<Ns_0 *> * Ns_el::Construct()
// Receiver side (NameServer) function:
// Routine to build and expose a vector of entities derived from the current
// state of the object.
// If there's a problem - and there shouldn't be - a entity type 'X' is written
// to the output vector and the routine returns prematurely (i.e. the 'X' entity
// is the last in the returned vector). Subsequent parts of the PMsg_p are
// ignored.
{
Ns_X * pX = new Ns_X();                // Error entity
static vector<Ns_0 *> vN;
vN.clear();                            // Output entity vector
int cnt;                               // Stored key vector size
unsigned * ploc = Get<unsigned>(0,cnt);// Get hold of the key vector
try {
  if (ploc==0) throw((*pX)("KEY VECTOR"));    // If it's not there....
  for (int i=0;i<cnt;i++,(*ploc)++) {    // Unpack, one key at a time
    int c2;                            // Item count (not used)
    unsigned char * pEtype = Get<unsigned char>(*ploc,c2);
    if (pEtype==0) throw((*pX)("ENTITY TYPE"));
    vector<string> vs;                 // Holder for the packed name vector
    Ns_0 * p;                          // Entity
    GetX(*ploc,vs);                    // Slurp it out
    switch (*pEtype) {
      case 'D' : { Ns_el::Dindex_t * pDindex = Get<Ns_el::Dindex_t>(*ploc,c2);
                   if (pDindex==0) throw((*pX)("DEVICE INDEX"));
                   p = new Ns_dev(pDindex,vs);   // No idea why -
                   vN.push_back(p);                     // Another BORLAND bug?
                 } break;
      case 'S' : { Ns_el::Sindex_t * pSindex = Get<Ns_el::Sindex_t>(*ploc,c2);
                   if (pSindex==0) throw((*pX)("SUPERVISOR INDEX"));
                   p = new Ns_sup(pSindex,vs);   // No idea why -
                   vN.push_back(p);                     // Another BORLAND bug?
                 } break;
      case 'T' : { Ns_el::Tindex_t * pTindex = Get<Ns_el::Tindex_t>(*ploc,c2);
                   if (pTindex==0) throw((*pX)("TASK INDEX"));
                   p = new Ns_tsk(pTindex,vs);   // No idea why -
                   vN.push_back(p);                     // Another BORLAND bug?
                 } break;
      case 'O' : { Ns_el::Oindex_t * pOindex = Get<Ns_el::Oindex_t>(*ploc,c2);
                   if (pOindex==0) throw((*pX)("OWNER INDEX"));
                   p = new Ns_own(pOindex,vs);   // No idea why -
                   vN.push_back(p);                     // Another BORLAND bug?
                 } break;
      default  : throw((*pX)("UNRECOGNISED ENTITY TYPE"));
    } // switch
    p->type = *pEtype;
    p->key  = *ploc;
  } // for
} // try
catch(Ns_0 * X) {                      // Cockup: Push the already allocated
  vN.push_back(X);                     // error entity onto the vector and bail
  return &vN;
}
delete pX;                             // Don't need the error entity
return &vN;
}

//------------------------------------------------------------------------------

void Ns_el::Dump(unsigned off,FILE * fp)
// The usual, but the logic is a bit fiddly here. We want to dump the *contents*
// of the PMsg_p class that is the base class of Ns_el. Calling PMsg_p::Dump()
// will not do the biz, because PMsg_p doesn't know anything about types, so the
// Dump() will be largely impenetrable. The only state variable of Ns_el is
// keyv, the vector of keys that have currently been used to load the class.
// So we walk keyv, extracting the (integer) keys. We can use these to get the
// entity types: Get<char>(keyv[?],...), and this gives us the complete
// signature for the entity: {key,type}.
// If keyv is empty, this is either because the object is genuinely empty,
// or because the POETS entities have been streamed to the internal byte
// buffer.
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sNs_el ++++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sCurrent key vector (%lu elements):\n",os,keyv.size());
WALKVECTOR(unsigned,keyv,i)fprintf(fp,"%s%3u\n",os,*i);
fprintf(fp,"%s+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + \n",os);
fprintf(fp,"%sWalking the entities; dumping with unstreamed keys:\n",os);
                                       // Dump with the unstreamed keys
WALKVECTOR(unsigned,keyv,i) Dump0(off+2,fp,*i);
fprintf(fp,"%s+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + \n",os);
fprintf(fp,"%sWalking the entities; dumping with stored key vector:\n",os);
int cnt;                               // Stored key vector size
unsigned * ploc = Get<unsigned>(0,cnt);// Get hold of the key vector
fprintf(fp,"%sStored key vector has %u elements\n",os,cnt);
                                       // Dump with streamed keys
if (ploc!=0) for(int i=0;i<cnt;i++) Dump0(off+2,fp,*ploc++);
// If there are any duplicates, there shouldn't be
fprintf(fp,"%s+ + + + + + + + + + + + + + + + + + + + + + + + + + + + ++\n",os);
fprintf(fp,"%sDown and dirty dump:\n",os);
PMsg_p::Dump(fp);
fprintf(fp,"%sNs_el ----------------------------------------------------\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void Ns_el::Dump0(unsigned off,FILE * fp,unsigned k)
// Dump the entity with the POETS key k
// There is an argument here for combining the four index types. They form a
// nested subset sequence, and the top ones (owner and task) - these being the
// ones with the potentialy wasted space - will be relatively rare.
{
string s(off,' ');
const char * os = s.c_str();
int cnt;
unsigned char * pEtype = Get<unsigned char>(k,cnt);
unsigned char Etype = 'X';
if (pEtype==0) fprintf(fp,"%s*** TYPE UNLOAD FAIL ***\n",os);
else Etype = *pEtype;
vector<string> vs;
switch (Etype) {
  case 'D' : { fprintf(fp,"\n%sDevice key %3u:\n",os,k);
               Ns_el::Dindex_t Dindex;
               Ns_el::Dindex_t * pDindex = Get<Ns_el::Dindex_t>(k,cnt);
               if (pDindex==0) {
                 fprintf(fp,"%s*** DEVICE INDEX UNLOAD FAIL ***\n",os);
                 break;
               }
               else Dindex = *pDindex;
               fprintf(fp,"%skey       = %u\n",os,Dindex.key);
               if (Dindex.key!=k) fprintf(fp,"%s*** KEY MISMATCH ***\n",os);
               Dindex.addr.Dump(off+2,fp);
               fprintf(fp,"%sattr      = %u\n",os,Dindex.attr);
               fprintf(fp,"%sbin       = %u\n",os,Dindex.bin);
               fprintf(fp,"%sinP(size) = %u\n",os,Dindex.inP/2);
               fprintf(fp,"%souP(size) = %u\n",os,Dindex.ouP/2);
               GetX(k,vs);
               fprintf(fp,"%sName vector (%lu elements)\n",os,vs.size());
               WALKVECTOR(string,vs,i) fprintf(fp,"%s%s\n",os,(*i).c_str());
             } break;
  case 'S' : { fprintf(fp,"\n%sSupervisor key %3u:\n",os,k);
               Ns_el::Sindex_t Sindex;
               Ns_el::Sindex_t * pSindex = Get<Ns_el::Sindex_t>(k,cnt);
               if (pSindex==0) {
                 fprintf(fp,"%s*** SUPERVISOR INDEX UNLOAD FAIL ***\n",os);
                 break;
               }
               else Sindex = *pSindex;
               fprintf(fp,"%skey       = %u\n",os,Sindex.key);
               if (Sindex.key!=k) fprintf(fp,"%s*** KEY MISMATCH ***\n",os);
               Sindex.addr.Dump(off+2,fp);
               fprintf(fp,"%sattr      = %u\n",os,Sindex.attr);
               fprintf(fp,"%sbin       = %u\n",os,Sindex.bin);
               GetX(k,vs);
               fprintf(fp,"%sName vector (%lu elements)\n",os,vs.size());
               WALKVECTOR(string,vs,i) fprintf(fp,"%s%s\n",os,(*i).c_str());
             } break;
  case 'T' : { fprintf(fp,"\n%sTask key %3u:\n",os,k);
               Ns_el::Tindex_t Tindex;
               Ns_el::Tindex_t * pTindex = Get<Ns_el::Tindex_t>(k,cnt);
               if (pTindex==0) {
                 fprintf(fp,"%s*** TASK INDEX UNLOAD FAIL ***\n",os);
                 break;
               }
               else Tindex = *pTindex;
               fprintf(fp,"%skey       = %u\n",os,Tindex.key);
               if (Tindex.key!=k) fprintf(fp,"%s*** KEY MISMATCH ***\n",os);
               GetX(k,vs);
               fprintf(fp,"%sName vector (%lu elements)\n",os,vs.size());
               WALKVECTOR(string,vs,i) fprintf(fp,"%s%s\n",os,(*i).c_str());
             } break;

  case 'O' : { fprintf(fp,"\n%sOwner key %3u:\n",os,k);
               Ns_el::Oindex_t Oindex;
               Ns_el::Oindex_t * pOindex = Get<Ns_el::Oindex_t>(k,cnt);
               if (pOindex==0) {
                 fprintf(fp,"%s*** OWNER INDEX UNLOAD FAIL ***\n",os);
                 break;
               }
               else Oindex = *pOindex;
               fprintf(fp,"%skey       = %u\n",os,Oindex.key);
               if (Oindex.key!=k) fprintf(fp,"%s*** KEY MISMATCH ***\n",os);
               GetX(k,vs);
               fprintf(fp,"%sName vector (%lu elements)\n",os,vs.size());
               WALKVECTOR(string,vs,i) fprintf(fp,"%s%s\n",os,(*i).c_str());
             } break;
  default  :   fprintf(fp,"%s*** UNRECOGNISED ENTITY TYPE ***\n",os);
}

}

//------------------------------------------------------------------------------

int Ns_el::Length()
// Retrieve length of streamed message
{
if (!keyv.empty()) {
  Put<unsigned>(0,&keyv[0],keyv.size()); // Load the key list
  keyv.clear();                          // Everything is in the stream buffer
}
Msg_p::Stream();
return (int)Msg_p::vm.size();
}

//------------------------------------------------------------------------------

void Ns_el::PutD(string Dname,             // Device name
                 string Dtype,             // Device type
                 string Sname,             // Supervisor name
                 string Tname,             // Task name
                 string Oname,             // Owner name
                 vector<string> inpin,     // Device input pin names
                 vector<string> inpintype, // Device input pin types
                 vector<string> oupin,     // Device output pin names
                 vector<string> oupintype, // Device output pin types
                 unsigned key,             // PMsg_p key
                 P_addr_t addr,            // POETS hardware address
                 unsigned attr,            // Device attribute
                 unsigned bin)             // Binary file indentifier
{
struct Ns_el::Dindex_t Dindex;
// This rather clunky initialisation section is the highest common factor of my
// three C++ compilers.....
Dindex.key  = key;                     // PMsg_p key
Dindex.addr = addr;                    // POETS composite address
Dindex.attr = attr;                    // POETS device attribute
Dindex.bin  = bin;                     // ..and so on: see above
Dindex.inP  = inpin.size() + inpintype.size();
Dindex.ouP  = oupin.size() + oupintype.size();
vector<string> vs;                     // Name container
vs.push_back(Dname);                   // Device name
vs.push_back(Dtype);                   // Device type
vs.push_back(Sname);                   // Supervisor name
vs.push_back(Tname);                   // Task name
vs.push_back(Oname);                   // Owner name
vs.insert(vs.end(),inpin.begin(),inpin.end());
vs.insert(vs.end(),inpintype.begin(),inpintype.end());
vs.insert(vs.end(),oupin.begin(),oupin.end());
vs.insert(vs.end(),oupintype.begin(),oupintype.end());
unsigned char Entity = 'D';
Put<unsigned char>(key,&Entity);       // Entity type (device/supervisor...)
Put<Dindex_t>(key,&Dindex);            // Index structure
PutX(key,&vs);                         // All the strings in one container
keyv.push_back(key);                   // Add to the vector of used keys
}

//------------------------------------------------------------------------------

void Ns_el::PutO(string Oname,         // Owner name
                 unsigned key)         // PMsg_p key
{
struct Ns_el::Oindex_t Oindex;
// This rather clunky initialisation section is the highest common factor of my
// three C++ compilers.....
Oindex.key  = key;                     // PMsg_p key
vector<string> vs;                     // Name container
vs.push_back(Oname);                   // Owner name
unsigned char Entity = 'O';
Put<unsigned char>(key,&Entity);       // Entity type (device/supervisor...)
Put<Oindex_t>(key,&Oindex);            // Index structure
PutX(key,&vs);                         // All the strings in one container
keyv.push_back(key);                   // Add to the vector of used keys
}

//------------------------------------------------------------------------------

void Ns_el::PutS(string Sname,         // Supervisor name
                 string Tname,         // Task name
                 string Oname,         // Owner name
                 unsigned key,         // PMsg_p key
                 P_addr_t addr,        // POETS hardware address
                 unsigned attr,        // Device attribute
                 unsigned bin)         // Binary file indentifier
{
struct Ns_el::Sindex_t Sindex;
// This rather clunky initialisation section is the highest common factor of my
// three C++ compilers.....
Sindex.key  = key;                     // PMsg_p key
Sindex.addr = addr;                    // POETS composite address
Sindex.attr = attr;                    // POETS device attribute
Sindex.bin  = bin;                     // ..and so on: see above
vector<string> vs;                     // Name container
vs.push_back(Sname);                   // Supervisor name
vs.push_back(Tname);                   // Task name
vs.push_back(Oname);                   // Owner name
unsigned char Entity = 'S';
Put<unsigned char>(key,&Entity);       // Entity type (device/supervisor...)
Put<Sindex_t>(key,&Sindex);            // Index structure
PutX(key,&vs);                         // All the strings in one container
keyv.push_back(key);                   // Add to the vector of used keys
}

//------------------------------------------------------------------------------

void Ns_el::PutT(string Tname,         // Task name
                 string Oname,         // Owner name
                 unsigned key)         // PMsg_p key
{
struct Ns_el::Tindex_t Tindex;
// This rather clunky initialisation section is the highest common factor of my
// three C++ compilers.....
Tindex.key  = key;                     // PMsg_p key
vector<string> vs;                     // Name container
vs.push_back(Tname);                   // Task name
vs.push_back(Oname);                   // Owner name
unsigned char Entity = 'T';            // Guess
Put<unsigned char>(key,&Entity);       // Entity type (device/supervisor...)
Put<Tindex_t>(key,&Tindex);            // Index structure
PutX(key,&vs);                         // All the strings in one container
keyv.push_back(key);                   // Add to the vector of used keys
}

//==============================================================================

Ns_X * Ns_X::operator()(string s)
{
msg = s;
return this;
}

//==============================================================================

Ns_dev::Ns_dev(Ns_el::Dindex_t * pDindex,vector<string> & vs)
{
addr  = pDindex->addr;
attr  = pDindex->attr;
bin   = pDindex->bin;
Ename = vs[0];
Etype = vs[1];
Sname = vs[2];
Tname = vs[3];
Oname = vs[4];
vector<string>::iterator inbegin     = vs.begin()+5;
vector<string>::iterator inend       = inbegin + pDindex->inP/2;
vector<string>::iterator intypebegin = inend;
vector<string>::iterator intypeend   = intypebegin + pDindex->inP/2;
vector<string>::iterator oubegin     = intypeend;
vector<string>::iterator ouend       = oubegin + pDindex->ouP/2;
vector<string>::iterator outypebegin = ouend;
vector<string>::iterator outypeend   = outypebegin + pDindex->ouP/2;
inpin     = vector<string>(inbegin,inend);
inpintype = vector<string>(intypebegin,intypeend);
oupin     = vector<string>(oubegin,ouend);
oupintype = vector<string>(outypebegin,outypeend);
}

//------------------------------------------------------------------------------

void Ns_dev::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sNs_dev +++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
addr.Dump(off+2,fp);
fprintf(fp,"%sDevice type = %s\n",os,Etype.c_str());
fprintf(fp,"%sSuper name  = %s\n",os,Sname.c_str());
fprintf(fp,"%sTask name   = %s\n",os,Tname.c_str());
fprintf(fp,"%sOwner name  = %s\n",os,Oname.c_str());
fprintf(fp,"%sattr        = %u\n",os,attr);
fprintf(fp,"%sbin         = %u\n",os,bin);
fprintf(fp,"\n%sInput pin name vector (%lu elements)\n",os,inpin.size());
WALKVECTOR(string,inpin,i) fprintf(fp,"%s %s\n",os,(*i).c_str());
fprintf(fp,"\n%sInput pin type name vector (%lu elements)\n",os,inpintype.size());
WALKVECTOR(string,inpintype,i) fprintf(fp,"%s %s\n",os,(*i).c_str());
fprintf(fp,"\n%sOutput pin name vector (%lu elements)\n",os,oupin.size());
WALKVECTOR(string,oupin,i) fprintf(fp,"%s %s\n",os,(*i).c_str());
fprintf(fp,"\n%sOutput pin type name vector (%lu elements)\n",os,oupintype.size());
WALKVECTOR(string,oupintype,i) fprintf(fp,"%s %s\n",os,(*i).c_str());
Ns_0::Dump(off+2,fp);
fprintf(fp,"%sNs_dev ---------------------------------------------------\n",os);
}

//==============================================================================

Ns_0::Ns_0()
{
}

//------------------------------------------------------------------------------

void Ns_0::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sNs_0 +++++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%skey         = %u\n",os,key);
fprintf(fp,"%stype        = %c\n",os,type);
fprintf(fp,"%sentity name = %s\n",os,Ename.c_str());
fprintf(fp,"%sNs_0 -----------------------------------------------------\n",os);
}

//==============================================================================

Ns_sup::Ns_sup(Ns_el::Sindex_t * pSindex,vector<string> & vs)
{
Ename = vs[0];
Tname = vs[1];
Oname = vs[2];
addr = pSindex->addr;
attr = pSindex->attr;
bin  = pSindex->bin;
}

//------------------------------------------------------------------------------

void Ns_sup::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sNs_sup +++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
addr.Dump(off+2,fp);
fprintf(fp,"%sTask name   = %s\n",os,Tname.c_str());
fprintf(fp,"%sOwner name  = %s\n",os,Oname.c_str());
fprintf(fp,"%sattr        = %u\n",os,attr);
fprintf(fp,"%sbin         = %u\n",os,bin);
Ns_0::Dump(off+2,fp);
fprintf(fp,"%sNs_sup ---------------------------------------------------\n",os);
}

//==============================================================================

Ns_tsk::Ns_tsk(Ns_el::Tindex_t *,vector<string> & vs)
{
Ename = vs[0];
Oname = vs[1];
}

//------------------------------------------------------------------------------

void Ns_tsk::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sNs_tsk +++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sOwner name  = %s\n",os,Oname.c_str());
Ns_0::Dump(off+2,fp);
fprintf(fp,"%sNs_tsk ---------------------------------------------------\n",os);
}

//==============================================================================

Ns_own::Ns_own(Ns_el::Oindex_t *,vector<string> & vs)
{
Ename = vs[0];
}

//------------------------------------------------------------------------------

void Ns_own::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sNs_own +++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
Ns_0::Dump(off+2,fp);
fprintf(fp,"%sNs_own ---------------------------------------------------\n",os);
}

//==============================================================================

//------------------------------------------------------------------------------

#include "NameBase.h"
#include "macros.h"
#include <stdint.h>

#include "OSFixes.hpp"

unsigned NameBase::uid = 0;
map<unsigned,NameBase *> NameBase::NBmap;

//==============================================================================

NameBase::NameBase()
{
npar  = 0;                             // No name parent
name  = string("**undefined**");
id    = Uid();                         // Unique numeric identifier
rtrap = false;                         // Recursion trap unset
}

//------------------------------------------------------------------------------

NameBase::~NameBase()
{
//printf("********* NameBase %s (%x) destructor\n",name.c_str,id); fflush(stdout);
}

//------------------------------------------------------------------------------

string NameBase::AutoName(string s)
// Generate the name automatically by appending the UID to the string s
{
char buf[32];
sprintf(buf,"%04u",Id());
string n = s+buf;
Name(n);
return n;
}

//------------------------------------------------------------------------------

void NameBase::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sNameBase dump+++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sthis           %" PTR_FMT "\n",os,reinterpret_cast<uint64_t>(this));
fprintf(fp,"%sName           %s\n",os,name.c_str());
fprintf(fp,"%sId             %10u(%#010x)\n",os,id,id);
fprintf(fp,"%sParent         %" PTR_FMT "\n",os,reinterpret_cast<uint64_t>(npar));
fprintf(fp,"%sRecursion trap %s\n",os,rtrap ? "Set" : "Unset");
fprintf(fp,"%sUnique id      %u\n",os,uid);
fprintf(fp,"%sNameBase id    Name\n,os");
if (NBmap.empty()) fprintf(fp,"%s ** No map entries ** \n",os);
WALKMAP(unsigned,NameBase *,NBmap,i)
  fprintf(fp,"%s%6u : %s\n",os,(*i).first,(*i).second->FullName(7).c_str());
fprintf(fp,"%sNameBase dump-------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

NameBase * NameBase::Find(unsigned id)
// Locate the NameBase object with the provided id
{
if (NameBase::NBmap.find(id)==NameBase::NBmap.end()) return 0;   // Not there?
return NameBase::NBmap[id];
}

//------------------------------------------------------------------------------

string NameBase::FullName(unsigned d)
// Return component full hierarchical name.
// Argument indicates amount of decoration:
// 0 - none - bit pointless asking, really
// 1 - string
// 2 -          hex id
// 3 - string + hex id
// 4 -                   dec id
// 5 - string          + dec id
// 6 -          hex id + dec id
// 7 - string + hex id + dec id
{
                                       // Been here before?
if (rtrap) return "Recursive name " + name + " trapped";
rtrap = true;                          // Set "I've been here" flag
char buff[256];                        // Yes, I know. Get over it.
switch (d) {                           // OK, I got carried away
  case 0  : buff[0] = '\0';                                        break;
  case 1  : sprintf(buff,"%s",name.c_str());                       break;
  case 2  : sprintf(buff,"(%#010x)",id);                           break;
  case 3  : sprintf(buff,"%s(%#010x)",name.c_str(),id);            break;
  case 4  : sprintf(buff,"(%010u)",id);                            break;
  case 5  : sprintf(buff,"%s(%010u)",name.c_str(),id);             break;
  case 6  : sprintf(buff,"(%#010x=%010u)",id,id);                  break;
  case 7  : sprintf(buff,"%s(%#010x=%010u)",name.c_str(),id,id);   break;
  default : sprintf(buff,"????");                                  break;
}
string s = buff;                       // Initialise build of full name
if (npar!=0) s = npar->FullName(d) + "." + s;       // Add on the next bit
rtrap = false;                         // Unwind recursion trap for next time
return s;
}

//------------------------------------------------------------------------------

unsigned NameBase::Id()
// Return component numeric label
{
return id;
}

//------------------------------------------------------------------------------

void NameBase::Id(unsigned u)
// Set component numeric label and load search map
{
id = u;
NBmap[id] = this;
}

//------------------------------------------------------------------------------

string NameBase::Name()
// Return component short name
{
return name;
}

//------------------------------------------------------------------------------

void NameBase::Name(string s)
// Set component short name
{
name = s;
}

//------------------------------------------------------------------------------

NameBase * NameBase::Npar()
// Return component parent
{
return npar;
}

//------------------------------------------------------------------------------

void NameBase::Npar(NameBase * p)
// Set component parent
{
if (p==this) return;                   // Pre-emptively save us from ourselves
npar = p;
}

//------------------------------------------------------------------------------

unsigned NameBase::Uid()
// Return a NameBase-wide unique unsigned integer
{
return ++uid;
}

//==============================================================================





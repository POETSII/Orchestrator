//------------------------------------------------------------------------------

#include "DevI_t.h"
#include "GraphI_t.h"
#include "DevT_t.h"
#include "CFrag.h"
#include "PinI_t.h"
#include "Meta_t.h"

//==============================================================================

DevI_t::DevI_t(GraphI_t * G, string name):par(G),pT(0)
{
Name(name);
Npar(G);
Key = 0;
devTyp = 'U';
pPropsI = "";
pStateI = "";
}

//------------------------------------------------------------------------------

DevI_t::~DevI_t()
{
WALKVECTOR(Meta_t *,Meta_v,i) delete *i;
}

//------------------------------------------------------------------------------

void DevI_t::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
string sF = FullName();
fprintf(fp,"\n%sDevI_t +++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sNameBase       %s\n",os,sF.c_str());
fprintf(fp,"%sMe,Parent     %#018lx,%#018lx\n",os,(uint64_t)this,(uint64_t)par);
if (par!=0) fprintf(fp,"%s...%s\n",os,par->FullName().c_str());
fprintf(fp,"%sP-address :\n",os);
addr.Dump(off+2,fp);
fprintf(fp,"%sDevice instance type      %c\n",os,devTyp);
fprintf(fp,"%sDevice type link          %#018lx\n",os,(uint64_t)pT);
if (pT!=0) fprintf(fp,"%s...%s\n",os,pT->FullName().c_str());
fprintf(fp,"%sGraph device key          %u\n",os,Key);
fprintf(fp,"%sMetadata vector has %lu entries:\n",os,Meta_v.size());
WALKVECTOR(Meta_t *,Meta_v,i) (*i)->Dump(off+2,fp);
fprintf(fp,"%sScaffold name:pin map has %lu entries:\n",os,Pmap.size());
WALKMAP(string,PinI_t *,Pmap,i)
  fprintf(fp,"%s%s:%#018lx(%s)\n",
             os,(*i).first.c_str(),
             (uint64_t)((*i).second),(*i).second->FullName().c_str());
NameBase::Dump(off+2,fp);
DumpChan::Dump(off+2,fp);
DefRef::Dump(off+2,fp);
fprintf(fp,"%sDevI_t -------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void DevI_t::DevDat_cb(DevI_t * const & d)
// Debug callback for device data
{
if (d!=0) fprintf(dfp,"dev(D): %s",d->Name().c_str());
else fprintf(dfp,"dev(D): ***");
fflush(dfp);
}

//------------------------------------------------------------------------------

void DevI_t::DevKey_cb(unsigned const & u)
// Debug callback for device key
{
fprintf(dfp,"dev(K): %03u",u);
fflush(dfp);
}

//------------------------------------------------------------------------------

PinI_t * DevI_t::GetPin(string & rname)
// Get = one way or another - a pin instance of name "rname"
{
PinI_t * pP;                           // 'to' pin
map<string,PinI_t *>::iterator iP = Pmap.find(rname);
if (iP==Pmap.end()) {                  // Not there ?
  pP = new PinI_t(par,rname);          // Create it
  Pmap[rname] = pP;                    // Insert into pin map
}
else pP = Pmap[rname];                 // Yes, pre-existing: extract pin address
unsigned kP = UniU();                  // Unique pin key
pP->Key_v.push_back(kP);               // Store the key inside its data
// For the  pin, we now have address (pPto) and key (kPto)
return pP;
}

//------------------------------------------------------------------------------

void DevI_t::Par(GraphI_t * _p)
{
par = _p;
}

//==============================================================================

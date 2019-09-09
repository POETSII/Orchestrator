//------------------------------------------------------------------------------

#include "PinI_t.h"

set<PinI_t *> PinI_t::DelSet;

//==============================================================================

PinI_t::PinI_t(GraphI_t * _p,string _s):par(_p),pT(0),pPropsI(0),pStateI(0)
{
Name(_s);                              // Save name
tyId = _s;                             // The name of the pin *is* the type
Npar(_p);                              // Namebase parent
}

//------------------------------------------------------------------------------

PinI_t::~PinI_t()
// The problem here is that a single pin instance is pointed to by several
// pdigraph pins.
// The solution is to handle this at the next level up. The enclosing graph
// pushes all the candidate pin addresses into a static set, then calls
// static void PinI_t::Delete() to walk the set and delete everything in it.
{
if (pPropsI!=0) delete pPropsI;
if (pStateI!=0) delete pStateI;
}

//------------------------------------------------------------------------------

void PinI_t::AddKey(unsigned k)
{
Keyv.push_back(k);
}

//------------------------------------------------------------------------------

void PinI_t::Delete()
{
WALKSET(PinI_t *,DelSet,i) delete *i;
DelSet.clear();
}

//------------------------------------------------------------------------------

void PinI_t::Dump(FILE * fp)
{
fprintf(fp,"PinI_t+++++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      0x%#08p,0x%#08p\n",this,par);
fprintf(fp,"NameBase       %s\n",FullName().c_str());
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
fprintf(fp,"Pin type       %#08p\n",pT);
if (pT!=0) fprintf(fp,"...%s\n",pT->FullName().c_str());
fprintf(fp,"Properties initialiser    %#08p\n",pPropsI);
if (pPropsI!=0) pPropsI->Dump(fp);
fprintf(fp,"State initialiser         %#08p\n",pStateI);
if (pStateI!=0) pStateI->Dump(fp);
fprintf(fp,"Key vector has %u entries\n",Keyv.size());
WALKVECTOR(unsigned,Keyv,i) fprintf(fp,"%05u\n",*i);
NameBase::Dump(fp);
DumpChan::Dump(fp);
fprintf(fp,"PinI_t---------------------------------------\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void PinI_t::PinDat_cb(PinI_t * const & p)
// Debug callback for pin data
{
if (p!=0) fprintf(dfp,"pin(D): %s",p->Name().c_str());
else fprintf(dfp,"pin(D): ***");
fflush(dfp);
}

//------------------------------------------------------------------------------

void PinI_t::PinKey_cb(unsigned const & u)
// Debug callback for pin key
{
fprintf(dfp,"pin(K): %03u",u);
fflush(dfp);
}

//==============================================================================




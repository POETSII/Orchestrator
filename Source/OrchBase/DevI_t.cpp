//------------------------------------------------------------------------------

#include "DevI_t.h"
#include "GraphI_t.h"
#include "DevT_t.h"
#include "P_thread.h"

//==============================================================================

DevI_t::DevI_t(GraphI_t * G, string name):
          par(G),pP_thread(0),pT(0),pPropsI(0),pStateI(0)
{
Name(name);
Npar(G);
attr = 0;
idx  = 0;
Kdev = 0;
}

//------------------------------------------------------------------------------

DevI_t::~DevI_t()
{
WALKVECTOR(Meta_t *,Meta_v,i) delete *i;
}

//------------------------------------------------------------------------------

void DevI_t::Dump(FILE * fp)
{
string s = FullName();
fprintf(fp,"DevI_t %35s++++++++++++++++++++++++++++++++++++\n",s.c_str());
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      0x%#08p,0x%#08p\n",this,par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
fprintf(fp,"Properties initialiser    %#08p\n",pPropsI);
if (pPropsI!=0) pPropsI->Dump(fp);
fprintf(fp,"State initialiser         %#08p\n",pStateI);
if (pStateI!=0) pStateI->Dump(fp);
fprintf(fp,"Device type               %#08p\n",pT);
if (pT!=0) fprintf(fp,"...%s\n",pT->FullName().c_str());
fprintf(fp,"Thread cross-link         %#08p\n",pP_thread);
if (pP_thread!=0) fprintf(fp,"...%s\n",pP_thread->FullName().c_str());
fprintf(fp,"Attribute word   %u(0x%08x)\n",attr,attr);
fprintf(fp,"Graph key   %u\n",Kdev);
NameBase::Dump(fp);
DumpChan::Dump(fp);
fprintf(fp,"DevI_t %35s------------------------------------\n",s.c_str());
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
                   /*
vector<Entity::pin_t> DevI_t::NSGetinpn()
{
vector<Entity::pin_t> vs;
return vs;
}

//------------------------------------------------------------------------------

vector<Entity::pin_t> DevI_t::NSGetoupn()
{
vector<Entity::pin_t> vs;
return vs;
}
                     */
//------------------------------------------------------------------------------

void DevI_t::Par(GraphI_t * _p)
{
par = _p;
}

//------------------------------------------------------------------------------

void DevI_t::Unlink()
// Unlink this device from the topology database
{
                                       // Thread list
list<DevI_t *> * pL = &(pP_thread->P_devicel);
                                       // Zero the topology->graph link
WALKLIST(DevI_t *,(*pL),i) if ((*i)==this) (*i)=0;
pP_thread = 0;                         // Disconnect device from thread
}

//==============================================================================




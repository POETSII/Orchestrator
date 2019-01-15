//------------------------------------------------------------------------------

#include "P_device.h"
#include "D_graph.h"
#include "P_devtyp.h"
#include "PoetsThread.h"

//==============================================================================

P_device::P_device(D_graph * G, string name):
          par(G),pP_thread(0),pP_devtyp(0),pPropsI(0),pStateI(0)
{
Name(name);
Npar(G);
attr = 0;
idx  = 0;
}

//------------------------------------------------------------------------------

P_device::~P_device()
{
}

//------------------------------------------------------------------------------

void P_device::Dump(FILE * fp)
{
string s = FullName();
fprintf(fp,"P_device %35s++++++++++++++++++++++++++++++++++++\n",s.c_str());
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      0x%#08p,0x%#08p\n",this,par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
fprintf(fp,"Properties initialiser    %#08p\n",pPropsI);
if (pPropsI!=0) pPropsI->Dump(fp);
fprintf(fp,"State initialiser         %#08p\n",pStateI);
if (pStateI!=0) pStateI->Dump(fp);
fprintf(fp,"Device type               %#08p\n",pP_devtyp);
if (pP_devtyp!=0) fprintf(fp,"...%s\n",pP_devtyp->FullName().c_str());
fprintf(fp,"Thread cross-link         %#08p\n",pP_thread);
if (pP_thread!=0) fprintf(fp,"...%s\n",pP_thread->FullName().c_str());
fprintf(fp,"Attribute word   %u(0x%08x)\n",attr,attr);
NameBase::Dump(fp);
DumpChan::Dump(fp);
fprintf(fp,"P_device %35s------------------------------------\n",s.c_str());
fflush(fp);
}

//------------------------------------------------------------------------------

void P_device::DevDat_cb(P_device * const & d)
// Debug callback for device data
{
if (d!=0) fprintf(dfp,"dev(D): %s",d->Name().c_str());
else fprintf(dfp,"dev(D): ***");
fflush(dfp);
}

//------------------------------------------------------------------------------

void P_device::DevKey_cb(unsigned const & u)
// Debug callback for device key
{
fprintf(dfp,"dev(K): %03u",u);
fflush(dfp);
}

//------------------------------------------------------------------------------

vector<string> P_device::NSGetinpn()
{
vector<string> vs;
return vs;
}

//------------------------------------------------------------------------------

vector<string> P_device::NSGetinpt()
{
vector<string> vs;
return vs;
}

//------------------------------------------------------------------------------

vector<string> P_device::NSGetoupn()
{
vector<string> vs;
return vs;
}

//------------------------------------------------------------------------------

vector<string> P_device::NSGetoupt()
{
vector<string> vs;
return vs;
}

//------------------------------------------------------------------------------

void P_device::Par(D_graph * _p)
{
par = _p;
}

//------------------------------------------------------------------------------

void P_device::Unlink()
// Unlink this device from the topology database
{
                                       // Thread list
list<P_device *> * pL = &(pP_thread->PoetsDevices);
                                       // Zero the topology->task link
WALKLIST(P_device *,(*pL),i) if ((*i)==this) (*i)=0;
pP_thread = 0;                         // Disconnect device from thread
}

//==============================================================================

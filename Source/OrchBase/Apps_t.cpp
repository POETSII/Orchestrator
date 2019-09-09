//------------------------------------------------------------------------------

#include "Apps_t.h"
#include "P_super.h"

map<string,Apps_t *> Apps_t::Apps_m;   // Skyhook: Application map
unsigned Apps_t::PoLCnt = 0;           // Of which PoLCnt are proof-of-life
unsigned Apps_t::ExtCnt = 0;           // And the others are externally loaded

//==============================================================================

Apps_t::Apps_t(OrchBase * _p,string _s):par(_p),pSup(0)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
bValid = false;                        // Not validated yet
Apps_m[_s] = this;                     // Store in anchor structure
sTime = string(GetTime());             // Housekeeping
sDate = string(GetDate());
}

//------------------------------------------------------------------------------

Apps_t::~Apps_t()
{
par->Post(802,Name());
WALKVECTOR(GraphI_t *,GraphI_v,i) delete *i;
WALKVECTOR(GraphT_t *,GraphT_v,i) delete *i;
}

//------------------------------------------------------------------------------

void Apps_t::Clear()
{
}

//------------------------------------------------------------------------------

void Apps_t::Dump(FILE * fp)
{
fprintf(fp,"\nApps_t++++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      0x%#08p,0x%#08p\n",this,par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
if (GraphT_v.empty()) fprintf(fp,"No type declare trees\n");
else WALKVECTOR(GraphI_t *,GraphI_v,i) (*i)->Dump(fp);
if (pSup==0) fprintf(fp,"No supervisor device defined\n");
//else fprintf(fp,"Supervisor device %s\n",pSup->FullName().c_str());
else pSup->Dump(fp);
//fprintf(fp,"Linked       %s\n",linked?"yes":"no");
fprintf(fp,"Validated    %s\n",bValid?"yes":"no");
//fprintf(fp,"Owner        %s\n",pOwn==0?"No owner":pOwn->FullName().c_str());
fprintf(fp,"User source file (if any) ||%s||\n",filename.c_str());
fprintf(fp,"PROOF OF LIFE+++++++++++++++++++++++++++++++\n");
if (!PoL.IsPoL) fprintf(fp,"No - User-defined graph\n");
else PoL.Dump(fp);
fprintf(fp,"PROOF OF LIFE-------------------------------\n");
if (GraphI_v.empty()) fprintf(fp,"No device graphs defined\n");
else WALKVECTOR(GraphI_t *,GraphI_v,i) (*i)->Dump(fp);
fprintf(fp,"Apps_t--------------------------------------\n");
fflush(fp);
}

//------------------------------------------------------------------------------

Apps_t * Apps_t::FindApp(string name)
// Search the map.....
{
if (Apps_m.find(name)!=Apps_m.end()) return Apps_m[name];
return 0;
}

//------------------------------------------------------------------------------

GraphI_t * Apps_t::FindGrph(string s)
// Locate the graph instance object held in here with the name "s" (if any)
{
WALKVECTOR(GraphI_t *,GraphI_v,i) if ((*i)->Name()==s) return *i;
return 0;
}

//------------------------------------------------------------------------------

GraphT_t * Apps_t::FindTree(string s)
// Locate the graph type object held in here with the name "s" (if any)
{
WALKVECTOR(GraphT_t *,GraphT_v,i) if ((*i)->Name()==s) return *i;
return 0;
}

//------------------------------------------------------------------------------

bool Apps_t::IsPoL()
{
return PoL.IsPoL;
}

//------------------------------------------------------------------------------

void Apps_t::Show(FILE * fp,unsigned off)
{
string B(off,' ');
fprintf(fp,"%sFile %s\n",B.c_str(),FullName().c_str());

}

//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

Apps_t::PoL_t::PoL_t(): IsPoL(false){}

//------------------------------------------------------------------------------

void Apps_t::PoL_t::Dump(FILE * fp)
{
fprintf(fp,"Apps_t::PoL_t + + + + + + + + + + + + + + + \n");
fprintf(fp,"IsPoL      : %s\n",IsPoL ? "TRUE" : "FALSE");
fprintf(fp,"PoL type   : %s\n",type.c_str());
fprintf(fp,"Parameters :\n");
WALKVECTOR(string,params,i) fprintf(fp,"  %s\n",(*i).c_str());
fprintf(fp,"Apps_t::PoL_t - - - - - - - - - - - - - - - \n");
fflush(fp);
}

//==============================================================================




//------------------------------------------------------------------------------

#include "EdgeI_t.h"
#include "GraphI_t.h"
#include "Meta_t.h"

//==============================================================================

EdgeI_t::EdgeI_t(GraphI_t * G, string name): par(G)
{
Name(name);
Npar(G);
Key = 0;
pPropsI = "";
pStateI = "";
}

//------------------------------------------------------------------------------

EdgeI_t::~EdgeI_t()
{
WALKVECTOR(Meta_t *,Meta_v,i) delete *i;
}

//------------------------------------------------------------------------------

void EdgeI_t::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n%sEdgeI_t ++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sNameBase       %s\n",os,FullName().c_str());
fprintf(fp,"%sMe,Parent      0x%#018lx,0x%#018lx\n",
            os,(uint64_t)this,(uint64_t)par);
if (par!=0) fprintf(fp,"%s...%s\n",os,par->FullName().c_str());
fprintf(fp,"%sP-address :\n",os);
addr.Dump(off+2,fp);
fprintf(fp,"%sGraph key      %u\n",os,Key);
fprintf(fp,"%sMetadata vector has %lu entries:\n",os,Meta_v.size());
WALKVECTOR(Meta_t *,Meta_v,i) (*i)->Dump(off+2,fp);
NameBase::Dump(off+2,fp);
DumpChan::Dump(off+2,fp);
DefRef::Dump(off+2,fp);
fprintf(fp,"%sEdgeI_t ------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void EdgeI_t::EdgDat_cb(EdgeI_t * const & d)
// Debug callback for edge data
{
if (d!=0) fprintf(dfp,"edg(D): %s",d->Name().c_str());
else fprintf(dfp,"edg(D): ***");
fflush(dfp);
}

//------------------------------------------------------------------------------

void EdgeI_t::EdgKey_cb(unsigned const & u)
// Debug callback for edge key
{
fprintf(dfp,"edg(K): %03u",u);
fflush(dfp);
}

//==============================================================================

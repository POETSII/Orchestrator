//------------------------------------------------------------------------------

#include "Meta_t.h"

//==============================================================================

Meta_t::Meta_t()
{
}

//------------------------------------------------------------------------------

Meta_t::Meta_t(const vector<pair<string,string> > & _vps)
// It's a reference - gotta copy it
{
vps = _vps;
}

//------------------------------------------------------------------------------

Meta_t::~Meta_t()
{
}

//------------------------------------------------------------------------------

void Meta_t::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n%sMeta_t +++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sMetadata attribute list length: %lu\n",os,vps.size());
for (unsigned i=0;i<vps.size();i++)
  fprintf(fp,"%s%3u: %s = \"%s\"\n",
             os,i,vps[i].first.c_str(),vps[i].second.c_str());
fprintf(fp,"%sMeta_t -------------------------------------------------\n\n",os);
fflush(fp);
}

//==============================================================================

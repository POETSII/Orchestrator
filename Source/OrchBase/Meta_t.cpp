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

void Meta_t::Dump(FILE * fp)
{
fprintf(fp,"Meta_t+++++++++++++++++++++++++++++++++++++++\n");


fprintf(fp,"Meta_t---------------------------------------\n");
fflush(fp);
}

//==============================================================================



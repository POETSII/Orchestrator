//------------------------------------------------------------------------------

#include "DefRef.h"

//==============================================================================

DefRef::DefRef()
{
def = 0;                               // It's undefined
ref = 0;                               // It's unreferenced
}

//------------------------------------------------------------------------------

DefRef::~DefRef()
{
}

//------------------------------------------------------------------------------

void DefRef::Def(int inc)
// Increment definition counter (may be negative) - clamp def to zero
{
int val = int(def) + inc;
def = (val<0) ? 0 : unsigned(val);
}

//------------------------------------------------------------------------------

void DefRef::Dump(FILE * fp)
{
fprintf(fp,"DefRef++++++++++++\n");
fprintf(fp,"def  : %u\n",def);
fprintf(fp,"ref  : %u\n",ref);
fprintf(fp,"DefRef------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void DefRef::Ref(int inc)
// Increment reference counter (may be negative) - clamp def to zero
{
int val = int(ref) + inc;
ref = (val<0) ? 0 : unsigned(val);
}

//==============================================================================


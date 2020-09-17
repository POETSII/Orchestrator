//------------------------------------------------------------------------------

#include "Constraints.h"

//==============================================================================

Constraints::Constraints() : Constraintm()
{
}

//------------------------------------------------------------------------------

Constraints::~Constraints()
{
}

//------------------------------------------------------------------------------

void Constraints::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n%sConstraints+++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sNameBase       %s\n",os,FullName().c_str());
fprintf(fp,"%sMe             %#018lx\n",os,(uint64_t) this);
fprintf(fp,"%sConstraints---------------------------------\n\n",os);
fflush(fp);
}

//==============================================================================




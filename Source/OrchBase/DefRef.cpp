//------------------------------------------------------------------------------

#include "DefRef.h"
#include "macros.h"
#include <string>
using namespace std;

//==============================================================================

DefRef::DefRef()
{
def = 0;
}

//------------------------------------------------------------------------------

void DefRef::clRef1(unsigned r)
// Remove a reference from the vector
{
WALKVECTOR(unsigned,ref_v,i) {
  if ((*i)!=r) continue;               // Nope
  ref_v.erase(i);                      // Subsequent iterators INVALID
  return;                              // Gone
}
return;                                // Wasn't there. Probably, but not
                                       // necessarily (why not?), a bug.
}

//------------------------------------------------------------------------------

void DefRef::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n%sDefRef++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sDefinition line : %u\n",os,def);
fprintf(fp,"%sReference lines (%lu):\n",os,ref_v.size());
WALKVECTOR(unsigned,ref_v,i) fprintf(fp,"%s%3u",os,(*i));
fprintf(fp,"\n");
fprintf(fp,"%sDefRef--------------------------------------------------\n\n",os);
fflush(fp);
}

//==============================================================================

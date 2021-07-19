#ifndef __DefRefH__H
#define __DefRefH__H
//==============================================================================
/* Base class to handle referencing/definition counting for its derived ....
... sibling? parent? ancestor? derivative? Whatever.
The class retains a vector of source line numbers where the thing - whatever it
is - is referenced, plus the (single) location where it's defined. Thus the
*number* of references is the size of the reference vector.
*/
//==============================================================================

#include <stdio.h>
#include <vector>
using namespace std;

//==============================================================================

class DefRef
{
public:
                   DefRef();
virtual ~          DefRef(){}

unsigned           Def()             { return def;          } // Return def
void               Def(unsigned u)   { def = u;             } // Definition
void               Dump(unsigned=0,FILE * = stdout);     // Developer-facing ...
void               clDef()           { def = 0;             } // Kill def
void               clRef()           { ref_v.clear();       } // Kill refs
void               clRef1(unsigned);                          // Remove 1 ref
vector<unsigned> * pRef()            { return &ref_v;       } // Return refs
unsigned           Ref()             { return ref_v.size(); } // Ref count
void               Ref(unsigned u)   { ref_v.push_back(u);  } // Add ref

private:
vector<unsigned>   ref_v;              // Where I'm referenced
unsigned           def;                // Where I'm defined
};

//==============================================================================

#endif

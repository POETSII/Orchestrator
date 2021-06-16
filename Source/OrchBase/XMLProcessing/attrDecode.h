#ifndef __attrDecodeH__H
#define __attrDecodeH__H

#include <stdio.h>
#include <string>
#include <vector>
using namespace std;

//==============================================================================
/*
This class is a single method functor. The (sole) functionality is to decode the
attribute string appearing in the POETS XML validation file.
The string is (should be) of the form

    [lower..upper](attr_name, ...), [lower,upper](attr_name, ...), ...

"lower" and/or "upper" may be omitted, defaults are 0x0 and 0xffff.
If only a single attr_name is needed, the enclosing parentheses may be omitted.
If a blank attr_name is supplied, this is assumed to refer to the ENCLOSING
element name, NOT an attribute; from the perspective of this functor, this is
irrelevant, the only relevant point is that blank attr_name is allowed.
EXCEPT THAT it must be on its own: structures of the form [],[]A are allowed,
but [](,A) are not.
"lower" and "upper" are constraints, ANDed together, so if you specify U<L, you
get what you ask for: attr_name is not allowed at all.
Duplicate names are permitted (although a bit pointless) - ultimately you get
the pessimistic AND of the constraints specified for each instance of the name.

The output consists of a vector of elaborated quintuples:
{char,string,lower,upper,value}.

char is 'E' or 'A', indicating if the string refers to an attribute or an
element name . As the string doesn't know what the element name might be,
it's left empty if it's an 'E'.

value is used in the Validator tree matching later and can be ignored here;
it is arbitrarily initialised to 0.

The [0] element is always there, and the fields have different meanings:
[0].char  : 'S'
[0].string: copy of the input string
[0].lower : 0 if the string parsed OK, 1 otherwise
[0].upper : 0 if lower==0, otherwise the character position in the string that
caused the error. (The functor bails on the first error; prior valid input may
be in the output vector. If it's there, it's valid.)

Ss is the (optional) *name* of the inner grammar to be applied to the attribute
that is the subject of this struct (i.e. S)
*/
//==============================================================================

class attrDecode
{
public:
                   attrDecode(){}
virtual ~          attrDecode(void){}

struct a6_t {                          // Stencil element
           a6_t(char _C,string _S,unsigned _L,unsigned _U,unsigned _V=0):
                C(_C),S(_S),L(_L),U(_U),V(_V){}
  void     Dump(FILE * fo,unsigned off,const char * s) {
             string bstr(off,' ');
             fprintf(fo,"%s%s%c:[%#8x..%#-8x]%s(%s) -> %#x\n",
                         bstr.c_str(),s,C,L,U,S.c_str(),Ss.c_str(),V);
           }
  bool     OK() { return ((V>=L)&&(V<=U)); }  // Element internally consistent?
  char     C;                          // Type: 'E'|'A'|'S'
  string   S;                          // Element name (except first element)
  unsigned L;                          // Inclusive lower bound
  unsigned U;                          // Inclusive upper bound
  unsigned V;                          // Actual value  L<=V<=U -> OK=true
  string   Ss;                         // Name of attribute value syntax
};

static const unsigned     LOTS = 0xffffffff;
vector<a6_t>       operator()(string); // Functor that does the decode
};

//==============================================================================

#endif

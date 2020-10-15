//------------------------------------------------------------------------------

#include "CFrag.h"
#include "XMLTreeDump.h"

//==============================================================================

CFrag::CFrag()
{
}

//------------------------------------------------------------------------------

CFrag::CFrag(std::string str) : c_src(str)
{
}

//------------------------------------------------------------------------------

CFrag::~CFrag()
{
}

//------------------------------------------------------------------------------

void CFrag::Dump(unsigned off,FILE * fp)
// Not pretty-print at all
{
string s(off,' ');
const char * os = s.c_str();
string _c_src;
const unsigned LIM = 300;
for (unsigned i=0;(i<LIM)&&(i<c_src.size());i++) {
  _c_src += c_src[i];
  if (c_src[i]=='\n') _c_src += (s + "||");
}
if (c_src.size()>LIM) _c_src += ("\n" + s + "|| + more.......\n");

fprintf(fp,"\n%sCFrag+++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sc_src : %lu characters\n%s||%s\n",
            os,c_src.size(),os,_c_src.c_str());
fprintf(fp,"%sCFrag---------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void CFrag::Show(unsigned off,FILE * fp)
// Pretty-print precis
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n%sCFrag+++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sc_src : %lu characters\n%s||%s||\n",
            os,c_src.size(),os,xmlTreeDump::node::Prettify(c_src).c_str());
fprintf(fp,"%sCFrag---------------------------------------------------\n\n",os);
fflush(fp);
}

//==============================================================================

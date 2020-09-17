//---------------------------------------------------------------------------

#include <stdio.h>
#include "Stencil_t.h"
#include "flat.h"

//==============================================================================

Stencil_t::Stencil_t()
// Little more than a holder (vector) for a set of stencil elements.
// A stencil element is a node name (.ename), an argument count (.arg), an
// actual count (.val), and an operator (.op). If (.val) (.op) (.arg) is true,
// the stencil element is valid (OK() returns TRUE), otherwise not
//
{
                                       // Pointer from client stencil
pn = 0;                                // to definition node      
}

//------------------------------------------------------------------------------

Stencil_t::~Stencil_t()
// See comments in XValid source
{
//WALKVECTOR(stel_t *,stel_v,i) delete *i;
}

//------------------------------------------------------------------------------

void Stencil_t::Add(attrDecode::a6_t at)
{
a6_v.push_back(at);
}

//------------------------------------------------------------------------------

void Stencil_t::CopyOf(Stencil_t * ps)
// Copy the contents of the source stencil into the current one
{
if (ps==0) return;                 // Legitimate exit
WALKVECTOR(attrDecode::a6_t,ps->a6_v,i) Add(*i);
}

//------------------------------------------------------------------------------

void Stencil_t::Dump(FILE * fo,unsigned off,const char * s)
{
string bstr(off,' ');
const char * b = bstr.c_str();
fprintf(fo,"\n%s%sStencil nodepointer %" PTR_FMT "\n%s%s(Backpointer -> ",b,s,
        OSFixes::getAddrAsUint(pn),b,s);
if (pn!=0) fprintf(fo,"%s)\n",pn->FullName().c_str());
else fprintf(fo,"0)\n");
fprintf(fo,"%s%sStencil contains %lu elements\n",b,s,a6_v.size());
WALKVECTOR(attrDecode::a6_t,a6_v,i) (*i).Dump(fo,off,s);
fprintf(fo,"%s%s\n\n",b,s);
}

//------------------------------------------------------------------------------

attrDecode::a6_t * Stencil_t::FindStel(string s)
// Locate a stencil element concerning node named s
{
WALKVECTOR(attrDecode::a6_t,a6_v,i) if ((*i).S==s) return &(*i);
return 0;
}

//------------------------------------------------------------------------------

bool Stencil_t::OK()
// See if the stencil is OK: Every element must return good
{
WALKVECTOR(attrDecode::a6_t,a6_v,i) if (!(*i).OK()) return false;
return true;
}

//------------------------------------------------------------------------------

void Stencil_t::Show(unsigned & re,xmlTreeDump::node * me,FILE * fc)
// The pretty-print error output from a stencil
{
WALKVECTOR(attrDecode::a6_t,a6_v,i)
  if (!(*i).OK()) {
    string thing;
    switch ((*i).C) {
      case 'A' : thing = "attribute";      break;
      case 'E' : thing = "element";        break;
      case 'S' : thing = "syntax error";   break;
      default  : thing = "UNRECOVERABLE";  break;
    }
    fprintf(fc,"%3u. (%4u,%3u) : Element %s \n                  "
               "contains %u instance(s) of %s \"%s\": this (%u) should be ",
               ++re,me->lin,me->col,me->FullName().c_str(),(*i).V,
               thing.c_str(),(*i).S.c_str(),(*i).V);
    if ((*i).L==(*i).U) fprintf(fc,"exactly %u\n\n",(*i).L);
    else {
      fprintf(fc,">=%u",(*i).L);
      if ((*i).U!=attrDecode::LOTS-1) fprintf(fc," AND <=%u\n\n",(*i).U);
      else fprintf(fc,"\n\n");
    }
  }
fflush(fc);
}

//------------------------------------------------------------------------------

bool Stencil_t::Update(xmlTreeDump::node * me)
// Update the child count to see if a stencil is valid
{
                                       // Walk the 'me' actual attributes
for(vector<pair<string,string> >::iterator i=me->attr.begin();
    i!=me->attr.end();i++) {
  string aname = (*i).first;           // The attribute name is all we care about
                                       // Stencil of 'me'
  Stencil_t * ps = static_cast<Stencil_t *>(me->tag);
                                       // Stencil element corresponding to aname
  attrDecode::a6_t * pstel = FindStel(aname);
  if (pstel!=0) pstel->V++;            // There; increment counter
                                       // Not there; create '0' element
  else ps->Add(attrDecode::a6_t('A',aname,0,0,1));
}
                                       // Loop through the actual children of me
                                       // to update my stencil
WALKVECTOR(xmlTreeDump::node *,me->vnode,i) {
  string cs = (*i)->ename;             // Got a child name
                                       // Loop stencil elements, incrementing
                                       // actual count as necessary
  WALKVECTOR(attrDecode::a6_t,a6_v,j) if (((*j).S==cs)&&((*j).C=='E'))(*j).V++;
}
return OK();                           // Return OK status
}

//==============================================================================


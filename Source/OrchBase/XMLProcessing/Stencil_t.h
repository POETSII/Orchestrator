#ifndef __Stencil_tH__H
#define __Stencil_tH__H

#include <string>
#include <vector>
using namespace std;
#include "XMLTreeDump.h"
#include "attrDecode.h"
#include "OSFixes.hpp"

//==============================================================================

class Stencil_t
{
public:
                     Stencil_t();
virtual ~            Stencil_t(void);

void                 Add(attrDecode::a6_t);
void                 CopyOf(Stencil_t *);
void                 Dump(FILE *,unsigned,const char *);
attrDecode::a6_t *   FindStel(string);
bool                 OK();
xmlTreeDump::node *& Refpn() { return  pn; }
void                 Show(unsigned &,xmlTreeDump::node *,FILE * =stdout);
bool                 Update(xmlTreeDump::node *);

vector<attrDecode::a6_t> a6_v;
xmlTreeDump::node * pn;                // Backpointer client->definition

};

//==============================================================================

#endif

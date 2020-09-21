#ifndef __CmBuilH__H
#define __CmBuilH__H

//==============================================================================
/* Build command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmBuil
{
public:
              CmBuil(OrchBase *);
virtual ~     CmBuil();

void          Dump(unsigned = 0, FILE * = stdout);
void          Show(FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

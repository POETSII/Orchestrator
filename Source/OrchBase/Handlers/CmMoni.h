#ifndef __CmMoniH__H
#define __CmMoniH__H

//==============================================================================
/* Monitor command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmMoni
{
public:
              CmMoni(OrchBase *);
virtual ~     CmMoni();

void          Cm_Spy();

void          Dump(FILE * = stdout);
void          Show(FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

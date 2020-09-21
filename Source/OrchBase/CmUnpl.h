#ifndef __CmUnplH__H
#define __CmUnplH__H

//==============================================================================
/* Unplace command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmUnpl
{
public:
              CmUnpl(OrchBase *);
virtual ~     CmUnpl();

void          Dump(FILE * = stdout);
void          Show(FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

#ifndef __CmPlacH__H
#define __CmPlacH__H

//==============================================================================
/* Placement command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmPlac
{
public:
              CmPlac(OrchBase *);
virtual ~     CmPlac();

void          Dump(FILE * = stdout);
void          Show(FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

#ifndef __CmInjeH__H
#define __CmInjeH__H

//==============================================================================
/* Injector process command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmInje
{
public:
              CmInje(OrchBase *);
virtual ~     CmInje();

void          Dump(FILE * = stdout);
void          Show(FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

#ifndef __CmUntlH__H
#define __CmUntlH__H

//==============================================================================
/* Untlink command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmUntl
{
public:
              CmUntl(OrchBase *);
virtual ~     CmUntl();

void          Dump(FILE * = stdout);
void          Show(FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

#ifndef __CmNameH__H
#define __CmNameH__H

//==============================================================================
/* Nameserver command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmName
{
public:
              CmName(OrchBase *);
virtual ~     CmName();

void          Dump(FILE * = stdout);
void          Show(FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

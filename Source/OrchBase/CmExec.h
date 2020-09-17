#ifndef __CmExecH__H
#define __CmExecH__H

//==============================================================================
/* Execute OS command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmExec
{
public:
              CmExec(OrchBase *);
virtual ~     CmExec();

void          Dump(FILE * = stdout);
void          Show(FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================
   
#endif

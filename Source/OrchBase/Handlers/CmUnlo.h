#ifndef __CmUnloH__H
#define __CmUnloH__H

//==============================================================================
/* Unload command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmUnlo
{
public:
              CmUnlo(OrchBase *);
virtual ~     CmUnlo();

void          Cm_App(Cli::Cl_t);
void          Dump(FILE * = stdout);
void          Show(FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

#ifndef __CmRunH__H
#define __CmRunH__H

//==============================================================================
/* Run command handler */
//==============================================================================

#include <stdio.h>
#include "Cli.h"
#include "GraphI_t.h"
class OrchBase;

class CmRun
{
public:
              CmRun(OrchBase *);
virtual ~     CmRun();

void          Cm_App(Cli::Cl_t);
void          Dump(unsigned = 0, FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

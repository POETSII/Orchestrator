#ifndef __CmDeplH__H
#define __CmDeplH__H

//==============================================================================
/* Deploy command handler */
//==============================================================================

#include <stdio.h>
#include "Cli.h"
#include "GraphI_t.h"
class OrchBase;

class CmDepl
{
public:
              CmDepl(OrchBase *);
virtual ~     CmDepl();

void          Cm_App(Cli::Cl_t);
int           DeployGraph(GraphI_t *);
void          Dump(unsigned = 0, FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

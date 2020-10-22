#ifndef __CmInitH__H
#define __CmInitH__H

//==============================================================================
/* Initialise command handler */
//==============================================================================

#include <stdio.h>
#include "Cli.h"
#include "GraphI_t.h"
class OrchBase;

class CmInit
{
public:
              CmInit(OrchBase *);
virtual ~     CmInit();

void          Cm_App(Cli::Cl_t);
void          Dump(unsigned = 0, FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

#ifndef __CmStopH__H
#define __CmStopH__H

//==============================================================================
/* Stop command handler */
//==============================================================================

#include <stdio.h>
#include "Cli.h"
#include "GraphI_t.h"
class OrchBase;

class CmStop
{
public:
              CmStop(OrchBase *);
virtual ~     CmStop();

void          Cm_App(Cli::Cl_t);
void          Dump(unsigned = 0, FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

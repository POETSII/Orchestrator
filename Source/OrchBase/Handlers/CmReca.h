#ifndef __CmRecaH__H
#define __CmRecaH__H

//==============================================================================
/* Recall command handler */
//==============================================================================

#include <stdio.h>
#include "Cli.h"
#include "GraphI_t.h"
class OrchBase;

class CmReca
{
public:
              CmReca(OrchBase *);
virtual ~     CmReca();

void          Cm_App(Cli::Cl_t);
void          Dump(unsigned = 0, FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

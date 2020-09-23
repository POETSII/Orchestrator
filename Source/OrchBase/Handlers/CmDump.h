#ifndef __CmDumpH__H
#define __CmDumpH__H

//==============================================================================
/* Dump command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmDump
{
public:
              CmDump(OrchBase *);
virtual ~     CmDump();

void          Cm_Engine(Cli::Cl_t);

void          Dump(unsigned = 0,FILE * = stdout);
void          Show(FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

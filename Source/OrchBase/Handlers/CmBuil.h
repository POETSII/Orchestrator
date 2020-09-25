#ifndef __CmBuilH__H
#define __CmBuilH__H

//==============================================================================
/* Build command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
#include "GraphI_t.h"
class OrchBase;

class CmBuil
{
public:
              CmBuil(OrchBase *);
virtual ~     CmBuil();

void          Cm_Deploy(Cli::Cl_t);
void          Cm_Do(Cli::Cl_t, string);
int           DeployGraph(GraphI_t *);
void          Dump(unsigned = 0, FILE * = stdout);
void          Show(FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

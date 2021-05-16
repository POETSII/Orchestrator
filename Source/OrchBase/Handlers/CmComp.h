#ifndef __CmCompH__H
#define __CmCompH__H

//==============================================================================
/* Compose command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmComp
{
public:
              CmComp(OrchBase *);
virtual ~     CmComp();

void          Cm_App(Cli::Cl_t clause);
void          Cm_Generate(Cli::Cl_t clause);
void          Cm_Compile(Cli::Cl_t clause);
void          Cm_Decompose(Cli::Cl_t clause);
void          Cm_Degenerate(Cli::Cl_t clause);
void          Cm_Clean(Cli::Cl_t clause);
void          Cm_Reset(Cli::Cl_t clause);

void          Dump(unsigned = 0, FILE * = stdout);
void          Show(FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

#ifndef __CmSystH__H
#define __CmSystH__H

//==============================================================================
/* Splitter for system commands
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmSyst
{
public:
              CmSyst(OrchBase *);
virtual ~     CmSyst();

void          Dump(FILE * = stdout);
void          SyInte(Cli::Cl_t);
void          SyPing(Cli::Cl_t);
void          SyRun(Cli::Cl_t);
void          SyShow(Cli::Cl_t);
void          SyTime(Cli::Cl_t);
void          SyTrac(Cli::Cl_t);
unsigned      operator()(Cli *);

OrchBase *    par;
FILE *        fd;
};

//==============================================================================

#endif

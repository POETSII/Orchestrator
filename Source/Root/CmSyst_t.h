#ifndef __CmSyst_tH__H
#define __CmSyst_tH__H

//==============================================================================
/* Splitter for system commands
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class Root;

class CmSyst_t
{
public:
              CmSyst_t(Root *);
virtual ~     CmSyst_t();

void          Dump(FILE * = stdout);

void          Conn(Cli::Cl_t);
void          Ping(Cli::Cl_t);
void          Run(Cli::Cl_t);
void          Show(Cli::Cl_t);
void          Time(Cli::Cl_t);
unsigned      operator()(Cli *);

Root *        par;

};

//==============================================================================
   
#endif

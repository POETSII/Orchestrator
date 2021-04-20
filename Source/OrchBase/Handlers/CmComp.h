#ifndef __CmCompH__H
#define __CmCompH__H

//==============================================================================
/* Compose command handler
*/
//==============================================================================

#include <stdio.h>
#include <algorithm>
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
void          Cm_Bypass(Cli::Cl_t clause);
void          Cm_Decompose(Cli::Cl_t clause);
void          Cm_Degenerate(Cli::Cl_t clause);
void          Cm_Clean(Cli::Cl_t clause);
void          Cm_Reset(Cli::Cl_t clause);
void          Cm_SoftswitchBufferMode(Cli::Cl_t clause, bool mode);
void          Cm_SoftswitchInstrMode(Cli::Cl_t clause, bool mode);
void          Cm_SoftswitchLogHandler(Cli::Cl_t clause);
void          Cm_SoftswitchLogLevel(Cli::Cl_t clause);
void          Cm_SoftswitchSetRTSBuffSize(Cli::Cl_t clause);
void          Cm_SoftswitchAddFlags(Cli::Cl_t clause);

void          Dump(unsigned = 0, FILE * = stdout);
void          Show(FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

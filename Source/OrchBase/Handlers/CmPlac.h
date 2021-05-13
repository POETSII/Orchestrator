#ifndef __CmPlacH__H
#define __CmPlacH__H

//==============================================================================
/* Placement command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmPlac
{
public:
              CmPlac(OrchBase *);
virtual ~     CmPlac();

void          PlacementConstrain(Cli::Cl_t clause);
bool          PlacementDoIt(Cli::Cl_t clause);
void          PlacementDump(Cli::Cl_t clause);
void          PlacementLoad(Cli::Cl_t clause);
bool          PlacementSetArg(Cli::Cl_t clause);
void          PlacementUnplace(Cli::Cl_t clause);

void          Dump(FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

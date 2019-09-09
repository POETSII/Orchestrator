#ifndef __OrchBaseH__H
#define __OrchBaseH__H

class Placement;
class P_file;
class P_builder;
class T_gen;
class P_owner;
class P_super;
class Dialect1Deployer;

#include <stdio.h>
#include "GraphI_t.h"
#include "HardwareModel.h"
#include "HardwareFileParser.h"
#include "Placement.h"
#include "Cli.h"
#include "Environment.h"
#include "CommonBase.h"
#include "Trace.h"
#include "CmPath_t.h"
#include "CmGrph_t.h"
using namespace std;

//==============================================================================

class OrchBase : public CommonBase, public NameBase
{
public:
                       OrchBase(int,char *[],string,string);
virtual ~              OrchBase();
void                   Dump(FILE * = stdout);

P_engine *             pE;             // Poets engine (hardware model)
Placement *            pPlace;         // Cross-linker
T_gen *                pTG;            // PoL task generator
Trace                  Tr;             // Debug trace subsystem
CmGrph_t *             CmGrph;         // Graph command handler
CmPath_t *             CmPath;         // Path command handler
FILE *                 fd;             // Output file stream for details
};

//==============================================================================

#endif

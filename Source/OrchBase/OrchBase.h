#ifndef __OrchBaseH__H
#define __OrchBaseH__H

class Placement;
class P_task;
class P_builder;
class T_gen;
class P_owner;
class P_super;
class Dialect1Deployer;

#include <stdio.h>
#include "GraphI_t.h"
#include "HardwareModel.h"
#include "HardwareFileReader.h"
#include "Placement.h"
#include "Cli.h"
#include "Environment.h"
#include "CommonBase.h"
#include "Trace.h"
#include "CmBuil.h"
#include "CmCall.h"
#include "CmDump.h"
#include "CmExec.h"
#include "CmInje.h"
#include "CmLoad.h"
#include "CmName.h"
#include "CmPath.h"
#include "CmPlac.h"
#include "CmRTCL.h"
#include "CmShow.h"
#include "CmSyst.h"
#include "CmTest.h"
#include "CmTlin.h"
#include "CmUnlo.h"
#include "CmUnpl.h"
#include "CmUntl.h"

#define TASK_DEPLOY_DIR ".orchestrator/task_binaries"  // Relative to home.
using namespace std;

//==============================================================================

class OrchBase : public CommonBase, public NameBase
{
public:
                       OrchBase(int,char *[],string,string);
virtual ~              OrchBase();
void                   Dump(unsigned = 0,FILE * = stdout);

P_engine *             pE;             // Poets engine (hardware model)
Placement *            pPlace;         // Cross-linker
P_builder *            pB;             // Object to build the datastructure
T_gen *                pTG;            // PoL task generator
Trace                  Tr;             // Debug trace subsystem
FILE *                 fd;             // Output file stream for details

CmBuil *               pCmBuil;
CmCall *               pCmCall;
CmDump *               pCmDump;
CmExec *               pCmExec;
//CmExit
CmInje *               pCmInje;
CmLoad *               pCmLoad;
CmName *               pCmName;
CmPath *               pCmPath;
CmPlac *               pCmPlac;
CmRTCL *               pCmRTCL;
CmShow *               pCmShow;
CmSyst *               pCmSyst;
CmTest *               pCmTest;
CmTlin *               pCmTlin;
CmUnlo *               pCmUnlo;
CmUnpl *               pCmUnpl;
CmUntl *               pCmUntl;

// Legacy members retained for a (hopefully) smoother transition
string                 taskpath;       // Absolute file path for task commands
string                 topopath;       // Absolute file path for topo commands
map<string,P_super *>  P_superm;       // Container of supervisor devices
map<string,P_owner *>  P_ownerm;       // Task ownership container

};

//==============================================================================

#endif

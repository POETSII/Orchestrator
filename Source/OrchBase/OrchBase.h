#ifndef __OrchBaseH__H
#define __OrchBaseH__H

class P_builder;
class P_super;

#include <stdio.h>
#include "GraphI_t.h"
#include "Placer.h"
#include "HardwareModel.h"
#include "HardwareFileReader.h"
#include "Cli.h"
#include "Environment.h"
#include "CommonBase.h"
#include "Trace.h"
#include "CmCall.h"
#include "CmDepl.h"
#include "CmDump.h"
#include "CmExec.h"
#include "CmInit.h"
#include "CmInje.h"
#include "CmLoad.h"
#include "CmName.h"
#include "CmPath.h"
#include "CmPlac.h"
#include "CmReca.h"
#include "CmRTCL.h"
#include "CmRun.h"
#include "CmShow.h"
#include "CmStop.h"
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

void                   BuildMshipMap();
void                   ClearTopo();
int                    GetGraphIs(Cli::Cl_t, std::set<GraphI_t*>&,
                                  int skip=-1);
void                   MshipCommand(Cli::Cl_t clause, std::string command);
void                   PlacementReset(bool post=false);

P_engine *             pE;             // Poets engine (hardware model)
Placer *               pPlacer;        // Cross-linker
P_builder *            pB;             // Object to build the datastructure
Trace                  Tr;             // Debug trace subsystem
FILE *                 fd;             // Output file stream for details

// Bimap of boxes to motherships, where the pair holds unique processes by
// their procmap entry. Yes, the name is an attempt at facetiousness.
map2<P_box *, ProcMap::ProcMap_t *> P_SCMm2;

CmCall *               pCmCall;
CmDepl *               pCmDepl;
CmDump *               pCmDump;
CmExec *               pCmExec;
CmInit *               pCmInit;
CmInje *               pCmInje;
CmLoad *               pCmLoad;
CmName *               pCmName;
CmPath *               pCmPath;
CmPlac *               pCmPlac;
CmReca  *              pCmReca;
CmRTCL *               pCmRTCL;
CmRun  *               pCmRun;
CmShow *               pCmShow;
CmStop  *              pCmStop;
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
};

//==============================================================================

#endif

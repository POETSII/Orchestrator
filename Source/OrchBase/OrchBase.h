#ifndef __OrchBaseH__H
#define __OrchBaseH__H

class Composer;
class P_super;

#include <stdio.h>
#include "GraphI_t.h"
#include "Placer.h"
#include "HardwareModel.h"
#include "HardwareFileReader.h"
#include "Cli.h"
#include "Composer.h"
#include "Environment.h"
#include "CommonBase.h"
#include "Trace.h"
#include "SupervisorModes.h"

#include "CmCall.h"
#include "CmComp.h"
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
void                   ComposerReset(bool post=false);
int                    GetGraphIs(Cli::Cl_t, std::set<GraphI_t*>&,
                                  int skip=-1);
void                   MshipCommand(Cli::Cl_t clause, std::string command);
void                   PlacementReset(bool post=false);

P_engine *             pE;             // Poets engine (hardware model)
Placer *               pPlacer;        // Cross-linker
Composer *             pComposer;      // Object to compose binaries
Trace                  Tr;             // Debug trace subsystem
FILE *                 fd;             // Output file stream for details

#if SINGLE_SUPERVISOR_MODE
// There's only one mothership
ProcMap::ProcMap_t * loneMothership;
#else
// Bimap of boxes to motherships, where the pair holds unique processes by
// their procmap entry. Yes, the name is an attempt at facetiousness.
map2<P_box *, ProcMap::ProcMap_t *> P_SCMm2;
#endif
// Holds deployment information (which Motherships are hosting a graph
// instance by name).
map<string,vector<ProcMap::ProcMap_t *> > deplInfo;
// Holds deployment state for an application by name, one of:
//  - DEPLOYING/ED
//  - RECALLING
//  - ERROR
map<string,string> deplStat;
// Holds acknowledgement states from Motherships. Maps appnames to maps of
// ranks to one of:
//  - DEFINED
//  - READY
//  - RUNNING
//  - STOPPED
//  - RECALLED
map<string,map<int,string> > mshipAcks;

CmCall *               pCmCall;
CmComp *               pCmComp;
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
};

//==============================================================================

#endif

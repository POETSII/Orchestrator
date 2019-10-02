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
#include "D_graph.h"
#include "HardwareModel.h"
#include "HardwareFileReader.h"
#include "Placement.h"
#include "Cli.h"
#include "Environment.h"
#include "CommonBase.h"
#include "P_owner.h"
// include "Ns_el.h"
using namespace std;

#define TASK_DEPLOY_DIR ".orchestrator/task_binaries"  // Relative to home.

//==============================================================================

class OrchBase : public CommonBase, public NameBase
{
public:
                       OrchBase(int,char *[],string,string);
virtual ~              OrchBase();
void                   Dump(FILE * = stdout);

// These bodies are in OrchBaseTask.cpp:
void                   ClearDcls();
void                   ClearDcls(string);
void                   ClearTasks();
void                   ClearTasks(char);
void                   ClearTasks(string);
unsigned               CmTask(Cli *);  // Handle task command from monkey
void                   TaskBuild(Cli::Cl_t);
void                   TaskClear(Cli::Cl_t);
void                   TaskDeploy(Cli::Cl_t);
void                   TaskDump(Cli::Cl_t);
void                   TaskGeom(Cli::Cl_t);
void                   TaskLoad(Cli::Cl_t);
void                   TaskPath(Cli::Cl_t);
void                   TaskPol(Cli::Cl_t);
void                   TaskRecall(Cli::Cl_t);
void                   TaskMCmd(Cli::Cl_t,string);
void                   TaskShow(Cli::Cl_t);

// These bodies are in OrchBaseTopo.cpp:
void                   ClearBoxConfig();
void                   ClearBoxConfig(string);
void                   ClearTopo();
unsigned               CmTopo(Cli *);
void                   TopoClea(Cli::Cl_t);
void                   TopoConf(Cli::Cl_t);
void                   TopoCons(Cli::Cl_t);
void                   TopoDump(Cli::Cl_t);
void                   TopoDisc(Cli::Cl_t);
void                   TopoLoad(Cli::Cl_t);
void                   TopoMode(Cli::Cl_t);
void                   TopoPath(Cli::Cl_t);
void                   TopoSave(Cli::Cl_t);
void                   TopoSet1(Cli::Cl_t);
void                   TopoSet2(Cli::Cl_t);

// These bodies are in OrchBaseLink.cpp
unsigned               CmLink(Cli *);
void                   LinkCons(Cli::Cl_t);
void                   LinkDump(Cli::Cl_t);
void                   LinkLink(Cli::Cl_t);
// void                   LinkNser(Cli::Cl_t);
void                   LinkPath(Cli::Cl_t);
void                   LinkPlac(Cli::Cl_t);
void                   LinkUnli(Cli::Cl_t);
void                   UnlinkAll();
void                   Unlink(string);

// These bodies are in OrchBaseOwner.cpp
unsigned               CmOwne(Cli *);
void                   OwneAtta(Cli::Cl_t);
void                   OwneDump(Cli::Cl_t);
void                   OwneShow(Cli::Cl_t);
void                   OwneTask(Cli::Cl_t);

P_engine *             pE;             // Poets engine (hardware model)
Placement *            pPlace;         // Cross-linker
P_builder *            pB;             // Object to build the datastructure
T_gen *                pTG;            // PoL task generator
map<string,P_task *>   P_taskm;        // Holder for multiple task graphs
map<string,P_typdcl *> P_typdclm;      // Holder for ALL task type declarations
string                 taskpath;       // Absolute file path for task commands
string                 topopath;       // Absolute file path for topo commands
map<string,P_super *>  P_superm;       // Container of supervisor devices
map<string,P_owner *>  P_ownerm;       // Task ownership container

};

//==============================================================================

#endif

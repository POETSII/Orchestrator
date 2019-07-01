//------------------------------------------------------------------------------

#include "Pglobals.h"
#include "OrchBase.h"
// #include "P_builder.h"
#include "filename.h"
#include "Constraints.h"
#include "P_core.h"
#include "P_task.h"
#include "P_typdcl.h"
#include "P_devtyp.h"
// #include "CMsg_p.h"
#include "T_gen.h"
#include "Config_t.h"
// #include "Ns_el.h"
#include "P_super.h"


/*
//==============================================================================
// This class is chopped up into multiple .cpp files, because it's so
// bloody boring. The translation unit is *this* file + all its includes.
// The below are NOT translation units in their own right.

#include "OrchBaseTask.cpp"            // Handlers for "task" commands
#include "OrchBaseTopo.cpp"            // Handlers for "topo" commands
#include "OrchBaseLink.cpp"            // Handlers for "link" commands
#include "OrchBaseOwner.cpp"           // Handlers for "owner" commands

//==============================================================================
*/

OrchBase::OrchBase() //(int argc,char * argv[],string d,string s)
{
pP        = 0;
// pB        = new P_builder(argc, argv, this);       // Object to build the datastructure
pTG       = new T_gen(this);           // PoL task generator
pPlace    = new Placement(this);       // Xlink controller
Name("O_");                            // NameBase root name
taskpath  = string(" ");
}

//------------------------------------------------------------------------------

OrchBase::~OrchBase()
{
if (pP!=0)        delete pP;           // Kill the P-node graph
// if (pB!=0)        delete pB;           // Object to build the datastructure
if (pPlace!=0)    delete pPlace;       // Cross link controller
if (pTG!=0)       delete pTG;          // PoL generator

// ADR supervisors do not need to be deleted here as they will be removed
// in the task graphs.
                                       // Supervisors
//WALKMAP(string,P_super *,P_superm,i) delete (*i).second;
                                       // Task map contents
WALKMAP(string,P_task *,P_taskm,i) delete (*i).second;
                                       // Type declare map contents
WALKMAP(string,P_typdcl *,P_typdclm,i) delete (*i).second;
                                       // Owners
WALKMAP(string,P_owner *,P_ownerm,i) delete (*i).second;
}

//------------------------------------------------------------------------------

void OrchBase::Dump(FILE * fp)
{
fprintf(fp,"OrchBase dump+++++++++++++++++++++++++++++++\n");  fflush(fp);
fprintf(fp,"NameBase %s\n",FullName().c_str());
fprintf(fp,"Task path %s\n",taskpath.c_str());
fprintf(fp,"HARDWARE++++++++++++++++++++++++++++++++++++\n");
if (pP==0) fprintf(fp,"No hardware topology loaded\n");
else pP->Dump(fp);
if (pPlace==0) fprintf(fp,"No placement object to be found?\n");
else pPlace->Dump(fp);
fprintf(fp,"HARDWARE------------------------------------\n");
fprintf(fp,"SOFTWARE++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"SUPERVISORS+++++++++++++++++++++++++++++++++\n");
WALKMAP(string,P_super *,P_superm,i) (*i).second->Dump(fp);
fprintf(fp,"SUPERVISORS---------------------------------\n");
fprintf(fp,"TASK MAP++++++++++++++++++++++++++++++++++++\n");
WALKMAP(string,P_task *,P_taskm,i) (*i).second->Dump(fp);
fprintf(fp,"TASK MAP------------------------------------\n");
fprintf(fp,"TYPE DECLARES+++++++++++++++++++++++++++++++\n");
WALKMAP(string,P_typdcl *,P_typdclm,i) (*i).second->Dump(fp);
fprintf(fp,"TYPE DECLARES-------------------------------\n");
fprintf(fp,"SOFTWARE------------------------------------\n");
NameBase::Dump(fp);
fprintf(fp,"OrchBase dump-------------------------------\n");  fflush(fp);
}

//------------------------------------------------------------------------------

void OrchBase::UnlinkAll()
// As the name implies; we don't need to worry about tidying anything up,
// 'cos it's all got to go. No side effects or stuff left out here - on exit,
// the device <-> thread links are all deleted. Not ethe placement moduke needs
// not know - the topology is left untouched.
{
if (pP==0) return;                     // No node graph
                                       // Kill the thread->device links
WALKPDIGRAPHNODES(unsigned,P_box *,unsigned,P_link *,unsigned,P_port *,pP->G,i)
  WALKVECTOR(P_board *,pP->G.NodeData(i)->P_boardv,j)
    WALKVECTOR(P_core *,(*j)->P_corev,k)
      WALKVECTOR(P_thread *,(*k)->P_threadv,l) (*l)->P_devicel.clear();

if (P_taskm.empty()) return;           // No tasks at all
WALKMAP(string,P_task *,P_taskm,i) {   // Walk them that's there
  P_task * pTa = (*i).second;          // Task
  if (!pTa->linked) continue;          // Not linked anyway - skip it
  pTa->linked = false;                 // Not linked now
  D_graph * pDg = pTa->pD;             // Device graph
  WALKPDIGRAPHNODES(unsigned,P_device *,unsigned,P_message *,
                    unsigned,P_pin *,pDg->G,j)
    pDg->G.NodeData(j)->pP_thread=0;   // Kill link into threads
}

}

//==============================================================================




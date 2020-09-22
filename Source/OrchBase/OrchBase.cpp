//------------------------------------------------------------------------------

#include "Pglobals.h"
#include "OrchBase.h"
#include "P_builder.h"
#include "FileName.h"
#include "P_core.h"
#include "Ns_el.h"
#include "P_super.h"

//==============================================================================
// This class is chopped up into multiple .cpp files, because it's so
// bloody boring. The translation unit is *this* file + all its includes.
// The below are NOT translation units in their own right.

/* Not anymore!
#include "OrchBaseTask.cpp"            // Handlers for "task" commands
#include "OrchBaseTopo.cpp"            // Handlers for "topo" commands
#include "OrchBaseOwner.cpp"           // Handlers for "owner" commands
#include "OrchBasePlace.cpp"           // Guess
*/

//==============================================================================

OrchBase::OrchBase(int argc,char * argv[],string d,string sfile) :
  CommonBase(argc,argv,d,sfile)
{
pE        = 0;
pPlacer   = 0;
pB        = new P_builder(argc, argv, this);       // Object to build the datastructure
Name("O_");                            // NameBase root name
taskpath  = string(" ");
}

//------------------------------------------------------------------------------

OrchBase::~OrchBase()
{
if (pE!=0)        delete pE;           // Destroy the engine
if (pPlacer!=0)   delete pPlacer;      // Destroy the placer
if (pB!=0)        delete pB;           // Object to build the datastructure
}

//------------------------------------------------------------------------------

void OrchBase::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sOrchBase dump+++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sNameBase %s\n",os,FullName().c_str());
fprintf(fp,"%sTask path %s\n",os,taskpath.c_str());
fprintf(fp,"%sHARDWARE++++++++++++++++++++++++++++++++++++\n",os);
if (pE==0) fprintf(fp,"%sNo hardware topology loaded\n",os);
else pE->Dump(fp);
fprintf(fp,"%sHARDWARE------------------------------------\n",os);
NameBase::Dump(off,fp);
fprintf(fp,"%sOrchBase dump-------------------------------\n",os);
fflush(fp);
}

//==============================================================================

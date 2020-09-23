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
pB        = new P_builder(this);       // Object to build the datastructure
Name("O_");                            // NameBase root name

// Command handlers
pCmBuil = new CmBuil(this);
pCmCall = new CmCall(this);
pCmDump = new CmDump(this);
pCmExec = new CmExec(this);
pCmInje = new CmInje(this);
pCmLoad = new CmLoad(this);
pCmName = new CmName(this);
pCmPath = new CmPath(this);
pCmPlac = new CmPlac(this);
pCmRTCL = new CmRTCL(this);
pCmShow = new CmShow(this);
pCmSyst = new CmSyst(this);
pCmTest = new CmTest(this);
pCmTlin = new CmTlin(this);
pCmUnlo = new CmUnlo(this);
pCmUnpl = new CmUnpl(this);
pCmUntl = new CmUntl(this);

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

/* Constructs the bimap of mothership processes to boxes in the engine
 * (OrchBase.P_SCMm2). */
void OrchBase::BuildMshipMap()
{
    std::vector<ProcMap::ProcMap_t>::iterator procIt;
    std::map<AddressComponent, P_box*>::iterator boxIt;
    bool foundAMothershipForThisBox;

    /* Start from the first process. */
    procIt = pPmap->vPmap.begin();

    /* Iterate over each box in the hardware model. */
    for (boxIt = pE->P_boxm.begin(); boxIt != pE->P_boxm.end(); boxIt++)
    {
        /* Find the next available Mothership. We need the rank in order to
         * store entries in the 'mothershipPayloads' map. */
        foundAMothershipForThisBox = false;

        /* Find the next available Mothership. */
        while (procIt != pPmap->vPmap.end() and
               procIt->P_class != csMOTHERSHIPproc) procIt++;

        /* If we found one, store it. */
        if (procIt != pPmap->vPmap.end())
        {
            P_SCMm2.Add(boxIt->second, &*procIt);
            foundAMothershipForThisBox = true;
            procIt++;
        }

        /* If we didn't find a Mothership for this box, map the box to PNULL
         * and warn loudly. */
        if (!foundAMothershipForThisBox)
        {
            P_SCMm2.Add(boxIt->second, PNULL);
            Post(168, boxIt->second->Name().c_str());
        }
    }
}

//------------------------------------------------------------------------------

void OrchBase::ClearTopo()
{
    if (pE == 0) return;
    if (pE->FullName().empty())
    {
        Post(134, "with no name");
    }
    else
    {
        Post(134, pE->FullName());
    }
    delete pE;
    pE = 0;
    PlacementReset();
}

//------------------------------------------------------------------------------

void OrchBase::PlacementReset(bool post)
{
    if (pPlacer == 0) delete pPlacer;
    if (pE != 0) pPlacer = new Placer(pE);
    if (post) Post(308);
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

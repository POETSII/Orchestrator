//------------------------------------------------------------------------------

#include "Pglobals.h"
#include "OrchBase.h"
#include "FileName.h"
#include "P_core.h"
#include "Ns_el.h"
#include "P_super.h"

//==============================================================================

OrchBase::OrchBase(int argc,char * argv[],string d,string sfile) :
  CommonBase(argc,argv,d,sfile)
{
pE        = PNULL;
pPlacer   = PNULL;
pComposer = PNULL;
Name("O_");                            // NameBase root name

// Command handlers
pCmBuil = new CmBuil(this);
pCmCall = new CmCall(this);
pCmComp = new CmComp(this);
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
if (pE != PNULL) delete pE;               // Destroy the engine
if (pPlacer != PNULL) delete pPlacer;     // Destroy the placer
if (pComposer != PNULL) delete pComposer; // Destroy the composer

// Command handlers
if (pCmBuil != PNULL) delete pCmBuil;
if (pCmCall != PNULL) delete pCmCall;
if (pCmComp != PNULL) delete pCmComp;
if (pCmDump != PNULL) delete pCmDump;
if (pCmExec != PNULL) delete pCmExec;
if (pCmInje != PNULL) delete pCmInje;
if (pCmLoad != PNULL) delete pCmLoad;
if (pCmName != PNULL) delete pCmName;
if (pCmPath != PNULL) delete pCmPath;
if (pCmPlac != PNULL) delete pCmPlac;
if (pCmRTCL != PNULL) delete pCmRTCL;
if (pCmShow != PNULL) delete pCmShow;
if (pCmSyst != PNULL) delete pCmSyst;
if (pCmTest != PNULL) delete pCmTest;
if (pCmTlin != PNULL) delete pCmTlin;
if (pCmUnlo != PNULL) delete pCmUnlo;
if (pCmUnpl != PNULL) delete pCmUnpl;
if (pCmUntl != PNULL) delete pCmUntl;

// Applications
Apps_t::DelAll();
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
    if (pE == PNULL) return;
    if (pE->FullName().empty())
    {
        Post(134, "with no name");
    }
    else
    {
        Post(134, pE->FullName());
    }
    delete pE;
    pE = PNULL;
    PlacementReset();
}

//------------------------------------------------------------------------------

void OrchBase::ComposerReset(bool post)
{
    if (pComposer != PNULL)
    {
        delete pComposer;
        pComposer = PNULL;
    }
    if (pPlacer != PNULL)
    {
        pComposer = new Composer(pPlacer);
        pComposer->setOutputPath(pCmPath->pathBina);
    }
    if (post) Post(800);
}

//------------------------------------------------------------------------------

/* Collects all of the graph instances requested from a "Cli" command.
 *
 * This method iterates through all parameters in a command. Three forms of
 * parameters are considered:
 *
 * 1. "*": Fetch all graph instances from all tasks.
 *
 * 2. "<ALPHANUMERIC_APPNAME>": Fetch all graph instances associated with the
 *    application with name `<ALPHANUMERIC_APPNAME>`.
 *
 * 3. "<ALPHANUMERIC_APPNAME>::<ALPHANUMERIC_GRAPHINAME>": Fetches the graph
 *    instance named `<ALPHANUMERIC_GRAPHINAME>` that is owned by the
 *    application named `<ALPHANUMERIC_APPNAME>`.
 *
 * This method "errors" if:
 *
 * - Syntax (1) is used, and there are no graph instances.
 *
 * - Syntax (2) is used, and there is no application with that name.
 *
 * - Syntax (2) is used and the application exists, but that application has no
 *   graph instances.
 *
 * - Syntax (3) is used and the specified graph instance does not exist.
 *
 * - No parameters are passed in.
 *
 * - A parameter has a double-or-more "::" syntax (e.g. A::B::C).
 *
 * On error, this method fails fast - it posts, clears `graphs`, and bails.
 *
 * Operators are ignored (as per the documentation) when finding applications
 * and graphs in this way.
 *
 * Optionally, the `skip`th parameter in the clause may be skipped.
 *
 * Returns 0 if all is well, and 1 on error. */
int OrchBase::GetGraphIs(Cli::Cl_t clause, std::set<GraphI_t*>& graphs,
                         int skip)
{
    graphs.clear();

    /* Error if no parameters are passed in. */
    if (clause.Pa_v.empty())
    {
        Post(171, clause.Cl);
        return 1;
    }

    /* We'll be needing some iterators... */
    int paramIndex = 0;
    std::vector<Cli::Pa_t>::iterator paramIt;
    std::map<std::string, Apps_t*>::iterator appIt;
    std::vector<GraphI_t*>::iterator graphIt;

    /* Iterate over all parameters. */
    for (paramIt = clause.Pa_v.begin(); paramIt != clause.Pa_v.end();
         paramIt++)
    {
        /* Skip condition. */
        if (paramIndex == skip) continue;
        else paramIndex++;

        /* Is this even possible? */
        if (paramIt->Va_v.empty()) continue;

        /* Syntax (1). */
        if (paramIt->Va_v.size() == 1 and paramIt->Va_v[0] == "*")
        {
            /* For each application... */
            for (appIt = Apps_t::Apps_m.begin();
                 appIt != Apps_t::Apps_m.end(); appIt++)
            {
                /* For each graph instance... */
                for (graphIt = appIt->second->GraphI_v.begin();
                     graphIt != appIt->second->GraphI_v.end(); graphIt++)
                {
                    /* Store */
                    graphs.insert(*graphIt);
                }
            }

            /* No graph instances? It means we haven't added any. Error out. */
            if (graphs.empty())
            {
                Post(172);
                return 1;
            }
        }

        /* Syntax (2/3). */
        else if (paramIt->Va_v.size() == 1 or paramIt->Va_v.size() == 2)
        {
            /* Find the application. */
            Apps_t* application = Apps_t::FindApp(paramIt->Va_v[0]);

            /* Couldn't find it? Error out. */
            if (application == 0)
            {
                graphs.clear();
                Post(173, paramIt->Va_v[0]);
                return 1;
            }

            /* Found it, but it has no graph instances? Error out. */
            if (application->GraphI_v.empty())
            {
                graphs.clear();
                Post(174, paramIt->Va_v[0]);
                return 1;
            }

            /* Syntax (2). */
            if (paramIt->Va_v.size() == 1)
            {
                /* For each graph instance... */
                for (graphIt = appIt->second->GraphI_v.begin();
                     graphIt != appIt->second->GraphI_v.end(); graphIt++)
                {
                    /* Store */
                    graphs.insert(*graphIt);
                }
            }

            /* Syntax (3). */
            else
            {
                /* Find the graph instance. */
                GraphI_t* graph = application->FindGrph(paramIt->Va_v[1]);

                /* Couldn't find it? Error out. */
                if (graph == 0)
                {
                    graphs.clear();
                    Post(175, paramIt->Va_v[0], paramIt->Va_v[1]);
                    return 1;
                }

                /* Otherwise, store. */
                graphs.insert(graph);
            }
        }

        /* Strange syntax. */
        else
        {
            graphs.clear();
            Post(176, paramIt->Concatenate());
            return 1;
        }
    }

    /* All is well. */
    return 0;
}

//------------------------------------------------------------------------------

void OrchBase::PlacementReset(bool post)
{
    if (pComposer != PNULL)
    {
        delete pComposer;
        pComposer = PNULL;
    }
    if (pPlacer != PNULL)
    {
        delete pPlacer;
        pPlacer = PNULL;
    }
    if (pE != PNULL)
    {
        pPlacer = new Placer(pE);
        pPlacer->outFilePath = pCmPath->pathPlac;
    }
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

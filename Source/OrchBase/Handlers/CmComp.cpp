//------------------------------------------------------------------------------

#include "CmComp.h"
#include "OrchBase.h"

//==============================================================================

CmComp::CmComp(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmComp::~CmComp()
{
}

//------------------------------------------------------------------------------

void CmComp::Cm_App(Cli::Cl_t clause)
{
    /* Shout if no hardware model is loaded (i.e. there is no placer) */
    if (par->pPlacer == PNULL)
    {
        par->Post(805, "compose");
        return;
    }

    /* Grab the graph instances of interest. */
    std::set<GraphI_t*> graphs;
    if (par->GetGraphIs(clause, graphs) == 1) return;

    /* Check they're all placed before proceeding. */
    std::set<GraphI_t*>::iterator graphIt;
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        if (par->pPlacer->placedGraphs.find(*graphIt) ==
            par->pPlacer->placedGraphs.end())
        {
            par->Post(802, (*graphIt)->Name(), "composing");
            return;
        }
    }

    /* Compose each app in sequence, failing fast. */
#warning "CmComp::Cm_App: No logic defined for compose failure (also assumes that a non-zero exit code is the only indication of an error, and doesn't post)."
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        par->Post(803, "Compos", (*graphIt)->Name());
        if (par->pComposer->compose(*graphIt) != 0) return;
        par->Post(804, (*graphIt)->Name(), "composed");
    }
}

//------------------------------------------------------------------------------

void CmComp::Cm_Generate(Cli::Cl_t clause)
{
    /* Shout if no hardware model is loaded (i.e. there is no placer) */
    if (par->pPlacer == PNULL)
    {
        par->Post(805, "generate");
        return;
    }

    /* Grab the graph instances of interest. */
    std::set<GraphI_t*> graphs;
    if (par->GetGraphIs(clause, graphs) == 1) return;

    /* Check they're all placed before proceeding. */
    std::set<GraphI_t*>::iterator graphIt;
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        if (par->pPlacer->placedGraphs.find(*graphIt) ==
            par->pPlacer->placedGraphs.end())
        {
            par->Post(802, (*graphIt)->Name(), "generating source");
            return;
        }
    }

    /* Generate source for each app in sequence, failing fast. */
#warning "CmComp::Cm_Generate: No logic defined for generate failure (also assumes that a non-zero exit code is the only indication of an error, and doesn't post)."
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        par->Post(803, "Generat", (*graphIt)->Name());
        if (par->pComposer->generate(*graphIt) != 0) return;
        par->Post(804, (*graphIt)->Name(), "generated");
    }
}

//------------------------------------------------------------------------------

void CmComp::Cm_Compile(Cli::Cl_t clause)
{
    /* Shout if no hardware model is loaded (i.e. there is no placer) */
    if (par->pPlacer == PNULL)
    {
        par->Post(805, "compile");
        return;
    }

    /* Grab the graph instances of interest. */
    std::set<GraphI_t*> graphs;
    if (par->GetGraphIs(clause, graphs) == 1) return;

    /* Check they're all generated before proceeding. */
#warning "CmComp::Cm_Compile: No logic defined for checking whether an application's source has been generated"
    std::set<GraphI_t*>::iterator graphIt;
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        if (par->pPlacer->placedGraphs.find(*graphIt) ==
            par->pPlacer->placedGraphs.end())
        {
            par->Post(802, (*graphIt)->Name(), "compiling source");
            return;
        }
    }

    /* Generate source for each app in sequence, failing fast. */
#warning "CmComp::Cm_Compile: No logic defined for source compiling failure (also assumes that a non-zero exit code is the only indication of an error, and doesn't post)."
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        par->Post(803, "Compil", (*graphIt)->Name());
        if (par->pComposer->compile(*graphIt) != 0) return;
        par->Post(804, (*graphIt)->Name(), "compiled");
    }
}

//------------------------------------------------------------------------------

void CmComp::Cm_Decompose(Cli::Cl_t clause)
{
    /* Shout if no hardware model is loaded (i.e. there is no placer) */
    if (par->pPlacer == PNULL)
    {
        par->Post(805, "decompose");
        return;
    }

    /* Grab the graph instances of interest. */
    std::set<GraphI_t*> graphs;
    if (par->GetGraphIs(clause, graphs) == 1) return;

    /* Check they're all placed before proceeding. */
    std::set<GraphI_t*>::iterator graphIt;
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        if (par->pPlacer->placedGraphs.find(*graphIt) ==
            par->pPlacer->placedGraphs.end())
        {
            par->Post(802, (*graphIt)->Name(), "decomposing");
            return;
        }
    }

    /* Degenerate each app in sequence. */
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        par->pComposer->decompose(*graphIt);
    }
}

//------------------------------------------------------------------------------

void CmComp::Cm_Degenerate(Cli::Cl_t clause)
{
    /* Shout if no hardware model is loaded (i.e. there is no placer) */
    if (par->pPlacer == PNULL)
    {
        par->Post(805, "degenerate");
        return;
    }

    /* Grab the graph instances of interest. */
    std::set<GraphI_t*> graphs;
    if (par->GetGraphIs(clause, graphs) == 1) return;

    /* Check they're all placed before proceeding. */
    std::set<GraphI_t*>::iterator graphIt;
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        if (par->pPlacer->placedGraphs.find(*graphIt) ==
            par->pPlacer->placedGraphs.end())
        {
            par->Post(802, (*graphIt)->Name(), "degenerating");
            return;
        }
    }

    /* Degenerate each app in sequence. */
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        par->pComposer->degenerate(*graphIt);
    }
}

//------------------------------------------------------------------------------

void CmComp::Cm_Clean(Cli::Cl_t clause)
{
    /* Shout if no hardware model is loaded (i.e. there is no placer) */
    if (par->pPlacer == PNULL)
    {
        par->Post(805, "clean");
        return;
    }

    /* Grab the graph instances of interest. */
    std::set<GraphI_t*> graphs;
    if (par->GetGraphIs(clause, graphs) == 1) return;

    /* Check they're all placed before proceeding. */
    std::set<GraphI_t*>::iterator graphIt;
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        if (par->pPlacer->placedGraphs.find(*graphIt) ==
            par->pPlacer->placedGraphs.end())
        {
            par->Post(802, (*graphIt)->Name(), "cleaning");
            return;
        }
    }

    /* Clean each app in sequence. */
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        par->pComposer->clean(*graphIt);
    }
}

//------------------------------------------------------------------------------

void CmComp::Cm_Reset(Cli::Cl_t clause)
{
    /* NB: If the operator passes an argument this command (e.g. 'composer
     * /reset = APPLICATION'), we warn the operator that they're doing
     * something a bit silly. */
    if (!clause.Pa_v.empty()) par->Post(801);
    else par->ComposerReset(true);
}

//------------------------------------------------------------------------------

void CmComp::Dump(unsigned off,FILE * fp)
{
#warning "CmComp::Dump: Not fully defined or connected to anything"
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sCmComp +++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
if (par==0) fprintf(fp,"%sOrchBase parent not defined\n",os);
else fprintf(fp,"%sOrchbase parent : %s\n",os,par->FullName().c_str());
fprintf(fp,"%sCmComp -------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void CmComp::Show(FILE * fp)
{
#warning "CmComp::Show: Not fully defined"
fprintf(fp,"\nComposer attributes and state:\n");
fprintf(fp,"\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmComp::operator()(Cli * pC)
// Handle "comp(ose)" command from the monkey.
{
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause string
  string sCo = pC->Co;                 // Pull out command string
  string sPa = (*i).GetP();            // Pull out (simple) parameter
  if (sCl=="app"  ) { Cm_App(*i);           continue; }
  if (sCl=="gene" ) { Cm_Generate(*i);      continue; }
  if (sCl=="comp" ) { Cm_Compile(*i);       continue; }
  if (sCl=="deco" ) { Cm_Decompose(*i);     continue; }
  if (sCl=="dege" ) { Cm_Degenerate(*i);    continue; }
  if (sCl=="clea" ) { Cm_Clean(*i);         continue; }
  if (sCl=="rese" ) { Cm_Reset(*i);         continue; }
  par->Post(25,sCl,"compose");         // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================

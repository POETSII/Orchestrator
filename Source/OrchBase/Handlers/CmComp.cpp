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
            par->Post(802, (*graphIt)->Name(), "placed", "composing");
            return;
        }
    }

    /* Compose each app in sequence, failing fast. */
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        par->Post(803, "Compos", (*graphIt)->Name());
        if (par->pComposer->compose(*graphIt) != 0)
        {
            par->Post(806, (*graphIt)->Name(), "compose");
            return;
        }
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
            par->Post(802, (*graphIt)->Name(), "placed", "generating source");
            return;
        }
    }

    /* Generate source for each app in sequence, failing fast. */
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        par->Post(803, "Generat", (*graphIt)->Name());
        if (par->pComposer->generate(*graphIt) != 0)
        {
            par->Post(806, (*graphIt)->Name(), "generate");
            return;
        }
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

    /* Check they're all placed and generated before proceeding. */
    std::set<GraphI_t*>::iterator graphIt;
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        if (par->pPlacer->placedGraphs.find(*graphIt) ==
            par->pPlacer->placedGraphs.end())
        {
            par->Post(802, (*graphIt)->Name(), "placed", "compiling source");
            return;
        }

        if(!par->pComposer->isGenerated(*graphIt))
        {
            par->Post(802, (*graphIt)->Name(), "generated", "compiling source");
            return;
        }
    }

    /* Generate source for each app in sequence, failing fast. */
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        par->Post(803, "Compil", (*graphIt)->Name());
        if (par->pComposer->compile(*graphIt) != 0)
        {
            par->Post(806, (*graphIt)->Name(), "compile");
            return;
        }
        par->Post(804, (*graphIt)->Name(), "compiled");
    }
}

//------------------------------------------------------------------------------

void CmComp::Cm_Bypass(Cli::Cl_t clause)
{
    /* Shout if no hardware model is loaded (i.e. there is no placer) */
    if (par->pPlacer == PNULL)
    {
        par->Post(805, "bypass");
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
            par->Post(802, (*graphIt)->Name(), "placed", "composing (bypass)");
            return;
        }
    }

    /* Compose each app in sequence, failing fast. */
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        par->Post(803, "Bypass", (*graphIt)->Name());
        if (par->pComposer->bypass(*graphIt) != 0)
        {
            par->Post(806, (*graphIt)->Name(), "bypass");
            return;
        }
        par->Post(804, (*graphIt)->Name(), "bypassed");
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
            par->Post(802, (*graphIt)->Name(), "placed", "decomposing");
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
            par->Post(802, (*graphIt)->Name(), "placed", "degenerating");
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
            par->Post(802, (*graphIt)->Name(), "placed", "cleaning");
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

void CmComp::Cm_SoftswitchBufferMode(Cli::Cl_t clause, bool mode = false)
{
    /* Shout if no hardware model is loaded (i.e. there is no placer) */
    if (par->pPlacer == PNULL)
    {
        par->Post(805, "buffer");
        return;
    }

    /* Grab the graph instances of interest. */
    std::set<GraphI_t*> graphs;
    if (par->GetGraphIs(clause, graphs) == 1) return;

    /* Set the softswitch buffering mode for each app in sequence. */
    std::set<GraphI_t*>::iterator graphIt;
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        par->pComposer->setBuffMode(*graphIt, mode);
    }
}

//------------------------------------------------------------------------------

void CmComp::Cm_SoftswitchReqIdleMode(Cli::Cl_t clause, bool mode = true)
{
    /* Shout if no hardware model is loaded (i.e. there is no placer) */
    if (par->pPlacer == PNULL)
    {
        par->Post(805, "requestIdle");
        return;
    }

    /* Grab the graph instances of interest. */
    std::set<GraphI_t*> graphs;
    if (par->GetGraphIs(clause, graphs) == 1) return;

    /* Set the softswitch requestIdle mode for each app in sequence. */
    std::set<GraphI_t*>::iterator graphIt;
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        par->pComposer->setReqIdleMode(*graphIt, mode);
    }
}

//------------------------------------------------------------------------------

void CmComp::Cm_SoftswitchInstrMode(Cli::Cl_t clause, bool mode = false)
{
    /* Shout if no hardware model is loaded (i.e. there is no placer) */
    if (par->pPlacer == PNULL)
    {
        par->Post(805, "nobuffer");
        return;
    }

    /* Grab the graph instances of interest. */
    std::set<GraphI_t*> graphs;
    if (par->GetGraphIs(clause, graphs) == 1) return;

    /* Set the instrumentation mode for each app in sequence. */
    std::set<GraphI_t*>::iterator graphIt;
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        par->pComposer->enableInstr(*graphIt, mode);
    }
}

//------------------------------------------------------------------------------

void CmComp::Cm_SoftswitchLogHandler(Cli::Cl_t clause)
{
    /* Shout if no hardware model is loaded (i.e. there is no placer) */
    if (par->pPlacer == PNULL)
    {
        par->Post(805, "loghandler");
        return;
    }

    /* Grab the graph instances of interest. */
    std::set<GraphI_t*> graphs;
    if (par->GetGraphIs(clause, graphs,1) == 1) return;

    /* Set the mode for each app in sequence. */
    std::set<GraphI_t*>::iterator graphIt;
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        if(clause.Pa_v.size() != 2)
        {   // Missing the log handler specification
            par->Post(807, "/LOGH", (*graphIt)->Name());
            return;
        }

        // Grab the handler type and convert to lower
        std::string hName = clause.Pa_v[1].Va_v[0]; //we ignore stuff after ::
        std::transform(hName.begin(), hName.end(), hName.begin(), ::tolower);

        if(hName == "none")
        {   // Disable the log handler
            par->pComposer->setLogHandler(*graphIt, disabled);
        }
        else if(hName == "trivial")
        {   // Set the trivial log handler
            par->pComposer->setLogHandler(*graphIt, trivial);
        }
        else
        {   // Unknown log handler
            //TODO: Barf
        }
    }
}

//------------------------------------------------------------------------------

void CmComp::Cm_SoftswitchLogLevel(Cli::Cl_t clause)
{
    /* Shout if no hardware model is loaded (i.e. there is no placer) */
    if (par->pPlacer == PNULL)
    {
        par->Post(805, "loghandler");
        return;
    }

    /* Grab the graph instances of interest. */
    std::set<GraphI_t*> graphs;
    if (par->GetGraphIs(clause, graphs,1) == 1) return;

    /* Set the level for each app in sequence. */
    std::set<GraphI_t*>::iterator graphIt;
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        if(clause.Pa_v.size() != 2)
        {   // Missing the log level specification
            par->Post(807, "/LOGL", (*graphIt)->Name());
            return;
        }

        // Grab the specified log level, we ignore stuff after ::
        unsigned long level = atol(clause.Pa_v[1].Va_v[0].c_str());

        par->pComposer->setLogLevel(*graphIt, level);
    }
}

//------------------------------------------------------------------------------

void CmComp::Cm_SoftswitchSetRTSBuffSize(Cli::Cl_t clause)
{
    /* Shout if no hardware model is loaded (i.e. there is no placer) */
    if (par->pPlacer == PNULL)
    {
        par->Post(805, "rtsbuffsize");
        return;
    }

    /* Grab the graph instances of interest. */
    std::set<GraphI_t*> graphs;
    if (par->GetGraphIs(clause, graphs,1) == 1) return;

    /* Set the mode for each app in sequence. */
    std::set<GraphI_t*>::iterator graphIt;
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        if(clause.Pa_v.size() != 2)
        {   // Missing the RTS Buff size
            par->Post(807, "/RTSB", (*graphIt)->Name());
            return;
        }

        // Grab the specified buffer size, we ignore stuff after ::
        unsigned long buffSz = atol(clause.Pa_v[1].Va_v[0].c_str());

        par->pComposer->setRTSSize(*graphIt, buffSz);
    }
}


void CmComp::Cm_SoftswitchAddFlags(Cli::Cl_t clause)
{
    /* Shout if no hardware model is loaded (i.e. there is no placer) */
    if (par->pPlacer == PNULL)
    {
        par->Post(805, "rtsbuffsize");
        return;
    }

    /* Grab the graph instances of interest. */
    std::set<GraphI_t*> graphs;
    if (par->GetGraphIs(clause, graphs,1) == 1) return;

    /* Set the flags for each app in sequence. */
    std::set<GraphI_t*>::iterator graphIt;
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        if(clause.Pa_v.size() != 2)
        {   // Missing the flags
            par->Post(807, "/ARGS", (*graphIt)->Name());
            return;
        }

        // Grab the specified flags and populate, we ignore stuff after ::
        par->pComposer->addFlags(*graphIt, clause.Pa_v[1].Va_v[0]);
    }
}
//------------------------------------------------------------------------------

void CmComp::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sCmComp +++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
if (par==0) fprintf(fp,"%sOrchBase parent not defined\n",os);
else fprintf(fp,"%sOrchbase parent : %s\n",os,par->FullName().c_str());
par->pComposer->Dump(off+2,fp);
fprintf(fp,"%sCmComp -------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void CmComp::Show(FILE * fp)
{
fprintf(fp,"\nComposer attributes and state:\n");
par->pComposer->Show(fp);
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
  if (sCl=="bypa" ) { Cm_Bypass(*i);       continue; }
  if (sCl=="deco" ) { Cm_Decompose(*i);     continue; }
  if (sCl=="dege" ) { Cm_Degenerate(*i);    continue; }
  if (sCl=="clea" ) { Cm_Clean(*i);         continue; }
  if (sCl=="rese" ) { Cm_Reset(*i);         continue; }

  // Softswitch control commands
  if (sCl=="buff" ) { Cm_SoftswitchBufferMode(*i, true);   continue; }
  if (sCl=="nobu" ) { Cm_SoftswitchBufferMode(*i, false);  continue; }
  if (sCl=="logh" ) { Cm_SoftswitchLogHandler(*i);         continue; }
  if (sCl=="logl" ) { Cm_SoftswitchLogLevel(*i);           continue; }
  if (sCl=="inst" ) { Cm_SoftswitchInstrMode(*i, true);    continue; }
  if (sCl=="noin" ) { Cm_SoftswitchInstrMode(*i, false);   continue; }
  if (sCl=="reqi" ) { Cm_SoftswitchReqIdleMode(*i, true);  continue; }
  if (sCl=="nore" ) { Cm_SoftswitchReqIdleMode(*i, false); continue; }
  if (sCl=="rtsb" ) { Cm_SoftswitchSetRTSBuffSize(*i);     continue; }
  if (sCl=="args" ) { Cm_SoftswitchAddFlags(*i);           continue; }

  if (sCl=="dump" ) { Dump(0,par->fd);                     continue; }
  if (sCl=="show" ) { Show(par->fd);                       continue; }

  par->Post(25,sCl,"compose");         // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================

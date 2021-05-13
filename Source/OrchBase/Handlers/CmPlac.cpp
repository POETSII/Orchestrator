/* Defines the behaviour of "placement" commands from the operator. A synopsis
 * can be found in the placement documentation (or talk to MLV). */

#include "CmPlac.h"
#include "OrchBase.h"

CmPlac::CmPlac(OrchBase * p):par(p){par->fd = stdout;}
CmPlac::~CmPlac(){}

/* Everything goes through here. always returns zero. */
unsigned CmPlac::operator()(Cli* cli)
{
    /* Simply put, no placement tasks make any sense if there's no engine
     * loaded, so we catch that early. */
    if (par->pE == PNULL)
    {
        par->Post(300);
        return 0;
    }
    if (par->pE->is_empty())
    {
        par->Post(300);
        return 0;
    }

    /* For each clause in the command, do something interesting. If there are
     * no clauses (i.e. the command is literally `placement`), do nothing. */
    std::vector<Cli::Cl_t>::iterator clause;
    std::string clauseRoot;
    for (clause = cli->Cl_v.begin(); clause != cli->Cl_v.end(); clause++)
    {
        /* Call the appropriate method (from the first four characters of the
         * clause). */
        bool isClauseValid = true;
        clauseRoot = clause->Cl.substr(0, 4);
        if (clauseRoot == "dump") PlacementDump(*clause);
        else if (clauseRoot == "unpl") PlacementUnplace(*clause);

        /* NB: If the operator passes an argument this command (e.g. 'placement
         * /reset = APPLICATION'), we warn the operator that they're doing
         * something a bit silly. */
        else if (clauseRoot == "rese")
        {
            if (!clause->Pa_v.empty()) par->Post(315);
            /* Argument supports shameless code reuse. */
            else par->PlacementReset(true);
        }

        /* Are we defining a constraint on the command line? */
        else if (clauseRoot == "cons") PlacementConstrain(*clause);

        /* Are we loading a placement spec? */
        else if (clauseRoot == "load") PlacementLoad(*clause);

        /* If nothing is appropriate, assume it's a placement algorithm. */
        else isClauseValid = PlacementDoIt(*clause);

        /* If it isn't a placement algorithm, we whinge. */
        if (!isClauseValid) par->Post(25, clause->Cl, "placement");
    }
    return 0;
}

/* Dumps the command object, not placement information. */
void CmPlac::Dump(FILE * fp)
{
fprintf(fp,"CmPlac+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
if (par==0) fprintf(fp,"OrchBase parent not defined\n");
else fprintf(fp,"Orchbase parent : %s\n",par->FullName().c_str());
fprintf(fp,"CmPlac-------------------------------------------------------\n\n");
fflush(fp);
}

/* Apply a single hard constraint to the placer, which will be enforced on all
 * applications placed on the hardware model going forward. */
void CmPlac::PlacementConstrain(Cli::Cl_t clause)
{
    /* Skip (and post) if there is less than one parameter (i.e. constraint
     * type). */
    if (clause.Pa_v.size() < 1)
    {
        par->Post(48, clause.Cl, "placement", "1");
        return;
    }

    /* Behaviour depends on the constraint type... */
    if (Cli::StrEq(clause.Pa_v[0].Concatenate(), "MaxDevicesPerThread", 20))
    {
        /* Skip (and post) if there are not exactly two parameters. The second
         * parameter is the constraining value. */
        if (clause.Pa_v.size() != 2)
        {
            par->Post(321, clause.Pa_v[0].Concatenate(), "1",
                      clause.Pa_v[1].Concatenate());
            return;
        }

        /* Remove existing constraint of this kind, regardless of its value. */
        std::list<Constraint*>::iterator constraintIt;
        for (constraintIt = par->pPlacer->constraints.begin();
             constraintIt != par->pPlacer->constraints.end();)
        {
            if ((*constraintIt)->category == maxDevicesPerThread and
                (*constraintIt)->gi == PNULL and
                (*constraintIt)->mandatory)
            {
                delete (*constraintIt);
                constraintIt = par->pPlacer->constraints.erase(constraintIt);
            }

            else constraintIt++;
        }

        /* Otherwise, apply the constraint to the placer. */
        par->pPlacer->constraints.push_back(
            new MaxDevicesPerThread(true, 0, PNULL,
                                    str2uint(clause.Pa_v[1].Concatenate())));
        par->Post(322, clause.Pa_v[1].Concatenate());
    }

    else if (Cli::StrEq(clause.Pa_v[0].Concatenate(), "MaxThreadsPerCore", 20))
    {
        /* Skip (and post) if there are not exactly two parameters. The second
         * parameter is the constraining value. */
        if (clause.Pa_v.size() != 2)
        {
            par->Post(321, clause.Pa_v[0].Concatenate(), "1",
                      clause.Pa_v[1].Concatenate());
            return;
        }

        /* Remove existing constraint of this kind, regardless of its value. */
        std::list<Constraint*>::iterator constraintIt;
        for (constraintIt = par->pPlacer->constraints.begin();
             constraintIt != par->pPlacer->constraints.end();)
        {
            if ((*constraintIt)->category == maxThreadsPerCore and
                (*constraintIt)->gi == PNULL and
                (*constraintIt)->mandatory)
            {
                delete (*constraintIt);
                constraintIt = par->pPlacer->constraints.erase(constraintIt);
            }

            else constraintIt++;
        }

        /* Otherwise, apply the constraint to the placer. */
        par->pPlacer->constraints.push_back(
            new MaxThreadsPerCore(true, 0, PNULL,
                                  str2uint(clause.Pa_v[1].Concatenate())));
        par->Post(323, clause.Pa_v[1].Concatenate());
    }

    else par->Post(320, clause.Pa_v[0].Concatenate());
}

/* Returns true if the clause was valid (or some other error happened), and
 * false otherwise. */
bool CmPlac::PlacementDoIt(Cli::Cl_t clause)
{
    /* Grab the graph instances of interest. */
    std::set<GraphI_t*> graphs;
    if (par->GetGraphIs(clause, graphs) == 1) return true;

    /* Check they're all typelinked before proceeding. */
    std::set<GraphI_t*>::iterator graphIt;
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        if ((*graphIt)->pT == PNULL)
        {
            par->Post(324, (*graphIt)->Name(), "placing");
            return true;
        }
    }

    /* Have a pop. */
    try
    {
        for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
        {
            par->Post(309, (*graphIt)->Name(), clause.Cl);
            par->pPlacer->place(*graphIt, clause.Cl);
            par->Post(302, (*graphIt)->Name());
        }
    }
    catch (InvalidAlgorithmDescriptorException&)
    {
        return false;  /* Let CmPlac() handle it - it's an invalid clause, and
                        * we need to be consistent with other Orchestrator
                        * commands. */
    }
    catch (AlreadyPlacedException&) {par->Post(303, (*graphIt)->Name());}
    catch (BadIntegrityException& e) {par->Post(304, (*graphIt)->Name(),
                                                e.message);}
    catch (CostCacheException& e) {par->Post(316, (*graphIt)->Name(),
                                             e.message);}
    catch (FileOpenException& e) {par->Post(318, (*graphIt)->Name(),
                                            e.message);}
    catch (InvalidArgumentException& e) {par->Post(325, (*graphIt)->Name(),
                                                   clause.Cl, e.message);}
    catch (NoEngineException&) {par->Post(305, (*graphIt)->Name());}
    catch (NoSpaceToPlaceException&) {par->Post(306, (*graphIt)->Name());}
    return true;
}

void CmPlac::PlacementDump(Cli::Cl_t clause)
{
    /* Grab the graph instances of interest. */
    std::set<GraphI_t*> graphs;
    if (par->GetGraphIs(clause, graphs) == 1) return;

    /* Don't do anything if any of the graphs have not been placed (by
     * address). */
    std::map<GraphI_t*, Algorithm*>::iterator graphFinder;
    std::set<GraphI_t*>::iterator graphIt;
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        graphFinder = par->pPlacer->placedGraphs.find(*graphIt);
        if (graphFinder == par->pPlacer->placedGraphs.end())
        {
            par->Post(311, (*graphIt)->Name());
            return;
        }
    }

    /* If one or more graphs have a placement score of zero, warn the user that
     * the dump will take a while, because we need to calculate that to make
     * the dump meaningful. */
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        graphFinder = par->pPlacer->placedGraphs.find(*graphIt);
        if (graphFinder->second->result.score == 0)
        {
            par->Post(312, graphFinder->first->Name());  /* Only the first */
        }
        break;
    }

    /* Have a pop. */
    try
    {
        for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
        {
            par->pPlacer->dump(*graphIt);
            par->Post(310, (*graphIt)->Name());
        }
    }
    catch (PthreadException& e) {par->Post(313, e.message);}
    catch (CostCacheException& e) {par->Post(317, e.message);}
    catch (FileOpenException& e) {par->Post(319, e.message);}
}

/* Load a placement map. Will fall over violently if the file is invalid. */
void CmPlac::PlacementLoad(Cli::Cl_t clause)
{
    /* Skip (and post) if there are not exactly two parameters (graph instance
     * and file). */
    if (clause.Pa_v.size() != 2)
    {
        par->Post(47, clause.Cl, "placement", "2");
        return;
    }

    /* Get the graph instance. */
    std::set<GraphI_t*> graphs;
    if (par->GetGraphIs(clause, graphs, 1) == 1) return;

    /* Check it's typelinked before proceeding. */
    if ((*graphs.begin())->pT == PNULL)
    {
        par->Post(324, (*graphs.begin())->Name(), "loading");
    }

    /* Get the file path. */
    std::string path = clause.Pa_v[1].Concatenate();

    /* Have a pop. */
    try
    {
        par->Post(314, path, (*graphs.begin())->Name());
        par->pPlacer->place_load((*graphs.begin()), path);
        par->Post(302, (*graphs.begin())->Name());
    }
    catch (AlreadyPlacedException&)
        {par->Post(303, (*graphs.begin())->Name());}
    catch (BadIntegrityException& e)
        {par->Post(304, (*graphs.begin())->Name(), e.message);}
    catch (NoEngineException&)
        {par->Post(305, (*graphs.begin())->Name());}
    catch (NoSpaceToPlaceException&)
        {par->Post(306, (*graphs.begin())->Name());}
    return;
}

void CmPlac::PlacementUnplace(Cli::Cl_t clause)
{
    /* Grab the graph instances of interest. */
    std::set<GraphI_t*> graphs;
    if (par->GetGraphIs(clause, graphs) == 1) return;

    /* Note that unplace doesn't whine if the task doesn't exist, and doesn't
     * whine if the task exists and has not been placed. It's an idempotent
     * method that ensures the task has no presence in the placer. */
    std::set<GraphI_t*>::iterator graphIt;
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        par->pPlacer->unplace(*graphIt);
        par->Post(307, (*graphIt)->Name());
    }
}

/* Synopsis of placement information. */
void CmPlac::Show(FILE* fp)
{
    fprintf(fp,
            "\nPlacement subsystem attributes and state:\n"
            "Number of graphs placed: %lu\n"
            "Number of devices placed: %lu\n"
            "Number of threads used for placement: %lu\n"
            "Number of explicit constraints defined: %lu\n",
            par->pPlacer->placedGraphs.size(),
            par->pPlacer->deviceToThread.size(),
            par->pPlacer->threadToDevices.size(),
            par->pPlacer->constraints.size());
    fflush(fp);
}

/* Defines the behaviour of "placement" commands from the operator. A synopsis
 * can be found in the placement documentation (or talk to MLV). */

#include "CmPlac.h"
#include "OrchBase.h"

CmPlac::CmPlac(OrchBase * p):par(p){par->fd = stdout;}

/* Everything goes through here. always returns zero. */
unsigned CmPlac::operator()(Cli* pC)
{
    /* Simply put, no placement tasks make any sense if there's no engine
     * loaded, so we catch that early. */
    if (par->pE == 0)
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
            else PlacementReset(true);
        }

        /* Are we defining a constraint on the command line? */
        else if (clauseRoot == "cons") PlacementConstrain(*clause);

        /* Are we loading a placement spec? */
        else if (clauseRoot == "load") PlacementLoad(*clause);

        /* If nothing is appropriate, assume it's a placement algorithm. */
        else isClauseValid = PlacementDoit(*clause);

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
    if (Cli::StrEq(clause.Pa_v[0].Val, "MaxDevicesPerThread", 20))
    {
        /* Skip (and post) if there are not exactly two parameters. The second
         * parameter is the constraining value. */
        if (clause.Pa_v.size() != 2)
        {
            par->Post(321, clause.Pa_v[0].Val, "1", clause.Pa_v[1].Val);
            return;
        }

        /* Remove existing constraint of this kind, regardless of its value. */
        std::list<Constraint*>::iterator constraintIt;
        for (constraintIt = par->pPlacer->constraints.begin();
             constraintIt != par->pPlacer->constraints.end();)
        {
            if ((*constraintIt)->category == maxDevicesPerThread and
                (*constraintIt)->task == PNULL and
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
                                    str2uint(clause.Pa_v[1].Val)));
        par->Post(322, clause.Pa_v[1].Val);
    }

    else if (Cli::StrEq(clause.Pa_v[0].Val, "MaxThreadsPerCore", 20))
    {
        /* Skip (and post) if there are not exactly two parameters. The second
         * parameter is the constraining value. */
        if (clause.Pa_v.size() != 2)
        {
            par->Post(321, clause.Pa_v[0].Val, "1", clause.Pa_v[1].Val);
            return;
        }

        /* Remove existing constraint of this kind, regardless of its value. */
        std::list<Constraint*>::iterator constraintIt;
        for (constraintIt = par->pPlacer->constraints.begin();
             constraintIt != par->pPlacer->constraints.end();)
        {
            if ((*constraintIt)->category == maxThreadsPerCore and
                (*constraintIt)->task == PNULL and
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
                                  str2uint(clause.Pa_v[1].Val)));
        par->Post(323, clause.Pa_v[1].Val);
    }

    else par->Post(320, clause.Pa_v[0].Val);
}

/* Returns true if the clause was valid (or some other error happened), and
 * false otherwise. */
bool CmPlac::PlacementDoit(Cli::Cl_t clause)
{
    /* Skip (and post) if there is not exactly one parameter (i.e. task). */
    if (clause.Pa_v.size() != 1)
    {
        par->Post(47, clause.Cl, "placement", "1");
        return true;
    }

    /* Get (what we assume to be) the task handle, and the task. */
    std::string taskHandle = clause.Pa_v[0].Val;
    P_task* task = PlacementGetTaskByName(taskHandle);
    if (task == PNULL) return true;

    /* Have a pop. */
    try
    {
        par->Post(309, taskHandle, clause.Cl);
        par->pPlacer->place(task, clause.Cl);
        par->Post(302, taskHandle);
    }
    catch (InvalidAlgorithmDescriptorException&)
    {
        return false;  /* Let CmPlace handle it - it's an invalid clause, and
                        * we need to be consistent with other Orchestrator
                        * commands. */
    }
    catch (AlreadyPlacedException&) {par->Post(303, taskHandle);}
    catch (BadIntegrityException& e) {par->Post(304, taskHandle, e.message);}
    catch (CostCacheException& e) {par->Post(316, taskHandle, e.message);}
    catch (FileOpenException& e) {par->Post(318, taskHandle, e.message);}
    catch (NoEngineException&) {par->Post(305, taskHandle);}
    catch (NoSpaceToPlaceException&) {par->Post(306, taskHandle);}
    return true;
}

void CmPlac::PlacementDump(Cli::Cl_t clause)
{
    /* Skip (and post) if there is not exactly one parameter (i.e. task). */
    if (clause.Pa_v.size() != 1)
    {
        par->Post(47, clause.Cl, "placement", "1");
        return;
    }

    /* Get (what we assume to be) the task handle, and the task. */
    std::string taskHandle = clause.Pa_v[0].Val;
    P_task* task = PlacementGetTaskByName(taskHandle);
    if (task == PNULL) return;

    /* Don't do anything if the task has not been placed (by address). */
    std::map<P_task*, Algorithm*>::iterator tasksIt;
    tasksIt = par->pPlacer->placedTasks.find(task);
    if (tasksIt == par->pPlacer->placedTasks.end())
    {
        par->Post(311, taskHandle);
        return;
    }

    /* If the task has a placement score of zero, warn the user that the dump
     * will take a while, because we need to calculate that to make the dump
     * meaningful. */
    if (tasksIt->second->result.score == 0) par->Post(312, taskHandle);

    /* Have a pop. */
    try
    {
        par->pPlacer->dump(task);
        par->Post(310, taskHandle);
    }
    catch (PthreadException& e) {par->Post(313, e.message);}
    catch (CostCacheException& e) {par->Post(317, e.message);}
    catch (FileOpenException& e) {par->Post(319, e.message);}
}

/* Shortcut method to get a task object from its handle. */
P_task* CmPlac::PlacementGetTaskByName(std::string taskHandle)
{
    std::map<std::string, P_task*>::iterator taskFinder = \
        P_taskm.find(taskHandle);
    if (taskFinder == P_taskm.end())
    {
        par->Post(301, taskHandle);
        return PNULL;
    }
    else
    {
        return taskFinder->second;
    }
}

/* Load a placement map. Will fall over violently if the file is invalid. */
void CmPlac::PlacementLoad(Cli::Cl_t clause)
{
    /* Skip (and post) if there are not exactly two parameters (i.e. task and
     * file). */
    if (clause.Pa_v.size() != 2)
    {
        par->Post(47, clause.Cl, "placement", "2");
        return;
    }

    /* Get (what we assume to be) the task handle, and the task. */
    std::string taskHandle = clause.Pa_v[0].Val;
    P_task* task = PlacementGetTaskByName(taskHandle);
    if (task == PNULL) return;

    std::string path = clause.Pa_v[1].Val;

    /* Have a pop. */
    try
    {
        par->Post(314, path, taskHandle);
        par->pPlacer->place_load(task, path);
        par->Post(302, taskHandle);
    }
    catch (AlreadyPlacedException&) {par->Post(303, taskHandle);}
    catch (BadIntegrityException& e) {par->Post(304, taskHandle, e.message);}
    catch (NoEngineException&) {par->Post(305, taskHandle);}
    catch (NoSpaceToPlaceException&) {par->Post(306, taskHandle);}
    return;
}

void CmPlac::PlacementUnplace(Cli::Cl_t clause)
{
    /* Skip (and post) if there is not exactly one parameter (i.e. task). */
    if (clause.Pa_v.size() != 1)
    {
        par->Post(47, clause.Cl, "placement", "1");
        return;
    }

    /* Get (what we assume to be) the task handle, and the task. */
    std::string taskHandle = clause.Pa_v[0].Val;
    P_task* task = PlacementGetTaskByName(taskHandle);
    if (task == PNULL) return;

    /* Note that unplace doesn't whine if the task doesn't exist, and doesn't
     * whine if the task exists and has not been placed. It's an idempotent
     * method that ensures the task has no presence in the placer. */
    par->pPlacer->unplace(task);
    par->Post(307, taskHandle);
}

/* Synopsis of placement information. */
void CmPlac::Show(FILE* fp)
{
    fprintf(fp,
            "\nPlacement subsystem attributes and state:\n"
            "Number of graphs placed: %lu\n",
            "Number of devices placed: %lu\n",
            "Number of threads used for placement: %lu\n",
            "Number of explicit constraints defined: %lu\n",
            par->pPlacer->placedGraphs.size(),
            par->pPlacer->deviceToThread.size(),
            par->pPlacer->threadToDevices.size(),
            par->pPlacer->constraints.size());
    fflush(fp);
}

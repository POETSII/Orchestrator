/* Defines the behaviour of "placement" commands from the operator. A synopsis
 * can be found in the placement documentation (or talk to MLV). */

/* Everything goes through here. always returns zero. */
unsigned OrchBase::CmPlace(Cli* cli)
{
    /* Simply put, no placement tasks make any sense if there's no engine
     * loaded, so we catch that early. */
    if (pE == 0)
    {
        Post(200);
        return 0;
    }
    if (pE->is_empty())
    {
        Post(200);
        return 0;
    }

    /* For each clause in the command, do something interesting. If there are
     * no clauses (i.e. the command is literally `placement`), do nothing. */
    std::vector<Cli::Cl_t>::iterator clause;
    for (clause = cli->Cl_v.begin(); clause != cli->Cl_v.end(); clause++)
    {
        /* Call the appropriate method (from the first four characters of the
         * clause). */
        bool isClauseValid = true;
        switch (clause->cl.substr(0, 4))
        {
        case "dump": PlacementDump(*clause);
        case "rese": PlacementReset(*clause);
        case "unpl": PlacementUnplace(*clause);

        /* If nothing is appropriate, assume it's a placement algorithm. */
        default: isClauseValid = PlacementDoit(*clause);
        }

        /* If it isn't a placement algorithm, we whinge. */
        if (!isClauseValid) Post(25, clause->Cl, "placement");
    }
    return 0;
}

bool PlacementDoit(Cli::Cl_t clause)
{
    /* Skip if there are no parameters. */
    if (clause.Pa_v.empty()) return;

    /* Get (what we assume to be) the task handle, and the task. */
    std::string taskHdl = clause.Pa_v[0].Val;
    P_task* task = PlacementGetTaskByName(taskHdl);
    if (task == PNULL) return true;

    /* Have a pop. */
    try
    {
        placer.place(task, clause.Cl);
        Post(202, taskHdl);
    }
    catch (InvalidAlgorithmDescriptorException exc)
    {
        return false;  /* Let CmPlace handle it - it's an invalid clause, and
                        * we need to be consistent with other Orchestrator
                        * commands. */
    }
    catch (AlreadyPlacedException exc) Post(203, taskHdl);
    catch (BadIntegrityException exc) Post(204, taskHdl);
    catch (NoEngineException exc) Post(205, taskHdl);
    catch (NoSpaceToPlaceException exc) Post(206, taskHdl);
    return true;
}

void PlacementDump(Cli::Cl_t clause){}  /* <!> Yet another stub. */

/* Shortcut method to get a task object from its handle. */
P_task* PlacementGetTaskByName(std::string taskHdl)
{
    std::map<std::string, P_task*>::iterator taskFinder = P_taskm.find(taskHdl)
    if (taskFinder == P_taskm.end())
    {
        Post(201, taskHdl);
        return PNULL;
    }
    else
    {
        return taskFinder->second;
    }
}

void PlacementReset(Cli::Cl_t clause)
{
    /* NB: We don't check the clause. If the operator types 'placement /reset =
     * APPLICATION', we clear everything anyway.
     *
     * Praise the deities of software design. */
    placer = Placer(pE);
    Post(208);
}

void PlacementUnplace(Cli::Cl_t clause)
{
    /* Skip if there are no parameters. */
    if (clause.Pa_v.empty()) return;

    /* Get (what we assume to be) the task handle, and the task. */
    std::string taskHdl = clause.Pa_v[0].Val;
    P_task* task = PlacementGetTaskByName(taskHdl)
    if (task == PNULL) return;

    /* Note that unplace doesn't whine if the task doesn't exist, and doesn't
     * whine if the task exists and has not been placed. It's an idempotent
     * method that ensures the task has no presence in the placer. */
    placer.unplace(task);
    Post(207, taskHdl);
}

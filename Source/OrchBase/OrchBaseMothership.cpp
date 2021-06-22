/* A split-off definitions file for OrchBase methods that deal or communicate
 * with Motherships in some way. */

#include "OrchBase.h"
#include "Pglobals.h"

/* Constructs the bimap of mothership processes to boxes in the engine
 * (OrchBase.P_SCMm2). In single-supervisor mode, just finds the lone
 * Mothership, if there is one. */
void OrchBase::BuildMshipMap()
{
    std::vector<ProcMap::ProcMap_t>::iterator procIt;
#if SINGLE_SUPERVISOR_MODE
    procIt = pPmap->vPmap.begin();
    while (procIt != pPmap->vPmap.end() and
           procIt->P_class != csMOTHERSHIPproc) procIt++;
    if (procIt == pPmap->vPmap.end()) loneMothership = PNULL;
    else loneMothership = &*procIt;
#else
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
#endif
}

/* Sends a command-and-control message to all Motherships for the set of graph
 * instances represented by a command clause. Up to the caller to verify that
 * the command makes sense. */
void OrchBase::MshipCommand(Cli::Cl_t clause, std::string command)
{
    std::set<GraphI_t*> graphs;
    std::set<GraphI_t*>::iterator graphIt;

    /* Get the application graph instances */
    GetGraphIs(clause, graphs);

    /* Ensure that all graph instances are deployed before proceeding. */
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        if (!(*graphIt)->deployed)
        {
            Post(169, command, (*graphIt)->GetCompoundName());
            return;
        }
    }

    /* Iterate through each graph instance in turn. */
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        PMsg_p message;
        std::string graphName = (*graphIt)->GetCompoundName();

        /* Boxes that are relevant for the application being commanded. */
        std::set<P_box*> boxesOfImport;
        std::set<P_box*>::iterator boxIt;

        /* Set up the message given the input arguments (catching an invalid
         * command input from somewhere). */
        int postIndex = 999;  /* These defaults can never be used. */
        byte commandKey = Q::N000;
        if (command == "init")
        {
            commandKey = Q::INIT;
            postIndex = 187;  /* Search string: Post(187 */
        }
        else if (command == "run")
        {
            commandKey = Q::RUN;
            postIndex = 188;  /* Search string: Post(188 */
        }
        else if (command == "stop")
        {
            commandKey = Q::STOP;
            postIndex = 189;  /* Search string: Post(189 */
        }
        else if (command == "reca")
        {
            commandKey = Q::RECL;
            postIndex = 190;
            (*graphIt)->deployed = false;
            /* Note we don't clear deployment information here, because we want
             * to see acknowledgements from the Mothership process(es). Those
             * acknowledgements will remove their appropriate process from the
             * deployment information structure. */
            deplStat[(*graphIt)->Name()] = "RECALLING";
        }

        message.Key(Q::CMND, commandKey);
        message.Src(Urank);
        message.Put(0, &graphName);

#if SINGLE_SUPERVISOR_MODE
        /* Just yeet it to the first Mothership (there's guaranteed to be one
         * at this point, as we could not have deployed earlier otherwise). */
        message.Tgt(loneMothership->P_rank);
        message.Send();
#else
        /* Get the set of important boxes. */
        pPlacer->get_boxes_for_gi(*graphIt, &boxesOfImport);

        /* For each box, send to the Mothership on that box. */
        for (boxIt = boxesOfImport.begin(); boxIt != boxesOfImport.end();
             boxIt++)
        {
            message.Tgt(P_SCMm2[*boxIt]->P_rank);
            message.Send();
        }
#endif
        Post(postIndex, (*graphIt)->Name());
    }
}

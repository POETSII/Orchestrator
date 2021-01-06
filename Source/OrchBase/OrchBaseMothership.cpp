/* A split-off definitions file for OrchBase methods that deal or communicate
 * with Motherships in some way. */

#include "OrchBase.h"
#include "Pglobals.h"

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
            Post(169, command, (*graphIt)->Name());
            return;
        }
    }

    /* Iterate through each graph instance in turn. */
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        PMsg_p message;
        std::string graphName = (*graphIt)->Name();

        /* Boxes that are relevant for the application being commanded. */
        std::set<P_box*> boxesOfImport;
        std::set<P_box*>::iterator boxIt;

        /* Set up the message given the input arguments (catching an invalid
         * command input from somewhere). */
        if (command == "reca")
        {
            message.Key(Q::CMND, Q::RECL);
            (*graphIt)->deployed = false;
        }
        else if (command == "init") message.Key(Q::CMND, Q::INIT);
        else if (command == "run") message.Key(Q::CMND,Q::RUN);
        else if (command == "stop") message.Key(Q::CMND,Q::STOP);
        message.Src(Urank);
        message.Put(0, &graphName);

        /* Get the set of important boxes. */
        pPlacer->get_boxes_for_gi(*graphIt, &boxesOfImport);

        /* For each box, send to the Mothership on that box. */
        for (boxIt = boxesOfImport.begin(); boxIt != boxesOfImport.end();
             boxIt++)
        {
            message.Tgt(P_SCMm2[*boxIt]->P_rank);
            message.Send();
        }
    }
}

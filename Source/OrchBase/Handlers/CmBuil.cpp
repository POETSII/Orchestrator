//------------------------------------------------------------------------------

#include "CmBuil.h"
#include "OrchBase.h"
#include "Pglobals.h"

//==============================================================================

CmBuil::CmBuil(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmBuil::~CmBuil()
{
}

//------------------------------------------------------------------------------

struct DistPayload
{
    std::string codePath;
    std::string dataPath;
    AddressComponent coreAddr;
    std::vector<AddressComponent> threadsExpected;
};

//------------------------------------------------------------------------------

void CmBuil::Cm_Deploy(Cli::Cl_t clause)
{
    std::set<GraphI_t*> graphs;
    std::set<GraphI_t*>::iterator graphIt;

    /* Get the application graph instances */
    par->GetGraphIs(clause, graphs);

    /* Ensure that all graph instances are placed before proceeding. */
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        if (!(*graphIt)->built)
        {
            par->Post(107, (*graphIt)->Name());
            return;
        }
    }

    /* Deploy each graph instance in sequence, failing fast. */
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        if (DeployGraph(*graphIt) != 0) return;
    }
}

//------------------------------------------------------------------------------

void CmBuil::Cm_Do(Cli::Cl_t clause, std::string command)
{
    std::set<GraphI_t*> graphs;
    std::set<GraphI_t*>::iterator graphIt;

    /* Get the application graph instances */
    par->GetGraphIs(clause, graphs);

    /* Ensure that all graph instances are deployed before proceeding. */
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        if (!(*graphIt)->deployed)
        {
            par->Post(169, (*graphIt)->Name());
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
        if (command == "recl")
        {
            message.Key(Q::CMND, Q::RECL);
            (*graphIt)->deployed = false;
        }
        else if (command == "init") message.Key(Q::CMND, Q::INIT);
        else if (command == "run") message.Key(Q::CMND,Q::RUN);
        else if (command == "stop") message.Key(Q::CMND,Q::STOP);
        message.Src(par->Urank);
        message.Put(0, &graphName);

        /* Get the set of important boxes. */
        par->pPlacer->get_boxes_for_gi(*graphIt, &boxesOfImport);

        /* For each box, send to the Mothership on that box. */
        for (boxIt = boxesOfImport.begin(); boxIt != boxesOfImport.end();
             boxIt++)
        {
            message.Tgt(par->P_SCMm2[*boxIt]->P_rank);
            message.Send();
        }
    }
}

//------------------------------------------------------------------------------

/* Deploys a built graph instance to Motherships.
 *
 * This involves two stages:
 *
 *  - Deploying the binaries to the filesystems on which the Motherships
 *    operate.
 *
 *  - Sending messages (MPI) to each relevant Mothership process describing the
 *    application (see the Mothership documentation).
 */
int CmBuil::DeployGraph(GraphI_t* gi)
{
    /* Finding the machine name of Root. */
    std::vector<ProcMap::ProcMap_t>::iterator rootFinder;
    std::string rootMachineName;

    /* Holding Mothership process information. */
    ProcMap::ProcMap_t* mothershipProc;
    int rank;

    /* Iteration through the hardware model with respect to boxes. */
    std::map<AddressComponent, P_box*>::iterator boxIt;
    std::vector<P_board*>::iterator boardIt;
    std::map<AddressComponent, P_core*>::iterator coreIt;
    std::map<AddressComponent, P_thread*>::iterator threadIt;
    P_core* core;
    P_thread* thread;

    /* Staging area for DIST message payloads, keyed by Mothership process
     * communicator and rank. */
    std::map<int, std::vector<DistPayload> > mothershipPayloads;
    std::map<int, std::vector<DistPayload> >::iterator mothershipPayloadsIt;
    std::vector<DistPayload>::iterator payloadIt;
    DistPayload* payload;

    /* Execution of commands, used for deploying binaries. */
    std::vector<std::string> commands;
    std::vector<std::string>::iterator command;
    std::string sourceBinaries;  /* Where the binaries are. */
    std::string target;  /* Where they will go. */
    std::string host;  /* For remote hosts. */

    /* Messages */
    PMsg_p specMessage;
    PMsg_p distMessage;
    PMsg_p supdMessage;
    std::vector<PMsg_p*> messages;
    std::vector<PMsg_p*>::iterator messageIt;

    /* Other payloady bits. */
    std::string graphName = gi->Name();
    unsigned distCount;
    uint8_t appNumber;
    std::string soPath;

    /* Identify the name of the machine on which Root is running. */
    rootFinder = par->pPmap->vPmap.begin();
    while (rootFinder->P_rank != par->pPmap->U.Root) rootFinder++;
    rootMachineName = rootFinder->P_proc;

    /* Iterate over each box in the hardware model */
    for (boxIt = par->pE->P_boxm.begin(); boxIt != par->pE->P_boxm.end();
         boxIt++)
    {
        /* Grab the Mothership for this box (which may be invalid). We don't
         * exit if we find an invalid entry - there may not be any devices for
         * this graph instance mapped to the box in question. */
        mothershipProc = par->P_SCMm2[boxIt->second];

        /* A special "null" value defined for this rank. We have not yet
         * checked to see if this box has a Mothership mapped to it - we don't
         * want to error out without first checking that there are no devices
         * attached to this box. */
        rank = -1;

        /* Iterate over all cores in this box, in an attempt to find devices
         * owned by the graph instance that are mapped to this box. Squashed
         * indentation to make logic easier to follow. */
        for (boardIt = boxIt->second->P_boardv.begin();
             boardIt != boxIt->second->P_boardv.end(); boardIt++)
        WALKPDIGRAPHNODES(AddressComponent, P_mailbox*,
                          unsigned, P_link*,
                          unsigned, P_port*, (*boardIt)->G, mailboxIt)
        for (coreIt = (*boardIt)->G.NodeData(mailboxIt)->P_corem.begin();
             coreIt != (*boardIt)->G.NodeData(mailboxIt)->P_corem.end();
             coreIt++)
        {
            core = coreIt->second;
            thread = core->P_threadm.begin()->second;  /* The first thread on
                                                        * this core */

            /* Skip this core if either nothing is placed on it, or if the
             * devices placed on it are owned by a different graph
             * instance. Recall that all devices within a core service the same
             * graph instance.
             *
             * Note the deliberate use of indexing instead of `at` for the
             * first predicate. */
            if (par->pPlacer->threadToDevices[thread].empty() or
                par->pPlacer->giToCores.at(gi).find(core) ==
                par->pPlacer->giToCores.at(gi).end())
            {
                continue;
            }

            /* If we couldn't find a Mothership for this box earlier, we panic
             * here, because we can't deploy this graph instance without enough
             * Motherships to support it. This happens because there are more
             * boxes in the Engine than there are Mothership processes
             * running. We leave in this case.
             *
             * NB: We don't leave as soon as we discover that there aren't
             * enough Motherships, because it's possible that the extra boxes
             * have nothing relevant placed on them. In that case, the graph
             * instance can still execute as expected. */
            if (mothershipProc == PNULL)
            {
                par->Post(178, graphName);
                return 1;
            }
            else rank = mothershipProc->P_rank;

            /* Define the payload for a DIST message for this core. */
            mothershipPayloads[rank].push_back(DistPayload());
            payload = &(mothershipPayloads[rank].back());

            /* paths */
            payload->codePath = core->instructionBinary;
            payload->dataPath = core->dataBinary;

            /* coreAddr */
            payload->coreAddr = core->get_hardware_address()->as_uint();

            /* threadsExpected - only push_back threads that have something
             * placed upon them. Again, we assume that all devices placed
             * within a core share the same application. */
            for (threadIt = core->P_threadm.begin();
                 threadIt != core->P_threadm.end(); threadIt++)
            {
                if (!(par->pPlacer->threadToDevices
                      .at(threadIt->second).empty()))
                    payload->threadsExpected.push_back(
                        threadIt->second->get_hardware_address()->as_uint());
            }
        }

        /* If we found no devices for this graph instance on this box, or if
         * the box isn't supported by a Mothership, skip to the next box. */
        if (mothershipPayloads.find(rank) == mothershipPayloads.end() or
            rank == -1) continue;

        /* At this point, we are sure that there is a Mothership that can
         * represent this box, and we are also sure that there are devices that
         * this Mothership needs to supervise. Now we are going to deploy
         * binaries to appropriate locations for the Mothership to find
         * them. To do this, we naively copy all binaries to all Motherships
         * for now. */
        target = dformat("%s/%s/%s", getenv("HOME"),
                         par->pCmPath->pathMshp.c_str(), graphName.c_str());
        sourceBinaries = dformat("%s/%s/*",
                                 par->pCmPath->pathBina.c_str(),
                                 graphName.c_str());

        /* Identify whether or not this Mothership is running on the same
         * machine as Root, to determine how we deploy binaries. Store the
         * commands-to-be-run in a vector. */
        if (rootMachineName == mothershipProc->P_proc)
        {
            commands.push_back(dformat("rm -r -f %s", target.c_str()));
            commands.push_back(dformat("mkdir -p %s", target.c_str()));
            commands.push_back(dformat("cp -r %s %s", sourceBinaries.c_str(),
                                       target.c_str()));
        }

        /* Note that, if the machine is different, we deploy binaries using
         * SCP. */
        else
        {
            host = dformat("%s@%s", mothershipProc->P_user,
                           mothershipProc->P_proc);
            commands.push_back(dformat("ssh %s \"rm -r -f %s\"",
                                       host.c_str(), target.c_str()));
            commands.push_back(dformat("ssh %s \"mkdir -p %s\"",
                                       host.c_str(), target.c_str()));
            commands.push_back(dformat("scp -r %s %s:%s",
                                       sourceBinaries.c_str(), host.c_str(),
                                       target.c_str()));
        }

        /* Run each staged command, failing fast if one of them breaks. */
        for (command = commands.begin(); command != commands.end(); command++)
        {
            if (system(command->c_str()) > 0)
            {
                /* Command failed, cancelling deployment. */
                if (errno == 0)
                {
                    par->Post(177, command->c_str());
                }
                else
                {
                    par->Post(179, command->c_str(),
                              OSFixes::getSysErrorString(errno).c_str());
                }
                return 1;
            }
        }
    }

    /* Send SPEC, DIST, and SUPD messages to each Mothership for this
     * graph instance. Order does not matter. */

    /* Prepare common behaviour for all messages of their type. */
    messages.push_back(&specMessage);
    messages.push_back(&distMessage);
    messages.push_back(&supdMessage);
    for (messageIt = messages.begin(); messageIt != messages.end();
         messageIt++)
    {
        (*messageIt)->Src(par->Urank);
        (*messageIt)->Put(0, &(graphName));
    }
    specMessage.Key(Q::APP, Q::SPEC);
    distMessage.Key(Q::APP, Q::DIST);
    supdMessage.Key(Q::APP, Q::SUPD);

    /* Iterate through participating Motherships. */
    for (mothershipPayloadsIt = mothershipPayloads.begin();
         mothershipPayloadsIt != mothershipPayloads.end();
         mothershipPayloadsIt++)
    {
        /* Set the destination. */
        for (messageIt = messages.begin(); messageIt != messages.end();
             messageIt++)
        {
            (*messageIt)->Tgt(mothershipPayloadsIt->first);
        }

        /* Customise and send the SPEC message. */
        distCount = mothershipPayloadsIt->second.size() + 1;  /* +1 for SUPD */
        appNumber = 0;  /* This is terrible - only one graph instance can be
                         * loaded at a time! <!> TODO */
        specMessage.Put<unsigned>(1, &distCount);
        specMessage.Put<unsigned char>(2,
            static_cast<unsigned char*>(&appNumber));
        specMessage.Send();

        /* Customise and send the SUPD message. */
        soPath = gi->pSup->binPath;
        supdMessage.Put(1, &soPath);
        supdMessage.Send();

        /* Customise and send the DIST messages (one per core) */
        for (payloadIt = mothershipPayloadsIt->second.begin();
             payloadIt != mothershipPayloadsIt->second.end(); payloadIt++)
        {
            distMessage.Put(1, &(payloadIt->codePath));
            distMessage.Put(2, &(payloadIt->dataPath));
            distMessage.Put<unsigned>(3, &(payloadIt->coreAddr));
            distMessage.Put<unsigned>
                (4, &(payloadIt->threadsExpected));
            distMessage.Send();

        }
    }

    gi->deployed = true;

    return 0;
}

//------------------------------------------------------------------------------

void CmBuil::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sCmBuil +++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
if (par==0) fprintf(fp,"%sOrchBase parent not defined\n",os);
else fprintf(fp,"%sOrchbase parent : %s\n",os,par->FullName().c_str());
fprintf(fp,"%sCmBuil -------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void CmBuil::Show(FILE * fp)
{
fprintf(fp,"\nBuilder attributes and state:\n");
fprintf(fp,"\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmBuil::operator()(Cli * pC)
// Handle "buil(d)" command from the monkey.
{
//printf("CmPath_t operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause string
  string sCo = pC->Co;                 // Pull out command string
  string sPa = (*i).GetP();            // Pull out (simple) parameter
  if (sCl=="app" ) { par->Post(247,sCo,sCl,sPa); continue; }
  if (sCl=="depl") { Cm_Deploy(*i);              continue; }
  if (sCl=="init" or sCl=="run" or sCl=="stop" or sCl=="recl")
                   { Cm_Do(*i, sCl);             continue; }
  par->Post(25,sCl,"build");           // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================

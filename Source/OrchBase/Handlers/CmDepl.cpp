//------------------------------------------------------------------------------

#include "CmDepl.h"
#include "Pglobals.h"
#include "Root.h"  /* Grabs Orchestrator config, and OrchBase. */
#include "SupervisorModes.h"

//==============================================================================

CmDepl::CmDepl(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmDepl::~CmDepl()
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

void CmDepl::Cm_App(Cli::Cl_t clause)
{
    std::set<GraphI_t*> graphs;
    std::set<GraphI_t*>::iterator graphIt;

    /* Get the application graph instances */
    par->GetGraphIs(clause, graphs);

    /* Ensure that all graph instances are built before proceeding. */
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        if (!(*graphIt)->built)
        {
            par->Post(107, (*graphIt)->GetCompoundName());
            return;
        }
    }

    /* Deploy each graph instance in sequence, failing fast. */
    for (graphIt = graphs.begin(); graphIt != graphs.end(); graphIt++)
    {
        /* Sanity */
        if (par->deplStat[(*graphIt)->GetCompoundName()] == "ERROR")
        {
            fprintf(par->fd, "ERROR: Unable to deploy graph instance '%s' - "
                    "it is in an error state from a previously-failed "
                    "command. Aborting.\n", (*graphIt)->Name().c_str());
            par->Post(185, (*graphIt)->Name());
            continue;
        }

        else if (par->deplStat[(*graphIt)->GetCompoundName()] == "RECALLING")
        {
            fprintf(par->fd, "ERROR: Unable to deploy graph instance '%s' - "
                    "it is still being recalled.\n",
                    (*graphIt)->Name().c_str());
            par->Post(185, (*graphIt)->Name());
            continue;
        }

        if (par->deplStat[(*graphIt)->GetCompoundName()] != "")
        {
            fprintf(par->fd, "Unable to deploy graph instance '%s' - it is "
                    "already deployed! Aborting.\n",
                    (*graphIt)->Name().c_str());
            par->Post(185, (*graphIt)->Name());
            continue;
        }

        /* Let's try it. */
        fprintf(par->fd, "Staging deployment of graph instance '%s'...\n",
                (*graphIt)->Name().c_str());  /* Microlog */
        if (DeployGraph(*graphIt) != 0)
        {
            /* Failure, clear deployment information. */
            par->deplInfo[(*graphIt)->GetCompoundName()].clear();
            par->Post(185, (*graphIt)->Name());
            return;
        }

        fprintf(par->fd, "Deployment of graph instance '%s' staged. Wait for "
                "Mothership(s) to acknowledge receipt (they will Post).\n\n",
                (*graphIt)->Name().c_str());
        par->Post(184, (*graphIt)->Name());  /* We good, yo. */
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
int CmDepl::DeployGraph(GraphI_t* gi)
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
    std::string graphName = gi->GetCompoundName();
    std::string graphPathName = gi->GetCompoundName(true);
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
#if SINGLE_SUPERVISOR_MODE
        /* Grab the only Mothership (which may be invalid). We don't exit
         * immediately if there is no Mothership (we fail later). */
        mothershipProc = par->loneMothership;
#else
        /* Grab the Mothership for this box (which may be invalid). We don't
         * exit if we find an invalid entry - there may not be any devices for
         * this graph instance mapped to the box in question. */
        mothershipProc = par->P_SCMm2[boxIt->second];
#endif

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
                fprintf(par->fd, "ERROR: Not enough Motherships to deploy "
                        "this application.\n");
                par->Post(178, graphName);
                return 1;
            }
            else rank = mothershipProc->P_rank;
            fprintf(par->fd, "Selecting Mothership at MPI rank %d as a "
                    "deployment target.\n", rank);

            /* Store this process in a persistent deployment information
             * object. */
            par->deplInfo[gi->GetCompoundName()].push_back(mothershipProc);
            par->deplStat[gi->GetCompoundName()] = "DEPLOYING/ED";

            /* Define the payload for a DIST message for this core. */
            mothershipPayloads[rank].push_back(DistPayload());
            payload = &(mothershipPayloads[rank].back());

            /* paths */
            payload->codePath = std::string(getenv("HOME")) + "/" +
                par->pCmPath->pathMshp + graphPathName + "/" +
                core->instructionBinary;
            payload->dataPath = std::string(getenv("HOME")) + "/" +
                par->pCmPath->pathMshp + graphPathName + "/" +
                core->dataBinary;

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
                         par->pCmPath->pathMshp.c_str(),
                         graphPathName.c_str());
        sourceBinaries = dformat("%s%s/bin/*",
                                 par->pCmPath->pathBina.c_str(),
                                 graphPathName.c_str());

        /* Identify whether or not this Mothership is running on the same
         * machine as Root, to determine how we deploy binaries. Store the
         * commands-to-be-run in a vector. */
        if (rootMachineName == mothershipProc->P_proc)
        {
            fprintf(par->fd, "The Mothership at rank %d is running on the "
                    "same box as the Root process.\n", rank);
            commands.push_back(dformat("rm -r -f %s", target.c_str()));
            commands.push_back(dformat("mkdir -p %s", target.c_str()));
            commands.push_back(dformat("cp -r %s %s", sourceBinaries.c_str(),
                                       target.c_str()));
            fprintf(par->fd, "Copying binaries from '%s' to '%s' on the local "
                    "filesystem...\n", sourceBinaries.c_str(), target.c_str());
        }

        /* Note that, if the machine is different, we deploy binaries using
         * SCP. */
        else
        {
            fprintf(par->fd, "The Mothership at rank %d is running a "
                    "different box from the Root process.\n", rank);
            host = dformat("%s@%s", mothershipProc->P_user,
                           mothershipProc->P_proc);
            commands.push_back(dformat("ssh %s \"rm -r -f %s\"",
                                       host.c_str(), target.c_str()));
            commands.push_back(dformat("ssh %s \"mkdir -p %s\"",
                                       host.c_str(), target.c_str()));
            commands.push_back(dformat("scp -r %s %s:%s",
                                       sourceBinaries.c_str(), host.c_str(),
                                       target.c_str()));
            fprintf(par->fd, "Copying binaries from '%s' to '%s' using "
                    "SSH...\n ", sourceBinaries.c_str(), target.c_str());
        }

        /* Run each staged command, failing fast if one of them breaks. */
        for (command = commands.begin(); command != commands.end(); command++)
        {
            if (system(command->c_str()) > 0)
            {
                /* Command failed, cancelling deployment. */
                fprintf(par->fd, "ERROR: Command '%s' failed, cancelling "
                        "deployment.\n", command->c_str());
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
        fprintf(par->fd, "Binaries copied successfully.\n");
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
        appNumber = 0;  /* This is terrible - only one graph instance can be
                         * loaded at a time! <!> TODO */
        distCount = mothershipPayloadsIt->second.size() + 1;  /* +1 for SUPD */
        specMessage.Put<unsigned>(1, &distCount);
        specMessage.Put<unsigned char>(2,
            static_cast<unsigned char*>(&appNumber));
        bool soloApp = dynamic_cast<Root *>(par)->pOC->SingleApp();
        specMessage.Put<bool>(3,&soloApp);
        fprintf(par->fd, "Sending SPEC message to Mothership rank %d, with "
                "appNumber=%u, distCount=%u, and soloApp=%s...",
                mothershipPayloadsIt->first, appNumber, distCount,
                soloApp ? "true" : "false");
        specMessage.Send();
        fprintf(par->fd, " message sent.\n");

        /* Customise and send the SUPD message. */
        soPath = getenv("HOME") + std::string("/") +  par->pCmPath->pathMshp +
            graphPathName + "/" + gi->pSupI->binPath;
        supdMessage.Put(1, &soPath);
        fprintf(par->fd, "Sending SUPD message to Mothership rank %d, with "
                "soPath=%s...",
                mothershipPayloadsIt->first, soPath.c_str());
        supdMessage.Send();
        fprintf(par->fd, " message sent.\n");

        /* Customise and send the DIST messages (one per core) */
        for (payloadIt = mothershipPayloadsIt->second.begin();
             payloadIt != mothershipPayloadsIt->second.end(); payloadIt++)
        {
            distMessage.Put(1, &(payloadIt->codePath));
            distMessage.Put(2, &(payloadIt->dataPath));
            distMessage.Put<unsigned>(3, &(payloadIt->coreAddr));
            distMessage.Put<unsigned>
                (4, &(payloadIt->threadsExpected));

            /* Microlog, for the people. */
            fprintf(par->fd, "Sending DIST message to Mothership rank %d, "
                    "with codePath=%s, dataPath=%s, coreAddr=%u, and "
                    "threadsExpected=[",
                    mothershipPayloadsIt->first, payloadIt->codePath.c_str(),
                    payloadIt->dataPath.c_str(), payloadIt->coreAddr);
            std::vector<AddressComponent>::iterator threadIt;
            for (threadIt = payloadIt->threadsExpected.begin();
                 threadIt != payloadIt->threadsExpected.end(); threadIt++)
            {
                fprintf(par->fd, "%s%x",
                        threadIt != payloadIt->threadsExpected.begin() \
                            ? ", " : "",
                        *threadIt);
            }
            fprintf(par->fd, "] (size=%lu)...",
                    payloadIt->threadsExpected.size());

            /* And away we go. */
            distMessage.Send();
            fprintf(par->fd, " message sent.\n");
        }
    }

    gi->deployed = true;

    return 0;
}

//------------------------------------------------------------------------------

void CmDepl::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sCmDepl +++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
if (par==0) fprintf(fp,"%sOrchBase parent not defined\n",os);
else fprintf(fp,"%sOrchbase parent : %s\n",os,par->FullName().c_str());
fprintf(fp,"%sCmDepl -------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmDepl::operator()(Cli * pC)
// Handle "depl(oy)" command from the monkey.
{
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause string
  string sCo = pC->Co;                 // Pull out command string
  string sPa = (*i).GetP();            // Pull out (simple) parameter
  if (sCl=="app" ) { Cm_App(*i); continue; }
  par->Post(25,sCl,"deploy");          // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================

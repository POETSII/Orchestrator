struct DistPayload
{
    std::string codePath;
    std::string dataPath;
    AddressComponent coreAddr;
    std::vector<AddressComponent> threadsExpected;
}

/* Deploys a task to Motherships.
 *
 * If no task name is defined in the command-line argument structure, the first
 * task is used by default.
 *
 * This involves two stages:
 *
 *  - Deploying the binaries to the filesystems on which the Motherships
 *    operate.
 *
 *  - Sending messages (MPI) to each relevant Mothership process describing the
 *    application (see the Mothership documentation).
 */
void OrchBase::TaskDeploy(Cli::Cl_t Cl)
{
    std::map<std::string, P_task*>::iterator taskFinder;
    std::string taskName;
    P_task* task;

    /* Iteration through the process map, to find Motherships. */
    unsigned commIndex;
    std::vector<ProcMap::ProcMap_t>::iterator procIt;
    int rank;

    /* Finding the machine name of Root. */
    std::vector<ProcMap::ProcMap_t>::iterator rootFinder;
    std::string rootMachineName;

    /* Iteration through the hardware model with respect to boxes. */
    std::map<AddressComponent, P_box*>::iterator boxIt;
    std::vector<P_board*>::iterator boardIt;
    std::map<AddressComponent, P_core*>::iterator coreIt;
    std::map<AddressComponent, P_thread*>::iterator threadIt;
    P_core* core;
    P_thread* thread;

    /* Staging area for DIST message payloads, keyed by Mothership process
     * communicator and rank. */
    std::map<std::pair<int, int>, std::vector<DistPayload> >
        mothershipPayloads;
    std::map<std::pair<int, int>, std::vector<DistPayload> >::iterator
        mothershipPayloadsIt;
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
    unsigned distCount;
    unsigned appNumber;
    std::string soPath;

    /* The task must exist. */
    if (Cl.Pa_v.size() > 1)
    {
        Post(47, Cl.Cl, "task", "1");
        return;
    }

    if (!P_taskm.size())
    {
        Post(107, "definitions");
        return;
    }

    /* Grab the name from the input argument, and the task object address from
     * the name. */
    if (Cl.Pa_v.size())
    {
        taskName = Cl.Pa_v[0].Val;
        taskFinder = P_taskm.find(taskName);

        /* If a task name is provided, complain if it can't be found. */
        if (taskFinder == P_taskm.end())
        {
            Post(107, taskName);
            return;
        }
    }

    /* By default (if no arguments are passed), use the first task by
     * default. */
    else
    {
        taskFinder = P_taskm.begin();
        taskName = taskFinder->first;
    }
    task = taskFinder->second;

    /* Ensure the task has been placed before proceeding. */
    if (!task->linked)
    {
        Post(157, taskName);
        return;
    }

    /* Identify the name of the machine on which Root is running. */
    rootFinder = pPmap[RootCIdx()]->vPmap.begin();
    while (rootFinder->P_rank != pPmap[RootIndex]->U.Root) rootFinder++;
    rootMachineName = rootFinder->P_proc;

    /* Iterate over each box in the hardware model - this allows us to bind
     * boxes to Motherships (implicitly), which also going through all of the
     * cores in the hardware model that might be relevant for the task that we
     * are deploying. */
    commIndex = 0;
    procIt = pPmap[commIndex]->vPmap.begin();
    for (boxIt = pE->Pboxm.begin(); boxIt != pE->Pbox_m.end(); boxIt++)
    {
        /* Find the next available Mothership across all communicators. We need
         * the rank in order to store entries in the 'mothershipPayloads'
         * map. */
        rank = -1;  /* Set to something positive when a Mothership is found. */
        while (commIndex < Comms.size())
        {
            /* Find the next available Mothership in this communicator. */
            while (procIt != pPmap[commIndex]->vPmap.end() and
                   procIt->P_class != csMOTHERSHIPproc) procIt++;

            /* If we found one, leave the loop (usual case). Otherwise, search
             * the next communicator.*/
            if (procIt != pPmap[cIdx]->vPmap.end())
            {
                rank = procIt->P_rank;
                break;
            }
            else commIndex++;
        }

        /* Iterate over all cores in this box, in an attempt to find devices
         * owned by the task that are mapped to this box. Squashed indentation
         * to make logic easier to follow. */
        for (boardIt = boxIt->P_boardv.begin();
             boardIt != boxIt->boxIt->P_boardv.end(); boardIt++)
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
             * devices placed on it are owned by a different task. Recall that
             * all devices within a core service the same task, and threads are
             * loaded in a bucket-filled manner (for now). */
            if (thread->P_devicel.empty() or
                thread->P_devicel.front()->par->par != task) continue;

            /* If we couldn't find a Mothership for this box earlier, we panic
             * here, because we can't deploy this task without enough
             * Motherships to support it. This happens because there are more
             * boxes in the Engine than there are Mothership processes
             * running. We leave in this case.
             *
             * NB: We don't leave as soon as we discover that there aren't
             * enough Motherships, because it's possible that the extra boxes
             * have nothing relevant placed on them. In that case, the task can
             * still execute as expected. */
            if (rank == -1)
            {
                Post(166, taskName);
                return;
            }

            /* Define the payload for a DIST message for this core. */
            payload = &(mothershipPayloads[std::make_pair<int, int>
                                           (commIndex, rank)])

            /* paths */
            payload->codePath = core->instructionBinary;
            payload->dataPath = core->dataBinary;

            /* coreAddr */
            payload->coreAddr = core->get_hardware_address()->as_uint();

            /* threadsExpected */
            for (threadIt = core->P_threadm.begin();
                 threadIt != core->P_threadm.end(); threadIt++)
            {
                payload->threadsExpected.push_back(
                    threadIt->second->get_hardware_address()->as_uint());
            }
        }

        /* If we found no devices for this task on this box, skip to the next
         * box. */
        if (mothershipPayloads.find(std::make_pair<int, int>(commIndex, rank)
                                    == mothershipPayloads.end()))
            continue;

        /* At this point, we are sure that there is a Mothership that can
         * represent this box, and we are also sure that there are devices that
         * this Mothership needs to supervise. Now we are going to deploy
         * binaries to appropriate locations for the Mothership to find
         * them. To do this, we naively copy all binaries to all Motherships
         * for now. */
        target = dformat("/home/%s/%s/%s", procIt->P_user, TASK_DEPLOY_DIR,
                         taskName);
        sourceBinaries = dformat("%s/%s/*", taskpath + taskName, BIN_PATH);

        /* Identify whether or not this Mothership is running on the same
         * machine as Root, to determine how we deploy binaries. Store the
         * commands-to-be-run in a vector. */
        if (rootMachineName == procIt->P_proc)
        {
            commands.push_back(dformat("rm -r -f %s", target));
            commands.push_back(dformat("mkdir -p %s", target));
            commands.push_back(dformat("cp -r %s %s", sourceBinaries, target));
        }

        /* Note that, if the machine is different, we deploy binaries using
         * SCP. */
        else
        {
            host = dformat("%s@%s", procIt->P_user, procIt->P_proc);
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
                    Post(165, command->c_str());
                }
                else
                {
                    Post(167, command->c_str(),
                         POETS::getSysErrorString(errno).c_str());
                }
                return;
            }
        }
    }

    /* Send SPEC, DIST, and SUPD messages to each Mothership for this
     * task. Order does not matter. */

    /* Prepare common behaviour for all messages of their type. */
    messages.push_back(&specMessage);
    messages.push_back(&distMessage);
    messages.push_back(&supdMessage);
    for (messageIt = messages.begin(); messageIt != messages.end();
         messageIt++)
    {
        messageIt->Src(Urank);
        messageIt->Put<std::string>(0, &taskName);
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
            messageIt->comm(Comms[mothershipPayloadsIt->first->first]);
            messageIt->Tgt(mothershipPayloadsIt->first->second);
        }

        /* Customise and send the SPEC message. */
        distCount = mothershipPayloadsIt->second.size() + 1;  /* +1 for SUPD */
        appNumber = 0;  /* This is terrible - only one application can be
                         * loaded at a time! <!> TODO */
        specMessage.Put<unsigned>(1, &distCount);
        specMessage.Put<unsigned>(2, &appNumber);
        specMessage.Send();

        /* Customise and send the SUPD message. */
        soPath = task->pSup.binPath;
        supdMessage.Put<std::string>(1, &soPath);
        supdMessage.Send();

        /* Customise and send the DIST messages (one per core) */
        for (payloadIt = mothershipPayloadsIt->second.begin();
             payloadIt != mothershipPayloadsIt->second.end(); payloadIt++)
        {
            distMessage.Put<std::string>(1, &(payloadIt->codePath));
            distMessage.Put<std::string>(2, &(payloadIt->dataPath));
            distMessage.Put<unsigned>(3, &(payloadIt->coreAddr));
            distMessage.Put<std::vector<unsigned> >
                (4, &(payloadIt->threadsExpected));
            distMessage.Send();

        }
    }
}

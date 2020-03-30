//
// This is NOT a translation unit. Do not #include stuff here.
//
//==============================================================================

struct DistPayload
{
    std::string codePath;
    std::string dataPath;
    AddressComponent coreAddr;
    std::vector<AddressComponent> threadsExpected;
};

/* Constructs the bimap of mothership processes to boxes in the engine
 * (OrchBase.P_SCMm2). */
void OrchBase::BuildMshipMap()
{
    unsigned commIndex;
    std::vector<ProcMap::ProcMap_t>::iterator procIt;
    std::map<AddressComponent, P_box*>::iterator boxIt;
    bool foundAMothershipForThisBox

    /* Start from the first process on the first communicator. */
    commIndex = 0;
    procIt = pPmap[commIndex]->vPmap.begin();

    /* Iterate over each box in the hardware model. */
    for (boxIt = pE->Pboxm.begin(); boxIt != pE->Pbox_m.end(); boxIt++)
    {
        /* Find the next available Mothership across all communicators. We need
         * the rank in order to store entries in the 'mothershipPayloads'
         * map. */
        foundAMothershipForThisBox = false;
        while (commIndex < Comms.size())
        {
            /* Find the next available Mothership in this communicator. */
            while (procIt != pPmap[commIndex]->vPmap.end() and
                   procIt->P_class != csMOTHERSHIPproc) procIt++;

            /* If we found one, leave the loop (usual case). Otherwise, search
             * the next communicator.*/
            if (procIt != pPmap[cIdx]->vPmap.end())
            {
                P_SCMm2.Add(boxIt->second,
                            std::make_pair<unsigned, ProcMap::ProcMap_t*>
                            (commIndex, &*procIt));
                foundAMothershipForThisBox = true;
                break;
            }
            else commIndex++;
        }

        /* If we didn't find a Mothership for this box, map the box to 0, NULL
         * and warn loudly. */
        if (!foundAMothershipForThisBox)
        {
            P_SCMm2.Add(boxIt->second,
                        std::make_pair<unsigned, ProcMap::ProcMap_t*>
                        (0, PNULL));
            Post(168, boxIt->Name().c_str());
        }
    }
}

//------------------------------------------------------------------------------

void OrchBase::ClearDcls()
// Clear out all the graph declaration blocks that are not used (i.e. have no
// corresponding graph instance)
{
vector<string> dclsToDelete;           // need to build a list of declares to delete
WALKMAP(string,P_typdcl *,P_typdclm,i){// Walk the declaration blocks
  P_typdcl * p = (*i).second;          // Back reference vector empty?
  if (p->P_taskl.empty()) {            // If so, no-one wants this declare block
    delete p;
    dclsToDelete.push_back((*i).first);
  }
}
WALKVECTOR(string, dclsToDelete, d)    // get rid of the declares
  P_typdclm.erase(*d);
}

//------------------------------------------------------------------------------

void OrchBase::ClearDcls(string sb)
// Clear out a named declare block PLUS any instances that depend upon it.
// There exists a pathology here: if an instance that is cleared also depends
// on a declare block that has itself no other dependencies, that declare block
// will be left hanging (orphaned). The monkey can get it with a /clear = !,
// so just flag it as a warning.
{
                                       // Is it there at all?
if (P_typdclm.find(sb)==P_typdclm.end()) {
  Post(108,sb);                        // Hmph. Not there.
  return;
}
P_typdcl * p = P_typdclm[sb];          // Dereference the named declare block
if (p==0) {                            // Paranoia
  Post(903,sb);
  return;
}
                                       // Clear out dependent graphs
WALKLIST(P_task *,p->P_taskl,i) ClearTasks((*i)->Name());
delete p;                              // Clear the named block
P_typdclm.erase(sb);                   // Remove from map

WALKMAP(string,P_typdcl *,P_typdclm,i){// Look for orphaned declare blocks
  P_typdcl * p = (*i).second;
  if (p->P_taskl.empty()) Post(106,p->Name());
}

}

//------------------------------------------------------------------------------

void OrchBase::ClearTasks()
// Destroy the entire task section of the database. (This is the easy one)
// The code is different to the clear-individual-tasks because it's faster
{

// Walk the hardware, killing all the links into the task structures
if (pE != 0)
{
    // Walk the boxes and remove the supervisor links therein.
    WALKMAP(AddressComponent, P_box*, pE->P_boxm, boxIterator)
    {
        boxIterator->second->P_superv.clear();

        // Walk the boards and remove the supervisor links therein.
        WALKVECTOR(P_board*, boxIterator->second->P_boardv, boardIterator)
        {
            (*boardIterator)->sup_offv.clear();

            // Walk the cores and clear all binaries therein, if any are
            // loaded.
            WALKPDIGRAPHNODES(AddressComponent, P_mailbox*,
                              unsigned, P_link*,
                              unsigned, P_port*, (*boardIterator)->G,
                              mailboxIterator)
            {
                WALKMAP(AddressComponent, P_core*,
                        (*boardIterator)->G.NodeData(mailboxIterator)->P_corem,
                        coreIterator)
                {
                    coreIterator->second->clear_binaries();

                    // Walk the threads and remove the device links.
                    WALKMAP(AddressComponent, P_thread*,
                            coreIterator->second->P_threadm, threadIterator)
                    {
                        threadIterator->second->P_devicel.clear();
                    }
                }
            }
        }
    }
}

// Kill the supervisors
WALKMAP(string,P_super *,P_superm,i) delete (*i).second;
P_superm.clear();

// Kill the tasks
WALKMAP(string,P_task *,P_taskm,i) delete (*i).second;
P_taskm.clear();

// Kill the declares
WALKMAP(string,P_typdcl *,P_typdclm,i) delete (*i).second;
P_typdclm.clear();

// Kill the xml import
pB->Clear();

}

//------------------------------------------------------------------------------

void OrchBase::ClearTasks(char tt)
// Clear either all monkey tasks or all POL tasks
{
                                       // Enumerated types?
                                       // Hah! We spit on enumerated types.
if ((tt!='T')&&(tt!='P')) Post(902,string(tt,1));
vector<string> tasksToDelete;          // have to build a list of tasks to delete
WALKMAP(string,P_task *,P_taskm,i) {
  P_task * p = (*i).second;            // Get the task pointer
  char t = p->IsPoL() ? 'P' : 'T';     // Task or POL?
  if (t==tt) tasksToDelete.push_back((*i).first); // add matching type to the delete list
}
WALKVECTOR(string,tasksToDelete,k)     // now traverse the delete list
  ClearTasks(*k);                      // Kill the task

}

//------------------------------------------------------------------------------

void OrchBase::ClearTasks(string st)
// Clear out a specific named task. If a declare block is consequently orphaned,
// issue a warning
{
if (P_taskm.find(st)==P_taskm.end()) { // Is it there at all?
  Post(105,st);                        // Hmph. Not there.
  return;
}
P_task * p = P_taskm[st];              // Dereference the named task
if (p==0) {                            // Paranoia
  Post(904,st);
  return;
}
                                       // OK, the task pointer is valid....
if (p->pD!=0) WALKPDIGRAPHNODES
  (unsigned,P_device *,unsigned,P_message *,unsigned,P_pin *,p->pD->G,i) {
  if (i==p->pD->G.NodeEnd()) break;
  P_device * pd = p->pD->G.NodeData(i);// Device
                                       // Now disconnect from the hardware
  P_thread * pt = pd->pP_thread;       // Corresponding thread (if any)
  if (pt!=0)                           // If there's a thread.....
    WALKLIST(P_device *,pt->P_devicel,it)      // Look for the backlink...
      if ((*it)==pd)
      {
         pt->P_devicel.erase(it);      // ..and remove it
         break;
      }
}
                                       // Now disconnect from the declare block
P_typdcl * pdcl = p->pP_typdcl;        // This is the declare block
if (pdcl!=0)
{
  WALKLIST(P_task *,pdcl->P_taskl,i)   // Look for the back pointer...
    if ((*i)==p)
    {
       pdcl->P_taskl.erase(i);         // ...and remove it
       break;
    }
}
if (p->filename.length() > 0) pB->Clear(p); // clear out xml import
delete p;                              // Remove the task itself
P_taskm.erase(st);                     // Remove pointer from map

}

//------------------------------------------------------------------------------

unsigned OrchBase::CmTask(Cli * pC)
// Handle task command from monkey. It's already been trimmed.
{
if (pC->Cl_v.empty()) return 0;        // Nothing to do
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {
  const char * cs = (*i).Cl.c_str();
  if (strcmp(cs,"geom")==0) { TaskGeom(*i);  continue; }
  if (strcmp(cs,"load")==0) { TaskLoad(*i);  continue; }
  if (strcmp(cs,"buil")==0) { TaskBuild(*i); continue; }
  if (strcmp(cs,"depl")==0) { TaskDeploy(*i);continue; }
  if (strcmp(cs,"reca")==0) { TaskMCmd(*i,"recl");  continue; }
  if (strcmp(cs,"path")==0) { TaskPath(*i);  continue; }
  if (strcmp(cs,"pol" )==0) { TaskPol(*i);   continue; }
  if (strcmp(cs,"init")==0) { TaskMCmd(*i,"init"); continue; }
  if (strcmp(cs,"clea")==0) { TaskClear(*i); continue; }
  if (strcmp(cs,"run" )==0) { TaskMCmd(*i,"run");   continue; }
  if (strcmp(cs,"stop")==0) { TaskMCmd(*i,"stop");  continue; }
  if (strcmp(cs,"show")==0) { TaskShow(*i);  continue; }
  if (strcmp(cs,"dump")==0) { TaskDump(*i);  continue; }
  Post(25,(*i).Cl,"task");
}
return 0;
}

//------------------------------------------------------------------------------

void OrchBase::TaskClear(Cli::Cl_t Cl)
// Routine to remove a named task from the system.
// If it's there, we need to unlink it (easy), then remove the task from the
// task map.
// If the name is "*", remove all the user tasks (leave declares alone)
// If the name is "+", remove all the POL tasks  (leave declares alone)
// If the name is "**", remove all tasks AND hose the declares.
{
if (Cl.Pa_v.empty()) return;           // Nothing to do

WALKVECTOR(Cli::Pa_t,Cl.Pa_v,i) {      // Loop through things to remove
  string st = (*i).Val;                // Thing to remove
  string ost = (*i).Op;                // Task or dcl block?
                                       // Clear all monkey tasks
  if (st=="*" ) { ClearTasks('T'); continue; }
                                       // Clear all POL tasks
  if (st=="+" ) { ClearTasks('P'); continue; }
                                       // Clear everything
  if (st=="**") { ClearTasks();    continue; }
                                       // Clear all unused dcl blocks
  if (st=="!" ) { ClearDcls();     continue; }
                                       // Clear a named dcl block + dependencies
  if (ost=="!") { ClearDcls(st);   continue; }
                                       // Clear a specific named task.
  ClearTasks(st);
}

}

//------------------------------------------------------------------------------

void OrchBase::TaskDump(Cli::Cl_t Cl)
{
FILE * fp = stdout;                    // Default is terminal
string ufs = Cl.GetP(0);               // User supply filename?
if (!ufs.empty()) {
  ufs = taskpath + ufs;                // Bolt default extension on the front
  FILE * tfp = fopen(ufs.c_str(),"w"); // Try and open it
  if (tfp!=0) fp = tfp;
  else Post(109,ufs);
}
                                       // Just the task structures
if (P_taskm.empty()) fprintf(fp,"Task map empty\n");
else WALKMAP(string,P_task *,P_taskm,i) (*i).second->Dump(fp);
if (P_typdclm.empty()) fprintf(fp,"Type declaration map empty\n");
else WALKMAP(string,P_typdcl *,P_typdclm,i) (*i).second->Dump(fp);
if (fp!=stdout) fclose(fp);
}

//------------------------------------------------------------------------------

void OrchBase::TaskGeom(Cli::Cl_t Cl)
// Routine to decorate an existing task with geometry.
// For use in generating pretty pictures.....
// If it's a user task, it'll be generic P&R; if it's PoL, it'll be task-type
// specific, so there will be parameters....
{
string name = Cl.GetP(0);              // Unpack task name
if (name.empty()) {                    // Make sense?
  Post(48,Cl.Cl,"task /geometry","1");
  return;
}
if (P_taskm.find(name)==P_taskm.end()){// Is it there?
  Post(107,name);
  return;
}

// By some - currently opaque - method, do it....

}

//------------------------------------------------------------------------------

void OrchBase::TaskLoad(Cli::Cl_t Cl)
// Routine to load a task definition from a file. Task definitions may be split
// over several files; the arguments of this command are
// task name - that may or may not already exist in the database
// filename  - XML file from HbD that contains more task graph definition
{
if (Cl.Pa_v.size()!=1) {               // Command make sense?
  Post(47,Cl.Cl,"task","1");
  return;
}
string file = Cl.Pa_v[0].Val;          // Unpack file name
file = taskpath + file;                // Add default full file path
FileName Fn(file);                     // Make sure it's a semantic filename
if (Fn.Err()) Post(104,file);          // If not, warn the monkey

//P_task * pT = new P_task(this,name);   // Good to go, then: create a new task
//P_taskm[name] = pT;                    // Attach it to database
//pT->PoL.IsPoL = false;                 // It's a real task
//pT->filename = Cl.Pa_v[1].Val;         // Unpack task source file
//pB->Build(pT);                         // Build the thing

//string file = Cl.Pa_v[1].Val;          // Unpack file name
//file = taskpath + file;                // Add default full file path
//FileName Fn(file);                     // Make sure it's a semantic filename
//if (Fn.Err()) Post(104,file);          // If not, warn the monkey
// pB->Load(name, file);
// P_task * pT = new P_task(this,name);   // Good to go, then: create a new task
// P_taskm[name] = pT;                    // Attach it to database
// pT->PoL.IsPoL = false;                 // It's a real task
// pT->filename = Cl.Pa_v[1].Val;         // Unpack task source file
// pB->Build(pT);                         // Build the thing
pB->Load(file);                           // Parse the xml description of the task
//pB->Build();                            // Build the thing
}

//------------------------------------------------------------------------------

void OrchBase::TaskBuild(Cli::Cl_t Cl)
{
// Routine to build a task - that is, generate the source files and compile the
// binaries. Argument is the task name, which needs to exist. If the task has
// not been placed, the builder will interpret this as a command to build sources
// and binaries for a virtual board.
if (Cl.Pa_v.size()>1) {                       // Command make sense?
  Post(47,Cl.Cl,"task","1");
  return;
}
if (!P_taskm.size()){                          // Some tasks exist?
  Post(107,"definitions");
  return;
}
map<string, P_task*>::iterator task = P_taskm.begin(); // Default to first task
if (Cl.Pa_v.size()){
string name = Cl.Pa_v[0].Val;                 // Unpack task name
if ((task=P_taskm.find(name))==P_taskm.end()){// Is the task there?
  Post(107,name);
  return;
}
}
pB->Build(task->second);                      // Build the thing
}

//------------------------------------------------------------------------------

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

    /* Finding the machine name of Root. */
    std::vector<ProcMap::ProcMap_t>::iterator rootFinder;
    std::string rootMachineName;

    /* Holding Mothership process information. */
    unsigned commIndex;
    ProcMap::ProcMap_t* mothershipProc;

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

    /* Iterate over each box in the hardware model */
    for (boxIt = pE->Pboxm.begin(); boxIt != pE->Pbox_m.end(); boxIt++)
    {
        /* Grab the Mothership for this box (which may be invalid). We don't
         * exit if we find an invalid entry - there may not be any devices for
         * this task mapped to the box in question. */
        commIndex = P_SCMm2[boxIt->second]->first;
        mothershipProc = P_SCMm2[boxIt->second]->second;

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
            if (mothershipProc == PNULL)
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
        target = dformat("/home/%s/%s/%s", mothershipProc->P_user,
                         TASK_DEPLOY_DIR, taskName);
        sourceBinaries = dformat("%s/%s/*", taskpath + taskName, BIN_PATH);

        /* Identify whether or not this Mothership is running on the same
         * machine as Root, to determine how we deploy binaries. Store the
         * commands-to-be-run in a vector. */
        if (rootMachineName == mothershipProc->P_proc)
        {
            commands.push_back(dformat("rm -r -f %s", target));
            commands.push_back(dformat("mkdir -p %s", target));
            commands.push_back(dformat("cp -r %s %s", sourceBinaries, target));
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

//------------------------------------------------------------------------------

void OrchBase::TaskPath(Cli::Cl_t Cl)
// Change the default path extension for task files
{
Post(102,taskpath);                    // Tell the monkey the current path
if (Cl.Pa_v.empty()) return;           // Nothing to do - go without change
taskpath = Cl.Pa_v[0].Val;             // Update the path string
if (GetOS() == "Windows")
{
   if (taskpath[taskpath.size()-1] != '\\') taskpath += "\\"; // append a directory separator if none included
}
else
{
   if (taskpath[taskpath.size()-1] != '/') taskpath += "/";
}
Post(103,taskpath);                    // Tell the monkey
}

//------------------------------------------------------------------------------

void OrchBase::TaskPol(Cli::Cl_t Cl)
// Instantiate a new task as a proof-of-life
{
if (Cl.Pa_v.size()<=2) {               // Command make sense?
  Post(48,Cl.Cl,"task","2");
  return;
}
string name = Cl.Pa_v[0].Val;          // Unpack task name
if (P_taskm.find(name)!=P_taskm.end()){// Already one there?
  Post(49,name);
  return;
}
P_task * pT = new P_task(this,name);   // Create a new one
P_taskm[name] = pT;                    // Attach it to database
pT->PoL.IsPoL = true;                  // It's a PoL
pT->PoL.type = Cl.Pa_v[1].Val;         // Unpack task type
                                       // Unpack parameter strings
for (unsigned i=2;i<Cl.Pa_v.size();i++)pT->PoL.params.push_back(Cl.Pa_v[i].Val);

pTG->Build(pT);                        // Build the thing

}

//------------------------------------------------------------------------------

/* Sends a command, relating to a single task, to Motherships operating on that
 * task.
 *
 * If no task name is defined in the command-line argument structure, the first
 * task is used by default. */
void OrchBase::TaskMCmd(Cli::Cl_t Cl, std::string command)
{
    std::map<std::string, P_task*>::iterator taskFinder;
    std::string taskName;
    P_task* task;

    PMsg_p message;

    /* Boxes that are relevant for the application being commanded. */
    std::set<P_box*> boxesOfImport;
    std::set<P_box*>::iterator boxIt;

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

    /* Set up the message given the input arguments (catching an invalid
     * command input from somewhere). */
    if (command == "recl")
        message.Key(Q::CMND, Q::RECL);
    else if (command == "init")
        message.Key(Q::CMND, Q::INIT);
    else if (command == "run")
        Pkt.Key(Q::CMND,Q::RUN);
    else if (command == "stop")
        Pkt.Key(Q::CMND,Q::STOP);
    else
    {
        Post(25, command, "task");
        return;
    }
    message.Src(Urank);
    message.Put(0, &taskName);

    /* Get the set of important boxes. */
    pE->get_boxes_for_task(task, &boxesOfImport);

    /* For each box, send to the Mothership on that box. */
    for (boxIt = boxesOfImport.begin(); boxIt != boxesOfImport.end(); boxIt++)
    {
        message.comm = P_SCMm2[boxIt].first;
        message.Tgt(P_SCMm2[boxIt].second->P_rank);
        message.Send();
    }
}

//------------------------------------------------------------------------------

void OrchBase::TaskShow(Cli::Cl_t Cl)
// Pretty-print an overview of the task map
{
                                       // Get the output channel
FILE * fp = stdout;                    // Default is terminal
string ufs = Cl.GetP(0);               // User supply filename?
if (!ufs.empty()) {
  ufs = taskpath + ufs;                // Bolt default extension on the front
  FILE * tfp = fopen(ufs.c_str(),"w"); // Try and open it
  if (tfp!=0) fp = tfp;
  else Post(110,ufs);
}
                                       // And do it
fprintf(fp,"\nOrchestrator has %u tasks loaded:\n\n",
           static_cast<unsigned>(P_taskm.size()));
unsigned c=0;
fprintf(fp,"    |Task       |Supervisor |Linked   |Devices  |Channels |");
fprintf(fp,"Declare    |PoL? |PoL type   |Parameters    \n");
fprintf(fp,"    +-----------+-----------+---------+---------+---------+");
fprintf(fp,"-----------+-----+-----------+------------+----....\n");
WALKMAP(string,P_task *,P_taskm,i) {
  P_task * _2 = (*i).second;
  fprintf(fp,"%3u |%10s |%10s |%8s |%8u |%8u |%10s |",
          c++,(*i).first.c_str(),
          _2->pSup==0 ? "** none **" : _2->pSup->Name().c_str(),
          _2->linked ? "yes" : "no",
          _2->pD==0 ? 0 : _2->pD->G.SizeNodes(),
          _2->pD==0 ? 0 : _2->pD->G.SizeArcs(),
          _2->pP_typdcl==0 ? "** none **" : _2->pP_typdcl->Name().c_str());
  if (!_2->IsPoL()) {
    fprintf(fp,"User |           |%10s  |\n",_2->filename.c_str());
  }
  else {
    fprintf(fp,"PoL  |%10s |",_2->PoL.type.c_str());
    WALKVECTOR(string,_2->PoL.params,i) fprintf(fp," %10s |",(*i).c_str());
    fprintf(fp,"\n");
  }
}
fprintf(fp,"    +-----------+-----------+---------+---------+---------+");
fprintf(fp,"-----------+-----+-----------+------------+----....\n");
fprintf(fp,"Default display filepath ||%s||\n",taskpath.c_str());
fprintf(fp,"\n");
fflush(fp);
if (fp!=stdout) fclose(fp);
}

//==============================================================================

//
// This is NOT a translation unit. Do not #include stuff here.
//
//==============================================================================

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
  if (strcmp(cs,"reca")==0) { TaskRecall(*i);continue; }
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
// added additional dump parameters: summary (-s), NameServer (-n),
// task name (<task> || "*") 13 July 2019 ADR
string ufo = Cl.GetO(1);               // Look for summary operator "-"
string ufn = Cl.GetP(1);
if (!(ufo.empty() || ufo == "-"))      // reject other operators
{
   Post(25, ufo+ufs, "task /dump");
   return;  
}
bool summary = false;
bool dumpNs = false;
if (!ufn.empty())                      // look for dump qualifiers
{
   if (ufo == "-")                     // expect a summary with a "-" sign
   {
      if (ufn != "s")
      {
 	 Post(25, ufo+ufn, "task /dump"); // other switches are invalid
         return; 
      }
      summary = true;
      ufn = "*"; // summary always outputs all tasks
   }
   ufo = Cl.GetO(2);
   if (!(ufo.empty() || ufo == "-"))      // reject other operators
   {
      Post(25, ufo+Cl.GetP(2), "task /dump");
      return;  
   }
   if (ufo == "-")               // expect a NameServer dump with a "-" sign
   {
      if (Cl.GetP(2) != "n")     // look for the "-n"
      {
	 Post(25, ufo+Cl.GetP(2), "task /dump"); // other switches are invalid
         return; 
      }
      dumpNs = true;
   }
   
}
else ufn = "*";            // default to dumping all tasks.
map<string,P_task*>::iterator task = P_taskm.begin(); // may need to find a specific task
 
                                       // Just the task structures
if (P_taskm.empty()) fprintf(fp,"Task map empty\n");
else
{
   if (ufn == "*")          // all tasks
      WALKMAP(string,P_task *,P_taskm,i) i->second->Dump(fp);
   else                    // a named task given by parameter 1
   {
      task = P_taskm.find(ufn);
      if (task == P_taskm.end())
      {
	 Post(105,ufn);
	 fprintf(fp,"No such task: %s\n", ufn.c_str());
      }
      else task->second->Dump(fp);
   }
}
if (!summary) // print out type declares if not simply doing a summary
{
   if (P_typdclm.empty()) fprintf(fp,"Type declaration map empty\n");
   else
   {
      if (ufn == "*") // all type declarations 
         WALKMAP(string,P_typdcl *,P_typdclm,i) (*i).second->Dump(fp);
      else if (task != P_taskm.end()) // just the type declaration for a named task
      {
	 if (!task->second->pP_typdcl) // if it exists!
	 {
	    Post(113,ufn);
	    fprintf(fp,"Task %s has no type declarations\n", ufn.c_str());
	 }
	 else task->second->pP_typdcl->Dump(fp);
      }
   }
}
// close the file BEFORE sending to NameServer, so that if it happens to be on the
// same machine, the dumpfile won't have multiple open handles.
if (fp!=stdout) fclose(fp); 
if (dumpNs) // also dump NameServer info?
{
   int nsRank = Q::NAP; // search for NameServer
   MPI_Comm nsComm = MPI_COMM_NULL;
   for (unsigned comm = 0; comm < Comms.size(); comm++)
   {
       // find NameServer's comm
       if ((nsRank = pPmap[comm]->U.NameServer) != Q::NAP)
       {
          nsComm = Comms[comm];
          break;
       }
   }
   if (nsRank == Q::NAP)
   {
      Post(711); // No nameserver. This should be fatal 
      return;    // (need to return with a value from TaskDump)
   }
   if (nsComm == MPI_COMM_NULL)
   {
      Post(712); // Unknown comm for nameserver. VERY fatal
      return;    // (should abort)
   }
   PMsg_p nsDump(nsComm); // set up a dump message
   nsDump.Src(Urank);
   nsDump.Tgt(nsRank);
   // of the appropriate type (summary, all, or a named task)
   if (summary) nsDump.Key(Q::NAME,Q::DUMP,Q::LIST);
   if (ufn == "*") nsDump.Key(Q::NAME,Q::DUMP,Q::TASK,Q::ALL);
   else
   {
      nsDump.Key(Q::NAME,Q::DUMP,Q::TASK,Q::NM);
      nsDump.Zname(0,ufn);
   }
   nsDump.Zname(1,ufs); // NameServer should open the appropriate dumpfile
   nsDump.Send();       
}
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
string file = Cl.GetP(0);              // Unpack file name
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

// additions for NameServer/SBase 11 July 2019 ADR
int nsRank = Q::NAP;
MPI_Comm nsComm = MPI_COMM_NULL;
for (unsigned comm = 0; comm < Comms.size(); comm++)
{
    if ((nsRank = pPmap[comm]->U.NameServer) != Q::NAP)
    {
       nsComm = Comms[comm];
       break;
    }
}
if (nsRank == Q::NAP)
{
   Post(711); // No nameserver. This should be fatal 
   return;    // (need to return with a value from TaskLoad)
}
if (nsComm == MPI_COMM_NULL)
{
   Post(712); // Unknown comm for nameserver. VERY fatal
   return;    // (should abort)
}
PMsg_p nsTask(nsComm);
nsTask.Src(Urank);
WALKMAP(string, P_task*, P_taskm, task)
{
   if (task->second->state == Unknown)
   {
      nsTask.Key(Q::NAME,Q::DATA,Q::TASK);
      nsTask.Zname(0,task->first);
      nsTask.Zname(1,taskpath);
      nsTask.Zname(2,file);
      vector<string> msgs;
      WALKVECTOR(P_message*,task->second->pP_typdcl->P_messagev,msg) msgs.push_back((*msg)->Name());
      nsTask.PutX(0, &msgs);
      // in future should place attributes here.
      vector<string> attrs;
      attrs.push_back("_NO_ATTRIBUTE_");
      nsTask.PutX(1,&attrs);
      vector<unsigned long> counts;
      unsigned long deviceCount = 0;
      // tediously, we have to walk the graph because supervisors and externals
      // must be disambiguated.
      for (map<unsigned,pdigraph<unsigned,P_device *,unsigned,P_message *,unsigned,P_pin *>::node>::iterator dn = task->second->pD->G.index_n.begin(); dn != task->second->pD->G.index_n.end(); dn++)
           if (dn->second.data != task->second->pSup) ++deviceCount;
      counts.push_back(deviceCount);
      // it would be so much easier if we could do something like the following!
      // counts.push_back(task->second->pD->G.SizeNodes());
      counts.push_back(0); // number of externals. In future this will need to be distinguished.
      deviceCount = 0;
      nsTask.Put(2,&counts);
      nsTask.Send(nsRank);
      nsTask.Clear();
      nsTask.Src(Urank);
      nsTask.Key(Q::NAME,Q::DATA,Q::DEVT);
      nsTask.Zname(0,task->first);
      WALKVECTOR(P_devtyp*,task->second->pP_typdcl->P_devtypv,devType)
      {
	 nsTask.Zname(2,(*devType)->Name());
	 vector<unsigned>inMsgs;
	 vector<unsigned>outMsgs;
	 for (unsigned m = 0; m < task->second->pP_typdcl->P_messagev.size(); m++)
	 {
	   WALKVECTOR(P_pintyp*,(*devType)->P_pintypIv,iPin)
	   {
	      if ((*iPin)->pMsg == task->second->pP_typdcl->P_messagev[m])
	      {
                 inMsgs.push_back(m);
		 break;
	      }
	   }
	   WALKVECTOR(P_pintyp*,(*devType)->P_pintypOv,oPin)
	   {
	      if ((*oPin)->pMsg == task->second->pP_typdcl->P_messagev[m])
	      {
                 outMsgs.push_back(m);
		 break;
	      }
	   }
	 }
	 nsTask.Put(0,&inMsgs);
	 nsTask.Put(1,&outMsgs);
	 nsTask.Send(nsRank);
      }
      task->second->state = Loaded;
   }
}
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

void OrchBase::TaskDeploy(Cli::Cl_t Cl)
{
// Send the process map in simplified form to the Motherships, and the location of
// binaries. Argument is the task name, which needs to exist and be mapped to hardware.
// In large systems, there is a chance the user may ask to deploy the task before
// all available hardware has registered itself in the process map. In such a situation,
// a large task may not be able to be deployed yet. This isn't a fatal error; the user
// can try again and hope for success.
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
if (!task->second->linked){                   // task mapped?
  Post(157,task->first);
  return;
}

unsigned coreNum = 0;                     // virtual core number counter
unsigned cIdx = 0;                        // comm number to look for Motherships. Start from local MPI_COMM_WORLD.

vector<pair<unsigned,P_addr_t> > coreVec;  // core map container to send
vector<ProcMap::ProcMap_t>::iterator currBox = pPmap[cIdx]->vPmap.begin(); // process map for the Mothership being deployed to


CMsg_p PktC;                              // message to send to each participating box                    
PktC.Key(Q::NAME,Q::CFG,Q::DIST);         // distributing the core mappings  and the data directory
PktC.Src(Urank);
string taskname = task->first;
PktC.Zname(0, taskname);                  // first string field in the packet is the task name
//PktC.Put(0, &taskname);                 // first field in the packet is the task name
taskname+="/";                            // for the moment the binary directory will be fixed
taskname+=BIN_PATH;                       // later we could make this user-settable.
// duplicate P_builder's iteration through the task
P_core* thisCore;  // Core available during iteration.
P_thread* firstThread;  // The "first" thread in thisCore. "first" is arbitrary, because cores are stored in a map.
WALKMAP(AddressComponent, P_box*, pE->P_boxm, boxNode)
{
while (cIdx < Comms.size()) // grab the next available mothership
{
      while ((currBox != pPmap[cIdx]->vPmap.end()) && (currBox->P_class != csMOTHERSHIPproc)) ++currBox;
      if (currBox != pPmap[cIdx]->vPmap.end()) break;
      ++cIdx;
}
taskname.insert(0,string("/home/")+currBox->P_user+"/");
PktC.Zname(1,taskname); // second string field is the task binary directory
coreVec.clear();   // reset the packet content
WALKVECTOR(P_board*,boxNode->second->P_boardv,board)
{
WALKPDIGRAPHNODES(AddressComponent, P_mailbox*,
                  unsigned, P_link*,
                  unsigned, P_port*, (*board)->G, mailbox)
{
WALKMAP(AddressComponent, P_core*,
        (*board)->G.NodeData(mailbox)->P_corem, core)
{
thisCore = core->second;
firstThread = thisCore->P_threadm.begin()->second;
if (firstThread->P_devicel.size() && (firstThread->P_devicel.front()->par->par == task->second)) // only for cores which have something placed on them and which belong to the task
{
    // determine the last thread that has a device mapped onto it (recall that all devices within a core service the same task.
    std::map<AddressComponent, P_thread*>::reverse_iterator thread;
    for (thread=thisCore->P_threadm.rbegin();
         thread!=thisCore->P_threadm.rend(); thread++)
    {
        if (thread->second->P_devicel.size() > 0) break;
    }

    // Add the thread address to coreVec as a device address.
    HardwareAddress* threadHardwareAddress;
    threadHardwareAddress = thread->second->get_hardware_address();
    P_addr_t threadFullAddress;
    threadFullAddress.A_box = threadHardwareAddress->get_box();
    threadFullAddress.A_board = threadHardwareAddress->get_board();
    threadFullAddress.A_mailbox = threadHardwareAddress->get_mailbox();
    threadFullAddress.A_core = threadHardwareAddress->get_core();
    threadFullAddress.A_thread = threadHardwareAddress->get_thread();
    coreVec.push_back(pair<unsigned, P_addr_t>
                      (coreNum++, threadFullAddress));

   /* MLV and ADR, on 2019-01-18, discussed that the above was effectively synonymous with the below (for the new hardware model).
   // core lists here indicate the available hardware rather than the placed hardware, so we have
   // to search for the last placed thread.
   unsigned t_i = 1;
   while (t_i < thisCore->P_threadv.size() && thisCore->P_threadv[t_i]->P_devicel.size()) ++t_i;
   coreVec.push_back(pair<unsigned,P_addr_t>(coreNum++, *(dynamic_cast<P_addr_t*>(&(thisCore->P_threadv[--t_i]->addr))))); // add it to the core list to be sent (may need to cast thisCore->addr to P_addr_t)
   */
}
}
}
}
if ((cIdx >= Comms.size()) && coreVec.size()) // not enough Motherships have reported to deploy this task
{
   Post(164,task->first, int2str(cIdx), int2str(coreVec.size()));
   return;
}

// same machine running Root and Mothership? (Tediously this requires searching the
// process maps linearly because there is no method within a ProcMap to get the
// index of the entry which is Root)
int RootIndex = RootCIdx();
vector<ProcMap::ProcMap_t>::iterator RootProcMapI = pPmap[RootIndex]->vPmap.begin();
while (RootProcMapI->P_rank != pPmap[RootIndex]->U.Root) RootProcMapI++;
if (RootProcMapI->P_proc == currBox->P_proc)
{
   // then copy locally (inefficient, wasteful, using the files in place would be better but this would require
   // different messages to be sent to different Motherships. For a later revision.
   system((string("rm -r -f /home/")+currBox->P_user+"/"+task->first).c_str());
   system((string("mkdir /home/")+currBox->P_user+"/"+task->first).c_str());
   system((string("cp -r ")+taskpath+task->first+"/"+BIN_PATH+" "+taskname).c_str());
}
else
{
   // otherwise copy binaries to the Mothership using SCP. This assumes ssh-agent has been run for the user.
   system((string("ssh ")+currBox->P_user+"@"+currBox->P_proc+ "\"rm -r -f "+task->first+"\"").c_str());
   system((string("ssh ")+currBox->P_user+"@"+currBox->P_proc+ "\"mkdir "+task->first+"\"").c_str());
   system((string("scp -r ")+taskpath+task->first+"/"+BIN_PATH+" "+currBox->P_user+"@"+currBox->P_proc+":"+taskname).c_str());
}
PktC.comm = Comms[cIdx];  // Packet will go on the communicator it was found on
Post(726,int2str(currBox->P_rank),uint2str(coreVec.size()));
PktC.Put(0,&currBox->P_rank); // first field is the Mothership's address (rank or symbolic address)
PktC.Put(&coreVec);           // place the core map in the packet to this Mothership
// PktC.Send(currBox->P_rank);   // Send to the target Mothership
unsigned nsComm = 0;
for (; nsComm < pPmap.size(); nsComm++) // find the NameServer
{
    if (pPmap[nsComm]->U.NameServer != Q::NAP)
    {
       PktC.comm = Comms[nsComm];              // send the distribution message
       PktC.Send(pPmap[nsComm]->U.NameServer); // to the NameServer
       break;
    }
}
if (nsComm >= pPmap.size())                    // No NameServer. A severe error.
{
   Post(711);
   return;
}
}                                              // Next Mothership
}

//------------------------------------------------------------------------------

void OrchBase::TaskRecall(Cli::Cl_t Cl)
{
// Remove (recall) a given task from Motherships to which it is deployed.
// Argument is the task name, which needs to exist and be mapped to hardware.
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
if (!task->second->linked){                   // task mapped?
  Post(157,task->first);
  return;
}

unsigned cIdx = 0;                        // comm number to look for Motherships. Start from local MPI_COMM_WORLD.

vector<ProcMap::ProcMap_t>::iterator currBox = pPmap[cIdx]->vPmap.begin(); // process map for the Mothership being deployed to


PMsg_p PktD;                              // message to send to each mapped box
PktD.Key(Q::NAME,Q::RECL);                // telling it to remove its entries for the task
PktD.Src(Urank);
string taskname = task->first;
PktD.Put(0, &taskname);                    // first field in the packet is the task name
// duplicate P_builder's iteration through the task
P_thread* firstThread;  // The "first" thread in thisCore. "first" is arbitrary, because cores are stored in a map.
WALKMAP(AddressComponent, P_box*, pE->P_boxm, boxNode)
{
while (cIdx < Comms.size()) // grab the next available mothership
{
      while ((currBox != pPmap[cIdx]->vPmap.end()) && (currBox->P_class != csMOTHERSHIPproc)) ++currBox;
      if (currBox != pPmap[cIdx]->vPmap.end()) break;
      ++cIdx;
}
PktD.comm = 0; // reset the comm for the recall packet
// inefficient way to detect which boxes have this task mapped. In future the NameServer will hold this table and we should just be able to look it up.
WALKVECTOR(P_board*,boxNode->second->P_boardv,board)
{
WALKPDIGRAPHNODES(AddressComponent, P_mailbox*,
                  unsigned, P_link*,
                  unsigned, P_port*, (*board)->G, mailbox)
{
WALKMAP(AddressComponent, P_core*,
        (*board)->G.NodeData(mailbox)->P_corem, core)
{
firstThread = core->second->P_threadm.begin()->second;
if (firstThread->P_devicel.size() && (firstThread->P_devicel.front()->par->par == task->second)) // only for cores which have something placed on them and which belong to the task
{
   PktD.comm = Comms[cIdx];           // Packet will go on the communicator it was found on
   PktD.Send(currBox->P_rank);        // Send to the target Mothership
}
if (PktD.comm) break;                 // exit the search loop once any core mapped to the Mothership's box has been found.
}
if (PktD.comm) break;
}
}
} // Next Mothership
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

void OrchBase::TaskMCmd(Cli::Cl_t Cl,string cmd)
{
// Sends commands to the Motherships to control operation. At present the
// possible commands are "init", "run" and "stop". Also for this version,
// commands are sent sequentially to each Mothership, but future versions
// will multicast (this needs an extension to the PMsg_p interface)
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
if (!task->second->linked){                   // task mapped?
  Post(157,task->first);
  return;
}

// set up the message
PMsg_p Pkt;                              // message to send to each participating box
if (cmd == "init")
Pkt.Key(Q::CMND,Q::LOAD);                // distributing the core mappings
else if (cmd == "run")
Pkt.Key(Q::CMND,Q::RUN);
else if (cmd == "stop")
Pkt.Key(Q::CMND,Q::STOP);
else
{
  Post(25,cmd,"task");                   // invalid command to Mothership
  return;
}
Pkt.Src(Urank);
string taskname = task->first;
Pkt.Put(0, &taskname);                    // first field in the packet is the task name

unsigned cIdx = 0;                        // comm number to look for Motherships. Start from local MPI_COMM_WORLD.
vector<ProcMap::ProcMap_t>::iterator currBox = pPmap[cIdx]->vPmap.begin(); // process map for the Mothership being deployed to
P_thread* firstThread;  // The "first" thread in the core in the current iteration. "first" is arbitrary, because cores are stored in a map.

// search for boxes assigned to the task. For the future, it would be more efficient to build a map
// rather than redo the search.
WALKMAP(AddressComponent, P_box*, pE->P_boxm, boxNode)
{
while (cIdx < Comms.size()) // grab the next available mothership
{
      while ((currBox != pPmap[cIdx]->vPmap.end()) && (currBox->P_class != csMOTHERSHIPproc)) ++currBox;
      if (currBox != pPmap[cIdx]->vPmap.end()) break;
      ++cIdx;
}
bool UsedByTask = false;
WALKVECTOR(P_board*,boxNode->second->P_boardv,board)
{
WALKPDIGRAPHNODES(AddressComponent, P_mailbox*,
                  unsigned, P_link*,
                  unsigned, P_port*, (*board)->G, mailbox)
{
WALKMAP(AddressComponent, P_core*,
        (*board)->G.NodeData(mailbox)->P_corem, core)
{
firstThread = core->second->P_threadm.begin()->second;
if (firstThread->P_devicel.size() && (firstThread->P_devicel.front()->par->par == task->second)) // only for cores which have something placed on them and which belong to the task
{
   Pkt.comm = Comms[cIdx];           // Packet will go on the communicator it was found on
   Pkt.Send(currBox->P_rank);        // to the target Mothership. This will work for now, but it would be far better to broadcast the send (so everyone gets it synchronously)
   UsedByTask = true;
   break;
}
}
}
}
if (UsedByTask) break;               // only need to send once to each box
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

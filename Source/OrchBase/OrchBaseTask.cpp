//
// This is NOT a translation unit. Do not #include stuff here.
//
//==============================================================================

void OrchBase::ClearDcls()
// Clear out all the graph declaration blocks that are not used (i.e. have no
// corresponding graph instance)
{
WALKMAP(string,P_typdcl *,P_typdclm,i){// Walk the declaration blocks
  P_typdcl * p = (*i).second;          // Back reference vector empty?
  if (p->P_taskl.empty()) {            // If so, no-one wants this declare block
    delete p;
    P_typdclm.erase(i);
  }
}
}

//------------------------------------------------------------------------------

void OrchBase::ClearDcls(string sb)
// Clear out a named declare block PLUS any instances that depend upon it.
// There exists a pathology here: if an instance that is cleared also depends
// on a declare block tat has itself no other dependencies, that declare block
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
if (pP!=0) {
  WALKPDIGRAPHNODES(unsigned,P_box *,unsigned,P_link *,unsigned,P_port *,
                    pP->G,i){
    if (i==pP->G.NodeEnd()) break;
    P_box * pb = pP->G.NodeData(i);
    if (pb->pMothBin!=0) {             // Box-local binaries
      delete pb->pMothBin;
      pb->pMothBin=0;
    }
    WALKVECTOR(P_board *,pb->P_boardv,ib) {
      (*ib)->pSup=0;                   // Disconnect supervisor link
      WALKVECTOR(P_core *,(*ib)->P_corev,ic) {
        if ((*ic)->pCoreBin!=0) {      // Thread-local binaries
          delete (*ic)->pCoreBin;
          (*ic)->pCoreBin=0;
        }
        WALKVECTOR(P_thread *,(*ic)->P_threadv,it) {
          (*it)->P_devicel.clear();    // Kill the cross-links
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

}

//------------------------------------------------------------------------------

void OrchBase::ClearTasks(char tt)
// Clear either all monkey tasks or all POL tasks
{
                                       // Enumerated types?
                                       // Hah! We spit on enumerated types.
if ((tt!='T')&&(tt!='P')) Post(902,string(tt,1));
WALKMAP(string,P_task *,P_taskm,i) {
  P_task * p = (*i).second;            // Get the task pointer
  char t = p->IsPoL() ? 'P' : 'T';     // Task or POL?
  if (t==tt) {
    ClearTasks(p->Name());             // Kill it
    P_taskm.erase(i);                 // Remove map element
  }
}

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
      if ((*it)==pd) pt->P_devicel.erase(it);  // ..and remove it
}
                                       // Now disconnect from the declare block
P_typdcl * pdcl = p->pP_typdcl;        // This is the declare block
if (pdcl!=0)
  WALKLIST(P_task *,pdcl->P_taskl,i)   // Look for the back pointer...
    if ((*i)==p) pdcl->P_taskl.erase(i);       // ...and remove it
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
  if (strcmp(cs,"path")==0) { TaskPath(*i);  continue; }
  if (strcmp(cs,"pol" )==0) { TaskPol(*i);   continue; }
  if (strcmp(cs,"clea")==0) { TaskClear(*i); continue; }
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




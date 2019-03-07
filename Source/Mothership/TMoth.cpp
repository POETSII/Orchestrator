//------------------------------------------------------------------------------

#include "TMoth.h"
#include <stdio.h>
#include <sstream>
#include <dlfcn.h>
#include "Pglobals.h"
#include "CMsg_p.h"
#include "flat.h"
#include "string.h"
#include "limits.h"

const int   TMoth::NumBoards;

//==============================================================================

TMoth::TMoth(int argc,char * argv[],string d) :
  CommonBase(argc,argv,d,string(__FILE__)), HostLink()
{


FnMapx.push_back(new FnMap_t);    // create a new event map in the derived class

// Load the incoming event map
(*FnMapx[0])[PMsg_p::KEY(Q::EXIT                )] = &TMoth::OnExit; // overloads CommonBase
(*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::LOAD        )] = &TMoth::OnCmnd;
(*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::RUN         )] = &TMoth::OnCmnd;
(*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::STOP        )] = &TMoth::OnCmnd;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DIST        )] = &TMoth::OnName;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RECL        )] = &TMoth::OnName;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::TDIR        )] = &TMoth::OnName;
(*FnMapx[0])[PMsg_p::KEY(Q::SUPR                )] = &TMoth::OnSuper;
(*FnMapx[0])[PMsg_p::KEY(Q::SYST,Q::HARD        )] = &TMoth::OnSyst;
(*FnMapx[0])[PMsg_p::KEY(Q::SYST,Q::KILL        )] = &TMoth::OnSyst;
(*FnMapx[0])[PMsg_p::KEY(Q::SYST,Q::SHOW        )] = &TMoth::OnSyst;
(*FnMapx[0])[PMsg_p::KEY(Q::SYST,Q::TOPO        )] = &TMoth::OnSyst;
(*FnMapx[0])[PMsg_p::KEY(Q::TINS                )] = &TMoth::OnTinsel;

// mothership's address in POETS space is the host thread ID: X coordinate 0,
// Y coordinate max.
PAddress = TinselMeshYLen << (TinselMeshXBits+TinselLogCoresPerBoard+TinselLogThreadsPerCore);

twig_running = false;
ForwardMsgs = false; // don't forward tinsel traffic yet

MPISpinner();                          // Spin on *all* messages; exit on DIE
// printf("Exiting Mothership. Closedown flags: AcceptConns: %s, ForwardMsgs: %s\n", AcceptConns ? "true" : "false", ForwardMsgs ? "true" : "false");
// fflush(stdout);
if (twig_running) StopTwig(); // wait for the twig thread, if it's still somehow running.
printf("********* Mothership rank %d on the way out\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

TMoth::~TMoth()
{
//printf("********* Mothership rank %d destructor\n",Urank); fflush(stdout);
WALKMAP(uint32_t,PinBuf_t*,TwigMap,D)
{
  if (D->second)
  {
     WALKMAP(uint16_t,char*,(*(D->second)),P)
       delete[] P->second;
     delete D->second;
  }
}
WALKMAP(string,TaskInfo_t*,TaskMap,T)
  delete T->second;
}

//------------------------------------------------------------------------------

unsigned TMoth::Boot(string task)
{
   // printf("Entering boot stage\n");
   // fflush(stdout);
   if (TaskMap.find(task) == TaskMap.end())
   {
       Post(107,task);
       return 1;
   }
   // printf("Task %s being booted\n", task.c_str());
   // fflush(stdout);
   switch(TaskMap[task]->status)
   {
   case TaskInfo_t::TASK_BOOT:
   {
   Post(513, task, TaskInfo_t::Task_Status.find(TaskMap[task]->status)->second);
   TaskMap[task]->status = TaskInfo_t::TASK_ERR;
   return 2;
   }
   // printf("Task is ready to be booted\n");
   // fflush(stdout);
   case TaskInfo_t::TASK_RDY:
   {
   uint32_t mX, mY, core, thread;
   // create a response bitmap to receive the various startup barrier messages.
   // do this before running 'go' so that we can receive the responses in one block.
   map<unsigned, vector<unsigned>*> t_start_bitmap;
   vector<P_core*> taskCores = TaskMap[task]->CoresForTask();
   WALKVECTOR(P_core*,taskCores,C)
   {
       unsigned numThreads = (*C)->P_threadv.size();
       if (numThreads)
       {
          t_start_bitmap[TMoth::GetHWAddr((*C)->addr)] = new vector<unsigned>(numThreads/(8*sizeof(unsigned)),UINT_MAX);
          unsigned remainder = numThreads%(8*sizeof(unsigned));
          if (remainder) t_start_bitmap[TMoth::GetHWAddr((*C)->addr)]->push_back(UINT_MAX >> ((8*sizeof(unsigned))-remainder));
       }
   }
   // printf("Task start bitmaps created for %d cores\n", t_start_bitmap.size());
   // fflush(stdout);
   // actually boot the cores
   WALKVECTOR(P_core*,taskCores,C)
   {
     fromAddr(TMoth::GetHWAddr((*C)->addr),&mX,&mY,&core,&thread);
     // printf("Booting %d threads on threadID 0x%X at x:%d y:%d c:%d\n",(*C)->P_threadv.size(),TMoth::GetHWAddr((*C)->addr),mX,mY,core);
     // fflush(stdout);
     startOne(mX,mY,core,(*C)->P_threadv.size());
     // printf("%d threads started on threadID 0x%X at x:%d y:%d c:%d\n",(*C)->P_threadv.size(),TMoth::GetHWAddr((*C)->addr),mX,mY,core);
     // fflush(stdout);
     // printf("Triggering %d threads on core %d at x:%d y:%d c:%d\n",(*C)->P_threadv.size(),TMoth::GetHWAddr((*C)->addr),mX,mY,core);
     // fflush(stdout);
     goOne(mX,mY,core);
   }
   // per Matt Naylor comment safer to start all cores then issue the go command to all cores separately.
   // WALKVECTOR(P_core*,taskCores,C)
   // {
   //  printf("Triggering %d threads on core %d at x:%d y:%d c:%d\n",(*C)->P_threadv.size(),TMoth::GetHWAddr((*C)->addr),mX,mY,core);
   //  fflush(stdout);
   //  goOne(mX,mY,core);
   // }
   // printf("%d cores booted\n", taskCores.size());
   // fflush(stdout);
   P_Sup_Msg_t barrier_msg;
   while (!t_start_bitmap.empty())
   {
     recvMsg(&barrier_msg, p_sup_hdr_size());
     // printf("Received a message from a core during application barrier\n");
     // fflush(stdout);
     if ((barrier_msg.header.sourceDeviceAddr & P_SUP_MASK) && (barrier_msg.header.command == P_PKT_MSGTYP_BARRIER))
     {
        // printf("Barrier message from thread ID 0x%X\n", barrier_msg.header.sourceDeviceAddr ^ P_SUP_MASK);
	// fflush(stdout);
        fromAddr(((barrier_msg.header.sourceDeviceAddr ^ P_SUP_MASK) >> P_THREAD_OS),&mX,&mY,&core,&thread);
	unsigned hw_core = toAddr(mX,mY,core,0);
	// printf("Received a barrier acknowledge from core %#X\n", hw_core);
        // fflush(stdout);
	if (t_start_bitmap.find(hw_core) != t_start_bitmap.end())
	{
	   // printf("Thread %d on core %d responding\n", thread, core);
           // fflush(stdout);
	   // printf("Core bitmap for thread %d before acknowledge: %#X\n", thread, (*t_start_bitmap[hw_core])[thread/(8*sizeof(unsigned))]);
	   // fflush(stdout);
	   (*t_start_bitmap[hw_core])[thread/(8*sizeof(unsigned))] &= (~(1 << (thread%(8*sizeof(unsigned)))));
	   // printf("Core bitmap for thread %d after acknowledge: %#X\n", thread, (*t_start_bitmap[hw_core])[thread/(8*sizeof(unsigned))]);
	   // fflush(stdout);
	   vector<unsigned>::iterator S;
	   // printf("Core bitmap currently has %d elements\n", t_start_bitmap.size());
	   // fflush(stdout);
	   for (S = t_start_bitmap[hw_core]->begin(); S != t_start_bitmap[hw_core]->end(); S++) if (*S) break;
	   // printf("Core bitmap for core %d has %d subelements\n", hw_core, t_start_bitmap[hw_core]->size());
	   // fflush(stdout);
	   if (S == t_start_bitmap[hw_core]->end())
	   {
	      // printf("Removing core bitmap for core %d\n", thread, t_start_bitmap[hw_core]->size());
	      // fflush(stdout);
	      t_start_bitmap[hw_core]->clear();
	      delete t_start_bitmap[hw_core];
	      t_start_bitmap.erase(hw_core);
	   }
	}
     }
   }
   // printf("%d cores passed Mothership barrier and awaiting start signal \n", taskCores.size());
   // fflush(stdout);
   // MPI_Barrier(Comms[0]);         // barrier on the mothercore side (temporarily removed until we have multi-mothership systems)
   // create a thread (later will be a process) to deal with packets from
   // tinsel cores. Have to do it here, after all the initial setup is complete,
   // otherwise setup barrier communications may fail.
   void* args = this;
   // at this point, load the Supervisor for the task.
   SuperHandle = dlopen((TaskMap[task]->BinPath+"/libSupervisor.so").c_str(), RTLD_NOW);
   if (!SuperHandle) Post(532,(TaskMap[task]->BinPath+"/libSupervisor.so"),int2str(Urank),string(dlerror()));
   else
   {
      Post (540,int2str(Urank));
      int (*SupervisorInit)() = reinterpret_cast<int (*)()>(dlsym(SuperHandle, "SupervisorInit"));
      string badFunc("");
      if (!SupervisorInit || (*SupervisorInit)()) badFunc = "SupervisorInit";
      else if ((SupervisorCall = reinterpret_cast<int (*)(PMsg_p*, PMsg_p*)>(dlsym(SuperHandle, "SupervisorCall"))) == NULL) badFunc = "SupervisorCall";
      if (badFunc.size()) Post(533,badFunc,int2str(Urank),string(dlerror()));
   }
   ForwardMsgs = true; // set forwarding on so thread doesn't immediately exit
   if (pthread_create(&Twig_thread,NULL,Twig,args))
   {
      twig_running = false;
      ForwardMsgs = false;
      Post(531, int2str(Urank));
   }
   else twig_running =true;
   TaskMap[task]->status = TaskInfo_t::TASK_BARR; // now at barrier on the tinsel side.
   return 0;
   }
   case TaskInfo_t::TASK_RUN:
   case TaskInfo_t::TASK_STOP:
   case TaskInfo_t::TASK_END:
   break;
   default:
   TaskMap[task]->status = TaskInfo_t::TASK_ERR;
   }
   Post(511,task,"booted",TaskInfo_t::Task_Status.find(TaskMap[task]->status)->second);
   return 3;
}

//------------------------------------------------------------------------------

unsigned TMoth::CmLoad(string task)
// Load a task to the system
{
   // printf("%d tasks deployed to Mothership\n", TaskMap.size());
   // fflush(stdout);
   if (TaskMap.find(task) == TaskMap.end())
   {
      Post(515, task, int2str(Urank));
      return 0;
   }
   // printf("Task %s found\n", task.c_str());
   // fflush(stdout);
   WALKMAP(string,TaskInfo_t*,TaskMap,T)
   {
     // only one task can be active at a time (for the moment. Later we may want more sophisticated task mapping
     // that allows multiple active tasks on non-overlapping board sets)
     if ((T->first != task) && (T->second->status & (TaskInfo_t::TASK_BOOT | TaskInfo_t::TASK_RDY | TaskInfo_t::TASK_BARR | TaskInfo_t::TASK_RUN | TaskInfo_t::TASK_STOP | TaskInfo_t::TASK_ERR)))
     {
        map<unsigned char,string>::const_iterator othStatus = TaskInfo_t::Task_Status.find(T->second->status);
        if (othStatus == TaskInfo_t::Task_Status.end())
	T->second->status = TaskInfo_t::TASK_ERR;
	Post(508,T->first);
	return 2;
        Post(509,task,T->first,othStatus->second);
	return 0;
     }
   }
   // printf("Task status confirmed as TASK_LOAD\n");
   // fflush(stdout);
   if (!((TaskMap[task]->status == TaskInfo_t::TASK_IDLE) || (TaskMap[task]->status == TaskInfo_t::TASK_END)))
   {
      Post(511,task,"loaded to hardware",TaskInfo_t::Task_Status.find(TaskMap[task]->status)->second);
      return 0;
   }
   TaskMap[task]->status = TaskInfo_t::TASK_BOOT;
   // printf("Task status is TASK_BOOT\n");
   // fflush(stdout);
   int coresThisTask = 0;
   int coresLoaded = 0;
   // printf("Task will use %d boards\n", TaskMap[task]->BoardsForTask().size());
   // fflush(stdout);
   // for each board mapped to the task,
   WALKVECTOR(P_board*,TaskMap[task]->BoardsForTask(),B)
   {
      // printf("This board using %d cores\n", (*B)->P_corev.size());
      // fflush(stdout);
      // less work to compute as we go along than to call CoresForTask.size()
      coresThisTask += (*B)->P_corev.size();
      coresLoaded += LoadBoard(*B); // load the board (unthreaded model)
   }
   // printf("All boards finished; %d cores loaded\n", coresLoaded);
   // fflush(stdout);
   // abandoning the boot if load failed
   if (coresLoaded < coresThisTask)
   {
      TaskMap[task]->status = TaskInfo_t::TASK_ERR;
      Post(518,task,int2str(coresLoaded),int2str(coresThisTask));
      return 1;
   }
   TaskMap[task]->status = TaskInfo_t::TASK_RDY;
   // printf("Task status is TASK_RDY\n");
   // fflush(stdout);
   // boot the board (which has to be done from the main thread as it is not
   // thread-safe)
   return Boot(task);
}

//------------------------------------------------------------------------------

unsigned TMoth::CmRun(string task)
// Run a specified task. This passes the tinsel-side barrier to start
// application execution
{
   if (TaskMap.find(task) == TaskMap.end())
   {
      Post(107, task);
      return 0;
   }
   switch(TaskMap[task]->status)
   {
   case TaskInfo_t::TASK_BOOT:
   case TaskInfo_t::TASK_RDY:
   {
   Post(513, task, TaskInfo_t::Task_Status.find(TaskMap[task]->status)->second);
   TaskMap[task]->status = TaskInfo_t::TASK_ERR;
   return 1;
   }
   case TaskInfo_t::TASK_STOP:
   case TaskInfo_t::TASK_END:
   break;
   case TaskInfo_t::TASK_RUN:
   Post(511, task,"run","TASK_RUN");
   return 0;
   case TaskInfo_t::TASK_BARR:
   {
   // printf("Task %s entering tinsel barrier\n",task.c_str());
   // fflush(stdout);
   P_Msg_Hdr_t barrier_msg;
   barrier_msg.messageLenBytes = p_hdr_size(); // barrier is only a header. No payload.
   barrier_msg.destEdgeIndex = 0;                     // no edge index necessary.
   barrier_msg.destPin = P_SUP_PIN_INIT;              // it goes to the system __init__ pin
   barrier_msg.messageTag = P_MSG_TAG_INIT;           // and is of message type __init__.
   // printf("Building thread list for task %s\n",task.c_str());
   // fflush(stdout);
   // build a list of the threads in this task (that should be released from barrier)
   vector<unsigned> threadsToRelease;
   WALKVECTOR(P_thread*,TaskMap[task]->ThreadsForTask(),R)
     threadsToRelease.push_back(TMoth::GetHWAddr((*R)->addr));
   // printf("%d threads to release in task %s\n",threadsToRelease.size(),task.c_str());
   // fflush(stdout);
   while (!canSend()); // wait until a message can be sent. The Twig process should be fielding unexpected traffic by this point.
   // printf("Issuing barrier release to %d threads\n",threadsToRelease.size());
   // fflush(stdout);
   // and then issue the barrier release to the threads.
   WALKVECTOR(unsigned,threadsToRelease,R)
   {
     barrier_msg.destDeviceAddr = DEST_BROADCAST; // send to every device on the thread with a supervisor message
     // printf("Barrier release address: 0x%X\n", barrier_msg.destDeviceAddr);
     // fflush(stdout);
     send(*R,(p_hdr_size()/(4 << TinselLogWordsPerFlit) + (p_hdr_size()%(4 << TinselLogWordsPerFlit) ? 1 : 0)), &barrier_msg);
   }
   // printf("Tinsel threads now on their own for task %s\n",task.c_str());
   // fflush(stdout);
   TaskMap[task]->status = TaskInfo_t::TASK_RUN;
   return 0;
   }
   default:
   TaskMap[task]->status = TaskInfo_t::TASK_ERR;
   }
   Post(511,task,"run",TaskInfo_t::Task_Status.find(TaskMap[task]->status)->second);
   return 2;
}

//------------------------------------------------------------------------------

unsigned TMoth::CmStop(string task)
// Handle a stop command (which ends the simulation)
{
   if (TaskMap.find(task) == TaskMap.end())
   {
      Post(107, task);
      return 0;
   }
   switch(TaskMap[task]->status)
   {
   case TaskInfo_t::TASK_IDLE:
   case TaskInfo_t::TASK_BOOT:
   break;
   case TaskInfo_t::TASK_RDY:
   {
   Post(813, task, "TASK_RDY");
   return 1;
   }
   case TaskInfo_t::TASK_BARR:
   {
   // if we are at the barrier the simplest approach to
   // stop cleanly is simply to start and immediately stop.
   // thus as long as starting doesn't error, fall through
   // to the running condition immediately below.
   if (CmRun(task))
   {
      TaskMap[task]->status = TaskInfo_t::TASK_ERR;
      break;
   }
   }
   case TaskInfo_t::TASK_RUN:
   {
   TaskMap[task]->status = TaskInfo_t::TASK_STOP;
   // set up for shutdown by creating a global stop message
   P_Msg_Hdr_t stop_msg;
   stop_msg.destEdgeIndex = 0;           // ignore edge index. Unused.
   stop_msg.destPin = P_SUP_PIN_SYS_SHORT;     // goes to the system pin
   stop_msg.messageTag = P_MSG_TAG_STOP; // with a stop message type
   // printf("Stopping task %s\n",task.c_str());
   // fflush(stdout);
   // go through each thread of the task,
   WALKVECTOR(P_thread*, TaskMap[task]->ThreadsForTask(), R)
   {
     uint32_t destDevAddr = TMoth::GetHWAddr((*R)->addr);
     // printf("Stopping thread %d in task %s\n", destDevAddr, task.c_str());
     // fflush(stdout);
     stop_msg.destDeviceAddr = DEST_BROADCAST; // issue the stop message to all devices
     // wait for the interface
     while (!canSend());
     // then issue the stop packet
     send(destDevAddr,(p_hdr_size()/(4 << TinselLogWordsPerFlit) + p_hdr_size()%(4 << TinselLogWordsPerFlit) ? 1 : 0),&stop_msg);
   }
   TaskMap[task]->status = TaskInfo_t::TASK_END;
   // check to see if there are any other active tasks
   WALKMAP(string, TaskInfo_t*, TaskMap, tsk)
     if ((tsk->second->status == TaskInfo_t::TASK_BARR) || (tsk->second->status == TaskInfo_t::TASK_RUN)) return 0;
   // if not, shut down the Twig thread.
   if (twig_running) StopTwig();
   return 0;  
   }
   case TaskInfo_t::TASK_STOP:
   case TaskInfo_t::TASK_END:
   break;
   default:
   TaskMap[task]->status = TaskInfo_t::TASK_ERR;
   }
   Post(813,task,"stopped",TaskInfo_t::Task_Status.find(TaskMap[task]->status)->second);
   return 2;
}

//------------------------------------------------------------------------------

void TMoth::Dump(FILE * fp)
{
fprintf(fp,"Mothership dump+++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"Event handler table:\n");
unsigned cIdx = 0;
WALKVECTOR(FnMap_t*,FnMapx,F)
{
fprintf(fp,"Function table for comm %d:\n", cIdx++);
fprintf(fp,"Key        Method\n");
WALKMAP(unsigned,pMeth,(**F),i)
  fprintf(fp,"%#010x %018lx\n",(*i).first,(uint64_t)&(*i).second); //0x%010x
}
fprintf(fp,"Loaded tasks:\n");
WALKMAP(string,TaskInfo_t*,TaskMap,Task)
{
  fprintf(fp,"Task %s in state %s:\n",Task->second->TaskName.c_str(),TaskInfo_t::Task_Status.find(Task->second->status)->second.c_str());
  fprintf(fp,"Reading from binary path %s\n",Task->second->BinPath.c_str());
  fprintf(fp,"Task map dump:\n");
  Task->second->VirtualBox->Dump(fp);
  fprintf(fp,".......................................\n");
}
fprintf(fp,"Mothership dump-----------------------------------\n");
fflush(fp);
CommonBase::Dump(fp);
}

//------------------------------------------------------------------------------

int TMoth::LoadBoard(P_board* board)
{
      int coresLoaded = 0;
      string task;
      WALKMAP(string,TaskInfo_t*,TaskMap,K) // find our task
      {
	if (K->second->status == TaskInfo_t::TASK_BOOT) task = K->first;
	break;
      }
      if (!task.empty()) // anything to do?
      {
	 TaskInfo_t* task_map = TaskMap[task];
	 WALKVECTOR(P_core*,board->P_corev,C) // grab each core's code and data file
         {
	    string code_f(task_map->BinPath + "/softswitch_code_" + int2str(task_map->getCore(*C)) + ".v");
	    string data_f(task_map->BinPath + "/softswitch_data_" + int2str(task_map->getCore(*C)) + ".v");
	    (*C)->pCoreBin = new Bin(fopen(code_f.c_str(), "r"));
	    (*C)->pDataBin = new Bin(fopen(data_f.c_str(), "r"));
	    uint32_t mX, mY, core, thread;
	    // printf("Loading core with virtual address Bx:%d, Bd:%d, Cr:%d\n",(*C)->addr.A_box,(*C)->addr.A_board,(*C)->addr.A_core);
            // fflush(stdout);
	    fromAddr(TMoth::GetHWAddr((*C)->addr), &mX, &mY, &core, &thread);
	    // printf("Loading hardware thread 0x%X at x:%d y:%d c:%d\n",TMoth::GetHWAddr((*C)->addr),mX,mY,core);
            // fflush(stdout);
	    loadInstrsOntoCore(code_f.c_str(),mX,mY,core); // load instruction memory
	    loadDataViaCore(data_f.c_str(),mX,mY,core); // then data memory
	    ++coresLoaded;
         }
      }
      // printf("Boot process for board %s finished %d cores loaded\n", board->Name().c_str(), coresLoaded);
      // fflush(stdout);
      return coresLoaded;
}

//------------------------------------------------------------------------------

unsigned TMoth::NameDist(PMsg_p* mTask_Info)
// receive a broadcast core info block from the NameServer.
{
      CMsg_p task_info(*mTask_Info);
      string TaskName;
      task_info.Get(0, TaskName);
      map<string, TaskInfo_t*>::iterator T;
      if ((T=TaskMap.find(TaskName)) == TaskMap.end())
      {
         // printf("Inserting new task %s from NameDist\n", TaskName.c_str());
         // fflush(stdout);
	 TaskMap[TaskName] = new TaskInfo_t(TaskName);
      }
      vector<pair<unsigned,P_addr_t>> cores;
      task_info.Get(cores);
      // printf("Task %s has %d cores\n",TaskName.c_str(),cores.size());
      // fflush(stdout);
      // set up the cores
      for (vector<pair<unsigned,P_addr_t>>::iterator core = cores.begin(); core != cores.end(); core++)
          TaskMap[TaskName]->insertCore(core->first, core->second);
      // printf("%d cores inserted into TaskInfo structure for %s\n",cores.size(),TaskName.c_str());
      // fflush(stdout);
      return 0;
}

//------------------------------------------------------------------------------

unsigned TMoth::NameRecl(PMsg_p* mTask_Info)
// remove (recall) a core info block from the Mothership.
{
      string TaskName;
      mTask_Info->Get(0, TaskName);
      map<string, TaskInfo_t*>::iterator T;
      if ((T=TaskMap.find(TaskName)) == TaskMap.end()) // task exists?
      {
	 Post(607, TaskName); // No. Nothing to do.
	 return 0;
      }
      switch (TaskMap[TaskName]->status)
      {
      case TaskInfo_t::TASK_IDLE:
      case TaskInfo_t::TASK_BOOT:
      case TaskInfo_t::TASK_STOP:
      case TaskInfo_t::TASK_END:
      break;
      case TaskInfo_t::TASK_RDY:
      {
	 Post(813, TaskName, "recalled", "TASK_RDY");
         return 1;
      }
      case TaskInfo_t::TASK_BARR:
      case TaskInfo_t::TASK_RUN:
      CmStop(TaskName); // stop any running tasks before recalling them.
      break;
      default:
      {
         TaskMap[TaskName]->status = TaskInfo_t::TASK_ERR;
	 Post(812, TaskName);
	 return 1;
      }
      }
      system((string("rm -r -f ")+TaskMap[TaskName]->BinPath).c_str());
      delete T->second; // get rid of its TaskInfo object
      TaskMap.erase(T); // and then remove it from the task map
      return 0;
}

//------------------------------------------------------------------------------

unsigned TMoth::NameTdir(const string& task, const string& dir)
// set the path where binaries for a given task are to be found
{
      map<string, TaskInfo_t*>::iterator T;
      if ((T=TaskMap.find(task)) == TaskMap.end())
      {
         // printf("Inserting new task %s from NameTdir\n", task.c_str());
         // fflush(stdout);
	 TaskMap[task] = new TaskInfo_t(task);
      }
      TaskMap[task]->BinPath = dir;
      return 0;
}

//------------------------------------------------------------------------------

unsigned TMoth::OnCmnd(PMsg_p * Z, unsigned cIdx)
// Handler for a task command sent (probably) from the user.
{
// get the task that the command is going to operate on
string task;
Z->Get(0,task);
unsigned key = Z->Key();
if (key == PMsg_p::KEY(Q::CMND,Q::LOAD         ))
return CmLoad(task);
if (key ==  PMsg_p::KEY(Q::CMND,Q::RUN         ))
return CmRun(task);
if (key ==  PMsg_p::KEY(Q::CMND,Q::STOP        ))
return CmStop(task);
else
Post(510,"Control",int2str(Urank));
return 0;
}

//------------------------------------------------------------------------------

void* TMoth::Twig(void* par)
// the Twig processing deals with receiving packets from the managed Tinsel cores.
// Ideally, Twig, OnIdle, and OnTinselOut would reside in a separate *process*.
// For the moment we run Twig in a separate *thread* because to run a separate
// process would require identifying the matching Branch's rank, which could be
// done, using a variety of methods, but none of them trivial, and so that's
// 'for later'.
{
     TMoth* parent = static_cast<TMoth*>(par);
     const uint32_t szFlit = (1<<TinselLogBytesPerFlit);
     char recv_buf[p_msg_size()]; // buffer for one packet at a time
     void* p_recv_buf = static_cast<void*>(recv_buf);
     FILE* OutFile;
     char Line[4*P_MSG_MAX_SIZE];
     fpos_t readPos;
     fpos_t writePos;
     if ( (OutFile = fopen("./DebugOutput.txt", "a+")) )
     {
        fsetpos(OutFile, &readPos);
	fsetpos(OutFile, &writePos);
     }
     while (parent->ForwardMsgs) // until told otherwise,
     {
           // receive all available traffic. Should this be done or only one packet
           // and then try again for MPI? We don't expect MPI traffic to be intensive
           // and tinsel messsages might be time-critical.
           while (parent->canRecv())
           {
	         //printf("Message received from a Device\n");
		 //fflush(stdout);
                 parent->recv(recv_buf);
	         uint32_t* device = static_cast<uint32_t*>(p_recv_buf); // get the first word, which will be a device address
	         if (!(*device & P_SUP_MASK)) // bound for an external?
	         {
	            P_Msg_Hdr_t* m_hdr = static_cast<P_Msg_Hdr_t*>(p_recv_buf);
		    // printf("Message is bound for external device %d\n", m_hdr->destDeviceAddr);
		    // fflush(stdout);
	            if (parent->TwigExtMap[m_hdr->destDeviceAddr] == 0)
		       parent->TwigExtMap[m_hdr->destDeviceAddr] = new deque<P_Msg_t>;
	            if (m_hdr->messageLenBytes > szFlit)
		       parent->recvMsg(recv_buf+szFlit, m_hdr->messageLenBytes-szFlit);
	            parent->TwigExtMap[m_hdr->destDeviceAddr]->push_back(*(static_cast<P_Msg_t*>(p_recv_buf)));
		 }
	         else
                 {
	            P_Sup_Hdr_t* s_hdr = static_cast<P_Sup_Hdr_t*>(p_recv_buf);
		    s_hdr->sourceDeviceAddr ^= P_SUP_MASK;

		    if (s_hdr->command == P_PKT_MSGTYP_ALIVE)
		    {
		       // printf("Thread %d is still alive\n", s_hdr->sourceDeviceAddr >> P_THREAD_OS);
		       // fflush(stdout);
		    }
		    else
		    {
		       // printf("Message is a Supervisor request from device %d\n", s_hdr->sourceDeviceAddr);
		       // fflush(stdout);
	               if (parent->TwigMap[s_hdr->sourceDeviceAddr] == 0) // new device talking?
		       {
		          // printf("New device %d reporting to Supervisor\n", s_hdr->sourceDeviceAddr);
		          // fflush(stdout);
		          parent->TwigMap[s_hdr->sourceDeviceAddr] = new PinBuf_t;
		       }
	               if ((*(parent->TwigMap[s_hdr->sourceDeviceAddr]))[s_hdr->destPin] == 0) // inactive pin for the device?
		       {
		          // printf("New pin %d for device %d reporting to Supervisor\n", s_hdr->destPin, s_hdr->sourceDeviceAddr);
		          // fflush(stdout);
                          (*(parent->TwigMap[s_hdr->sourceDeviceAddr]))[s_hdr->destPin] = new char[MAX_P_SUP_MSG_BYTES]();
		       }
                       P_Sup_Msg_t* recvdMsg = static_cast<P_Sup_Msg_t*>(static_cast<void*>((*(parent->TwigMap[s_hdr->sourceDeviceAddr]))[s_hdr->destPin]));
	               memcpy(recvdMsg+s_hdr->seq,s_hdr,p_sup_hdr_size()); // stuff header into the persistent buffer
		       // printf("Expecting message of total length %d\n", s_hdr->cmdLenBytes);
		       // fflush(stdout);
                       uint32_t len = s_hdr->seq == s_hdr->cmdLenBytes/p_sup_msg_size() ?  s_hdr->cmdLenBytes%p_sup_msg_size() : p_sup_msg_size(); // more message to receive?
		       // printf("Length for sequence number %d: %d\n", s_hdr->seq, len);
		       // fflush(stdout);
	               if (len > szFlit) parent->recvMsg(((recvdMsg+s_hdr->seq)->data), len-szFlit); // get the whole message
	               if (super_buf_recvd(recvdMsg))
	               {
		          // printf("Entire Supervisor message received of length %d\n", s_hdr->cmdLenBytes);
		          // fflush(stdout);
		          if (parent->OnTinselOut(recvdMsg))
			     parent->Post(530, int2str(parent->Urank));
		          super_buf_clr(recvdMsg);
	               }
		    }
                 }
           }
	   // Capture anything happening on the DebugLink - which is text output directed at a file.
	   bool updated = false;
	   if (OutFile)
	   {
	      fsetpos(OutFile, &writePos);
	      while (parent->pollStdOut(OutFile)) updated = true;
	      if (updated)
	      {
		 // printf("Received a debug output message\n");
		 // fflush(stdout);
	      }
	      fgetpos(OutFile, &writePos);
	   }
	   else while (parent->pollStdOut()); // or possibly only dumped to the local console
	   // output the debug output buffer, which has to be done immediately because we are in a separate thread
           if (OutFile && updated)
           {
              fflush(OutFile);
	      fsetpos(OutFile, &readPos);
	      while (!feof(OutFile)) parent->Post(600, string(fgets(Line, 4*P_MSG_MAX_SIZE, OutFile)));
	      fgetpos(OutFile, &readPos);
	   }
     }
     printf("Exiting Twig thread\n");
     pthread_exit(par);
     return par;
}

//------------------------------------------------------------------------------

void TMoth::OnIdle()
// idle processing deals with forwarding packets from the managed Tinsel cores.
{
     // queues may be changing but we can deal with a static snapshot of the actual
     // queue because OnIdle will execute periodically.
     WALKMAP(uint32_t,deque<P_Msg_t>*,TwigExtMap,D)
     {
        int NameSrvComm = RootCIdx();
	// int NameSrvComm = NameSCIdx();
        PMsg_p W(Comms[NameSrvComm]);   // return packets to be routed via the NameServer's comm
        W.Key(Q::TINS);                 // it'll be a Tinsel packet
	W.Tgt(pPmap[NameSrvComm]->U.Root);     // temporary: dump external packets to root
        //W.Tgt(pPmap[NameSrvComm]->U.NameServer);     // directed to the NameServer (or UserIO, when we have it)
        W.Src(Urank);                   // coming from us
	/* well, this is awkward: the PMsg_p type has a Put method for vectors of objects,
           which is what we want. Our packet should have a vector of P_Msg_t's. But as things
           stand, the messages are trapped in a deque (because we want our twig process to
           be able to append to the vector of things to send). Which means copying them out
           into a vector. Again, this would be fine if we could copy them directly into a
           vector in the PMsg_p, but the interface doesn't allow it - it expects to copy
           from vector to vector. So we seem to be stuck with this silly bucket brigade
           approach. NOT the most efficient way to move messages.
	 */
	vector<P_Msg_t> packet;
	while (D->second->size())
	{
	      packet.push_back(D->second->front());
	      D->second->pop_front();
	}
        W.Put<P_Msg_t>(0,&packet);      // stuff the Tinsel messages into the packet
	W.Send();                       // and away it goes.
     }
}

//------------------------------------------------------------------------------

unsigned TMoth::OnName(PMsg_p * Z, unsigned cIdx)
// This handles what happens when the NameServer dumps a name subblock to the
// mothership
{
unsigned key = Z->Key();
if (key == PMsg_p::KEY(Q::NAME,Q::DIST         ))
{
   // printf("NameDist command received\n");
   // fflush(stdout);
   return NameDist(Z);
}
if (key == PMsg_p::KEY(Q::NAME,Q::RECL         ))
{
   // printf("NameRecl command received\n");
   // fflush(stdout);
   return NameRecl(Z);
}
if (key ==  PMsg_p::KEY(Q::NAME,Q::TDIR         ))
{
   string task,dir;
   Z->Get(0,task);
   Z->Get(1,dir);
   // printf("NameTdir command received: task %s, directory %s\n", task.c_str(), dir.c_str());
   return NameTdir(task,dir);
}
else
Post(510,"Name",int2str(Urank));
return 0;
}

//------------------------------------------------------------------------------

unsigned TMoth::OnExit(PMsg_p * Z, unsigned cIdx)
// This is what happens when a user command to stop happens
{
// We are going away. Shut down any active tasks.
WALKMAP(string, TaskInfo_t*, TaskMap, tsk)
{
       if ((tsk->second->status == TaskInfo_t::TASK_BOOT) || (tsk->second->status == TaskInfo_t::TASK_END)) continue;
       if (tsk->second->status == TaskInfo_t::TASK_BARR) CmRun(tsk->first);
       if (tsk->second->status == TaskInfo_t::TASK_RUN)  CmStop(tsk->first);
}
// stop accepting Tinsel messages
if (twig_running) StopTwig();
return CommonBase::OnExit(Z,cIdx); // exit through CommonBase handler
}

//------------------------------------------------------------------------------

unsigned TMoth::OnSuper(PMsg_p * Z, unsigned cIdx)
// Main hook for user-defined supervisor commands
{
// set up a return message if we are going to send some reply back
PMsg_p W(Comms[cIdx]);
W.Key(Q::SUPR);
W.Src(Z->Tgt());
int superReturn = 0;
if ((superReturn = (*SupervisorCall)(Z,&W)) > 0) // Execute. Send a reply if one is expected
{
  if (!cIdx && (Z->Tgt() == Urank) && (Z->Src() == Urank)) OnTinsel(&W, 0); // either to Tinsels,
  else W.Send(Z->Src());  // or to some external or internal process.
}
if (superReturn < 0) Post(530, int2str(Urank)); 
return 0;
}

//------------------------------------------------------------------------------

unsigned TMoth::OnSyst(PMsg_p * Z, unsigned cIdx)
// Handler for system commands sent by the user or operator
{
unsigned key = Z->Key();
if (key == PMsg_p::KEY(Q::SYST,Q::HARD        ))
{
vector<string> args;
Z->Get<string>(0,args);
if (SystHW(args)) // A system hardware command executes an external process.
{
   string cmd;
   WALKVECTOR(string,args,arg)
   {
     cmd+=(*arg);
     cmd+=(' ');
   }
   Post(520,int2str(Urank),cmd);
   return 0;
}
return 0;
}
if (key == PMsg_p::KEY(Q::SYST,Q::KILL        ))
return SystKill(); // Kill brutally shuts us down by exiting immediately
if (key == PMsg_p::KEY(Q::SYST,Q::SHOW        ))
return SystShow();
if (key == PMsg_p::KEY(Q::SYST,Q::TOPO        ))
return SystTopo();
else
Post(524,int2str(Z->Key()));
return 0;
}

//------------------------------------------------------------------------------

unsigned TMoth::OnTinsel(PMsg_p * Z, unsigned cIdx)
// Handler for direct packets to be injected into the network from an external source
{
vector<P_Msg_t> msgs; // messages are packed in Tinsel message format
Z->Get(0, msgs);      // We assume they're directly placed in the message
WALKVECTOR(P_Msg_t, msgs, msg) // and they're sent blindly
{
   uint32_t Len = static_cast<uint32_t>(msg->header.messageLenBytes);
   uint32_t FlitLen = Len >> TinselLogBytesPerFlit;
   if (Len << (32-TinselLogBytesPerFlit)) ++FlitLen;
   while (!canSend()); // if we have to we can run OnIdle to empty receive buffers
   send(msg->header.destDeviceAddr, FlitLen, &(*msg));
}
return 0;
}

//------------------------------------------------------------------------------

unsigned TMoth::OnTinselOut(P_Sup_Msg_t * packet)
// Deals with what happens when a Tinsel message is received. Generally we
// repack the message for delivery to the Supervisor handler and deal with
// it there. The Supervisor can do one of 2 things: A) process it itself,
// possibly generating another message; B) immediately export it over MPI to
// the user Executive or other external process.
{
// printf("Processing a command message 0x%x from Tinsel device %d\n", packet->header.command, packet->header.sourceDeviceAddr);
// fflush(stdout);
// handle the kill request from a tinsel core, which generally means an assert failed.
if ((packet->header.command == P_SUP_MSG_KILL)) return SystKill();
// output messages can simply be posted to the LogServer as an informational message.
if ((packet->header.command == P_SUP_MSG_LOG))
{
   // printf("Received a handler_log message from device %d\n", packet->header.sourceDeviceAddr, p_sup_msg_size());
   // fflush(stdout);
   // Just output the string (this will involve some rubbish at the end where arguments would be;
   // to be fixed later). Note that uint8_t*'s have to be reinterpret_casted to char*s.
   unsigned msg_len = ((packet->header.cmdLenBytes%p_sup_msg_size()) && (packet->header.seq == packet->header.cmdLenBytes/p_sup_msg_size())) ? packet->header.cmdLenBytes%p_sup_msg_size() : p_sup_msg_size()-p_sup_hdr_size();
   msg_len -= p_sup_hdr_size();
   Post(601, int2str(packet->header.sourceDeviceAddr), int2str(packet->header.seq), string(reinterpret_cast<const char*>(packet->data), msg_len));
   return 0;
}
// printf("Message from device %d is a Supervisor call. Redirecting\n", packet->header.sourceDeviceAddr);
// fflush(stdout);
PMsg_p W(Comms[0]);                        // Create a new packet on the local comm
W.Key(Q::SUPR);                            // it'll be a Supervisor packet
W.Src(Urank);                              // coming from the us
W.Tgt(Urank);                              // and directed at us
unsigned last_index = packet->header.cmdLenBytes/p_sup_msg_size() + (packet->header.cmdLenBytes%p_sup_msg_size() ? 1 : 0);
vector<P_Sup_Msg_t> pkt_v(packet,&packet[last_index]); // maybe slightly more efficient using the constructor
W.Put<P_Sup_Msg_t>(0,&pkt_v);    // stuff the Tinsel message into the packet
return OnSuper(&W, 0);
// W.Send();                        // away it goes.
// return 0;
}

//------------------------------------------------------------------------------

void TMoth::StopTwig()
// Cleanly shut down (or try to) the Twig process. This occurs whenever we
// end a task in preparation for shutting down, or exiting.
{
if (!twig_running) return;
ForwardMsgs = false;            // notify the Twig to shut down
pthread_join(Twig_thread,NULL); // wait for it to do so
if (SuperHandle)                // then unload its Supervisor
{
   // clean up memory first
   int (*SupervisorExit)() = reinterpret_cast<int (*)()>(dlsym(SuperHandle, "SupervisorExit"));
   if (!SupervisorExit || (*SupervisorExit)() || dlclose(SuperHandle))
   {  
      Post(534,int2str(Urank));
      SuperHandle = 0;          // even if we errored invalidate the Supervisor
   }
   SupervisorCall = 0;
}
twig_running = false;
}

//------------------------------------------------------------------------------

unsigned TMoth::SystHW(const vector<string>& args)
// Execute some command directly on the Mothership itself. Unlike a Supervisor
// message, which triggers the Supervisor function but doesn't change the
// functionality of the Mothership, this can directly interact with the
// running Mothership to do certain tasks
{
stringstream cmd(ios_base::out | ios_base::ate);
WALKCVECTOR(string, args, arg)
  cmd << *arg;
return system(cmd.str().c_str());
}

//------------------------------------------------------------------------------

unsigned TMoth::SystKill()
// Kill some process in the local MPI universe. We had better be sure there is
// no traffic destined for this process after such a brutal command!
{
return 1;
}

//------------------------------------------------------------------------------

unsigned TMoth::SystShow()
// Report the list of processes (active Motherships) on this comm
{
return 0;
}

//------------------------------------------------------------------------------

unsigned TMoth::SystTopo()
// Report this Mothership's local topology information (boards/cores/connections)
{
return 0;
}
//==============================================================================

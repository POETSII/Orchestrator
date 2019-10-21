//------------------------------------------------------------------------------

#include "Mothership.h"
#include <stdio.h>
#include <sstream>
#include <dlfcn.h>
#include "Pglobals.h"
#include "CMsg_p.h"
#include "flat.h"
#include "string.h"
#include "limits.h"

const int   Mothership::NumBoards;

//==============================================================================

Mothership::Mothership(int argc,char * argv[],string d) :
  SBase(argc,argv,d,string(__FILE__)), HostLink()
{


FnMapx.push_back(new FnMap_t);    // create a new event map in the derived class

// Load the incoming event map
(*FnMapx[0])[PMsg_p::KEY(Q::EXIT                        )] = &Mothership::OnExit; // overloads CommonBase
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DIST         )] = &Mothership::OnCfg;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::TDIR         )] = &Mothership::OnCfg;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::BLD          )] = &Mothership::OnCfg;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::RECL         )] = &Mothership::OnCfg;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DEL          )] = &Mothership::OnCfg;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::STATE        )] = &Mothership::OnCfg;
(*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::LOAD                )] = &Mothership::OnCmnd;
(*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::RUN                 )] = &Mothership::OnCmnd;
(*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::STOP                )] = &Mothership::OnCmnd;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::ALL )] = &Mothership::OnDump;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::NM  )] = &Mothership::OnDump;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::LIST        )] = &Mothership::OnDump;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::NM  )] = &Mothership::OnSend;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::ID  )] = &Mothership::OnSend;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::ALL )] = &Mothership::OnSend;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::NGRP)] = &Mothership::OnSend;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::IGRP)] = &Mothership::OnSend;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::NSUP)] = &Mothership::OnSend;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::ISUP)] = &Mothership::OnSend;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::SUPV        )] = &Mothership::OnSend;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::EXTN        )] = &Mothership::OnSend;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVT,Q::NM  )] = &Mothership::OnSend;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVT,Q::IN  )] = &Mothership::OnSend;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVT,Q::OUT )] = &Mothership::OnSend;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::ATTR        )] = &Mothership::OnSend;
(*FnMapx[0])[PMsg_p::KEY(Q::SUPR                        )] = &Mothership::OnSuper;
(*FnMapx[0])[PMsg_p::KEY(Q::SYST,Q::HARD                )] = &Mothership::OnSyst;
(*FnMapx[0])[PMsg_p::KEY(Q::SYST,Q::KILL                )] = &Mothership::OnSyst;
(*FnMapx[0])[PMsg_p::KEY(Q::SYST,Q::SHOW                )] = &Mothership::OnSyst;
(*FnMapx[0])[PMsg_p::KEY(Q::SYST,Q::TOPO                )] = &Mothership::OnSyst;
(*FnMapx[0])[PMsg_p::KEY(Q::TINS                        )] = &Mothership::OnTinsel;

// mothership's address in POETS space is the host bridge ID: 10 | BoxXCoordinate <<
// number of bits in the Tinsel address space. The version below only deals with
// left-hand-side boxes. Later this should be fixed.
PAddress = 1 << (1+TinselMeshYBits+TinselMeshXBits+TinselLogCoresPerBoard+TinselLogThreadsPerCore);

twig_running = false;
ForwardMsgs = false; // don't forward tinsel traffic yet

MPISpinner();        // Spin on *all* messages; exit on DIE
DebugPrint("Exiting Mothership. Closedown flags: AcceptConns: %s, "
           "ForwardMsgs: %s\n", AcceptConns ? "true" : "false",
           ForwardMsgs ? "true" : "false");
if (twig_running) StopTwig(); // wait for the twig thread, if it's still somehow running.
printf("********* Mothership rank %d on the way out\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

Mothership::~Mothership()
// Destructor gets rid of response pins added by the Twig, tasks, and
// additional event maps for comms other than MPI_COMM_WORLD
{
DebugPrint("********* Mothership rank %d destructor\n",Urank); fflush(stdout); 
WALKMAP(uint32_t,PinBuf_t*,TwigMap,D) // get rid of Twig pins 
{
  if (D->second)
  {
     WALKMAP(uint16_t,char*,(*(D->second)),P)
       delete[] P->second;
     delete D->second;
  }
}
WALKMAP(string,TaskInfo_t*,TaskMap,T) // and deployed tasks
  delete T->second;
 
WALKVECTOR(FnMap_t*,FnMapx,F)          
  delete *F;                           // get rid of derived class function tables
DebugPrint("********* Mothership rank %d destructor finished\n",Urank);
fflush(stdout);
}

//------------------------------------------------------------------------------

unsigned Mothership::Boot(string task)
/* Boot handles 2 functions: starting the cores and then dealing with a software
   barrier at startup. For the moment, the Tinsel barrier is not thread-safe,
   so this has to be executed at the level of the main mothership process.
   (We might prefer [PIP?] a thread-safe start process that can take a vector of
   cores and boot them all in parallel)
*/
{
   DebugPrint("Entering boot stage\n");
   if (TaskMap.find(task) == TaskMap.end()) // no such task
   {
       Post(107,task);
       return 1;
   }
   DebugPrint("Task %s being booted\n", task.c_str());
   switch(TaskMap[task]->status)
   {
   case TaskInfo_t::TASK_BOOT: // already being booted. This should not be run again.
   {
   Post(513, task, TaskInfo_t::Task_Status.find(TaskMap[task]->status)->second);
   TaskMap[task]->status = TaskInfo_t::TASK_ERR;
   return 2;
   }
   DebugPrint("Task is ready to be booted\n");
   case TaskInfo_t::TASK_RDY:
   {
   uint32_t mX, mY, core, thread;
   // create a response bitmap to receive the various startup barrier messages.
   // do this before running 'go' so that we can receive the responses in one block.
   map<unsigned, vector<unsigned>*> t_start_bitmap;
   vector<P_core*> taskCores = TaskMap[task]->CoresForTask();
   P_addr coreAddress;
   WALKVECTOR(P_core*,taskCores,C)
   {
       unsigned numThreads = (*C)->P_threadm.size();
       if (numThreads)
       {
          (*C)->get_hardware_address()->populate_a_software_address(&coreAddress);
	  // the bitmap is a vector of unsigneds, each bit representing a
	  // thread to be started. We can set up all threads in the bitmap 
	  // below the last modulo-32 group by simply initialising a vector of
	  // FFFFFFFF's
          t_start_bitmap[Mothership::GetHWAddr(coreAddress)] =
	    new vector<unsigned>(numThreads/(8*sizeof(unsigned)),UINT_MAX);
          unsigned remainder = numThreads%(8*sizeof(unsigned));
          if (remainder)
	      t_start_bitmap[Mothership::GetHWAddr(coreAddress)]->push_back
	      (UINT_MAX >> ((8*sizeof(unsigned))-remainder));
       }
   }
   DebugPrint("Task start bitmaps created for %d cores\n", t_start_bitmap.size());
   // actually boot the cores
   WALKVECTOR(P_core*,taskCores,C)
   {
     (*C)->get_hardware_address()->populate_a_software_address(&coreAddress);
     fromAddr(Mothership::GetHWAddr(coreAddress),&mX,&mY,&core,&thread);
     DebugPrint("Booting %d threads on threadID 0x%X at x:%d y:%d c:%d\n",
		(*C)->P_threadm.size(),Mothership::GetHWAddr(coreAddress),
		mX,mY,core);
     DebugPrint("* startOne(meshX=%u, meshY=%u, coreId=%u, numThreads=%lu)\n",
                mX, mY, core, (*C)->P_threadm.size());
     startOne(mX,mY,core,(*C)->P_threadm.size());
     DebugPrint("%d threads started on threadID 0x%X at x:%d y:%d c:%d\n",
		(*C)->P_threadm.size(),Mothership::GetHWAddr(coreAddress),
		mX,mY,core);
     DebugPrint("Triggering %d threads on core %d at x:%d y:%d c:%d\n",
		(*C)->P_threadm.size(),Mothership::GetHWAddr(coreAddress),
		mX,mY,core);
     DebugPrint("* goOne(meshX=%u, meshY=%u, coreId=%u)\n", mX, mY, core);
     goOne(mX,mY,core);
   }
   // per Matt Naylor comment safer to start all cores then issue the go command to all cores separately.
   // WALKVECTOR(P_core*,taskCores,C)
   // {
   //  (*C)->get_hardware_address()->populate_a_software_address(&coreAddress);
   //  DebugPrint("Triggering %d threads on core %d at x:%d y:%d c:%d\n",(*C)->P_threadm.size(),Mothership::GetHWAddr(coreAddress),mX,mY,core);
   //  goOne(mX,mY,core);
   // }
   // DebugPrint("%d cores booted\n", taskCores.size());

   // deal with the barrier. Keep receiving messages until the bitmap is clear.
   P_Sup_Msg_t barrier_msg;
   while (!t_start_bitmap.empty())
   {
     recvMsg(&barrier_msg, p_sup_hdr_size()); // blocking receive message call
     DebugPrint("Received a message from a core during application barrier\n");
     if ((barrier_msg.header.sourceDeviceAddr & P_SUP_MASK) &&
	 (barrier_msg.header.command == P_PKT_MSGTYP_BARRIER))
     {
        DebugPrint("Barrier message from thread ID 0x%X\n",
		   barrier_msg.header.sourceDeviceAddr ^ P_SUP_MASK);
        fromAddr(((barrier_msg.header.sourceDeviceAddr ^ P_SUP_MASK) >> P_THREAD_OS),
		   &mX,&mY,&core,&thread);
	unsigned hw_core = toAddr(mX,mY,core,0);
	DebugPrint("Received a barrier acknowledge from core %#X\n", hw_core);
	// valid core?
	if (t_start_bitmap.find(hw_core) != t_start_bitmap.end())
	{
	   DebugPrint("Thread %d on core %d responding\n", thread, core);
	   DebugPrint("Core bitmap for thread %d before acknowledge: %#X\n",
		      thread,
		      (*t_start_bitmap[hw_core])[thread/(8*sizeof(unsigned))]);
	   // clear the bitmap bit
	   (*t_start_bitmap[hw_core])[thread/(8*sizeof(unsigned))] &=
	     (~(1 << (thread%(8*sizeof(unsigned)))));
	   DebugPrint("Core bitmap for thread %d after acknowledge: %#X\n",
		      thread,
		      (*t_start_bitmap[hw_core])[thread/(8*sizeof(unsigned))]);
	   vector<unsigned>::iterator S;
	   DebugPrint("Core bitmap currently has %d elements\n",
		      t_start_bitmap.size());
	   // any elements of the vector still have uncleared bits?
	   for (S = t_start_bitmap[hw_core]->begin();
		S != t_start_bitmap[hw_core]->end(); S++) if (*S) break;
	   DebugPrint("Core bitmap for core %d has %d subelements\n",
		      hw_core, t_start_bitmap[hw_core]->size());
	   // all clear. This core is good to go. Remove it from the
	   // bitmap vector.
	   if (S == t_start_bitmap[hw_core]->end())
	   {
	      DebugPrint("Removing core bitmap for core %d\n",
			 thread, t_start_bitmap[hw_core]->size());
	      t_start_bitmap[hw_core]->clear();
	      delete t_start_bitmap[hw_core];
	      t_start_bitmap.erase(hw_core);
	   }
	}
     }
   }
   // Through the barrier.
   DebugPrint("%d cores passed Mothership barrier and awaiting start signal \n", taskCores.size());
   // MPI_Barrier(Comms[0]);         // barrier on the mothercore side (temporarily removed until we have multi-mothership systems)
   void* args = this;
   // at this point, load the Supervisor for the task.
   SuperHandle = dlopen((TaskMap[task]->BinPath+"/libSupervisor.so").c_str(),
			RTLD_NOW);
   if (!SuperHandle) Post(532,(TaskMap[task]->BinPath+"/libSupervisor.so"),
			  int2str(Urank),string(dlerror()));
   else
   {
      Post (540,int2str(Urank));
      //int (*SupervisorInit)(CommonBase*) = reinterpret_cast<int (*)(CommonBase*)>(dlsym(SuperHandle, "SupervisorInit"));
      // set up initialiser function
      int (*SupervisorInit)() = reinterpret_cast<int (*)()>(dlsym(SuperHandle,
                                                           "SupervisorInit"));
      string badFunc("");
      //if (!SupervisorInit || (*SupervisorInit)(this)) badFunc = "SupervisorInit";
      // run the initialiser
      if (!SupervisorInit || (*SupervisorInit)()) badFunc = "SupervisorInit";
      // and set up the main Supervisor function
      else if ((SupervisorCall = reinterpret_cast<int (*)(PMsg_p*, PMsg_p*)>
	       (dlsym(SuperHandle, "SupervisorCall"))) == NULL)
	       badFunc = "SupervisorCall";
      if (badFunc.size()) Post(533,badFunc,int2str(Urank),string(dlerror()));
   }
   // create a thread (later will be a process) to deal with packets from
   // tinsel cores. Have to do it here, after all the initial setup is complete,
   // otherwise setup barrier communications may fail.
   ForwardMsgs = true; // set forwarding on so thread doesn't immediately exit
   if (pthread_create(&Twig_thread,NULL,Twig,args))
   {
      twig_running = false;
      ForwardMsgs = false;
      Post(531, int2str(Urank));
   }
   else twig_running =true;
   Post(561, task.c_str());
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

unsigned Mothership::CmLoad(string task)
/* Load a task to the system. The task needs to have been previously deployed.
   This function responds to the task /init command (or equivalent). It farms
   off loading of individual boards to the LoadBoard function. LoadBoard CAN be
   threaded, but current Tinsel implementations don't support it, so for the
   moment LoadBoard works (slowly!) one board at a time.
*/
{
   DebugPrint("%d tasks deployed to Mothership\n", TaskMap.size());
   if (TaskMap.find(task) == TaskMap.end())  // task deployed?
   {
      Post(515, task, int2str(Urank));       // No. Abandon.
      return 0;
   }
   DebugPrint("Task %s found\n", task.c_str());
   WALKMAP(string,TaskInfo_t*,TaskMap,T)
   {
     // only one task can be active at a time (for the moment. Later we may
     // want more sophisticated task mapping that allows multiple active tasks
     // on non-overlapping board sets). Look for other active tasks.
     if ((T->first != task) && (T->second->status & (TaskInfo_t::TASK_BOOT |
						     TaskInfo_t::TASK_RDY |
						     TaskInfo_t::TASK_BARR |
						     TaskInfo_t::TASK_RUN |
						     TaskInfo_t::TASK_STOP |
						     TaskInfo_t::TASK_ERR)))
     {
        // found another task seemingly active. What state is it in?
        map<unsigned char,string>::const_iterator othStatus =
	   TaskInfo_t::Task_Status.find(T->second->status);
        if (othStatus == TaskInfo_t::Task_Status.end())
	   T->second->status = TaskInfo_t::TASK_ERR; // status corrupted.
	Post(508,T->first);
	return 2;
	// other task was in a sane but active state. Abandon.
        Post(509,task,T->first,othStatus->second);
	return 0;
     }
   }
   DebugPrint("Task status confirmed as TASK_LOAD\n");
   // Only this task is active. But since it's already active it
   // doesn't need to be loaded.
   if (!((TaskMap[task]->status == TaskInfo_t::TASK_IDLE) ||
	 (TaskMap[task]->status == TaskInfo_t::TASK_END)))
   {
      Post(511,task,"loaded to hardware",
	   TaskInfo_t::Task_Status.find(TaskMap[task]->status)->second);
      return 0;
   }
   // otherwise we are good to go. Task goes to boot state.
   TaskMap[task]->status = TaskInfo_t::TASK_BOOT;
   DebugPrint("Task status is TASK_BOOT\n");
   // Now load the boards. For the moment one at a time, later
   // this will be threaded.
   long coresThisTask = 0;
   long coresLoaded = 0;
   long boardsLoaded = 0;
   unsigned totalBoards = TaskMap[task]->BoardsForTask().size();
   unsigned totalCores = TaskMap[task]->CoresForTask().size();
   DebugPrint("Task will use %ld boards.\n",
              totalBoards);
   // for each board mapped to the task,
   WALKVECTOR(P_board*,TaskMap[task]->BoardsForTask(),B)
   {
      // some output is useful here to keep the operator aware that
      // we're doing something
      Post(563, long2str(coresLoaded),
	        uint2str(totalCores),
	        uint2str(boardsLoaded),
	        uint2str(totalBoards));
      DebugPrint("Board \"%s\" will use %ld mailboxes.\n",
                 (*B)->Name().c_str(), (*B)->G.SizeNodes());
      WALKPDIGRAPHNODES(AddressComponent, P_mailbox*,
                        unsigned, P_link*,
                        unsigned, P_port*, (*B)->G, MB)
      {
          DebugPrint("Mailbox \"%s\" will use %ld cores\n",
                     (*B)->G.NodeData(MB)->Name().c_str(),
                     (*B)->G.NodeData(MB)->P_corem.size());
          // less work to compute as we go along than to call CoresForTask.size()
          // MLV: Not so sure in a post-mailbox world any more...
          coresThisTask += (*B)->G.NodeData(MB)->P_corem.size();
      }
      coresLoaded += LoadBoard(*B); // load the board (unthreaded model)
      ++boardsLoaded;
   }
   DebugPrint("All boards finished. %ld cores were in the hardware model for "
              "this task. %ld cores have been loaded.\n",
              coresThisTask, coresLoaded);
   // abandoning the boot if load failed
   if (coresLoaded < coresThisTask)
   {
      TaskMap[task]->status = TaskInfo_t::TASK_ERR;
      Post(518,task,int2str(coresLoaded),int2str(coresThisTask));
      return 1;
   }
   TaskMap[task]->status = TaskInfo_t::TASK_RDY;
   DebugPrint("Task status is TASK_RDY\n");
   Post(560,task);
   // boot the board (which has to be done from the main thread as it is not
   // thread-safe)
   return Boot(task);
}

//------------------------------------------------------------------------------

unsigned Mothership::CmRun(string task)
// Run a specified task. This passes the tinsel-side barrier to start
// application execution. Invoked using task /run or equivalent.
{
   if (TaskMap.find(task) == TaskMap.end()) // task exists?
   {
      Post(107, task);
      return 0;
   }
   switch(TaskMap[task]->status) // and is it ready to run?
   {
   case TaskInfo_t::TASK_BOOT:   // these states mean it's still booting.
   case TaskInfo_t::TASK_RDY:    // i.e. not ready.
   {
   Post(513, task, TaskInfo_t::Task_Status.find(TaskMap[task]->status)->second);
   TaskMap[task]->status = TaskInfo_t::TASK_ERR;
   return 1;
   }
   case TaskInfo_t::TASK_STOP:
   Post(512, task,"run","TASK_STOP");
   TaskMap[task]->status = TaskInfo_t::TASK_ERR;
   return 2;
   case TaskInfo_t::TASK_END:
   case TaskInfo_t::TASK_IDLE:   // active or not deployed
   case TaskInfo_t::TASK_RUN:
   Post(511, task,"run",TaskInfo_t::Task_Status.find(TaskMap[task]->status)->second);
   return 0;
   case TaskInfo_t::TASK_BARR:
   {
   DebugPrint("Task %s entering tinsel barrier\n",task.c_str());
   P_Msg_Hdr_t barrier_msg;
   barrier_msg.messageLenBytes = p_hdr_size(); // barrier is only a header. No payload.
   barrier_msg.destEdgeIndex = 0;              // no edge index necessary.
   barrier_msg.destPin = P_SUP_PIN_INIT;       // it goes to the system __init__ pin
   barrier_msg.messageTag = P_MSG_TAG_INIT;    // and is of message type __init__.
   DebugPrint("Building thread list for task %s\n",task.c_str());
   // build a list of the threads in this task (that should be released
   // from barrier)
   vector<unsigned> threadsToRelease;
   P_addr threadAddress;
   WALKVECTOR(P_thread*,TaskMap[task]->ThreadsForTask(),R)
   {
   (*R)->get_hardware_address()->populate_a_software_address(&threadAddress);
   threadsToRelease.push_back(Mothership::GetHWAddr(threadAddress));
   }
   DebugPrint("Issuing barrier release to %d threads in task %s, using "
              "message address 0x%X\n", threadsToRelease.size(),
              task.c_str(), DEST_BROADCAST);
   // and then issue the barrier release to the threads.
   WALKVECTOR(unsigned,threadsToRelease,R)
   {
     // send to every device on the thread with a supervisor message
     barrier_msg.destDeviceAddr = DEST_BROADCAST;
     DebugPrint("Attempting to send barrier release message to the thread "
                "with hardware address %u.\n", *R);
     // this is the 'magic invocation' to release the Tinsel barrier
     send(*R,(p_hdr_size()/(4 << TinselLogWordsPerFlit) +
	  (p_hdr_size()%(4 << TinselLogWordsPerFlit) ? 1 : 0)),
	   &barrier_msg, true);
   }
   DebugPrint("Tinsel threads now on their own for task %s\n",task.c_str());
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

unsigned Mothership::CmStop(string task)
// Handle a task /stop command (which ends the simulation). Even if the task
// has internal termination detection, task /stop should always be issued to
// shut down the task cleanly (close Supervisors, clear memory, etc.)
{
   if (TaskMap.find(task) == TaskMap.end()) // no such task.
   {
      Post(107, task);
      return 0;
   }
   switch(TaskMap[task]->status) // is task in a state ready to be stopped?
   {
   case TaskInfo_t::TASK_IDLE:
   case TaskInfo_t::TASK_END:
   Post(511, task,"stopped",
	TaskInfo_t::Task_Status.find(TaskMap[task]->status)->second);
   return 0;
   case TaskInfo_t::TASK_BOOT:
   case TaskInfo_t::TASK_RDY:
   case TaskInfo_t::TASK_STOP:
   {
   Post(813, task, "stopped",
	TaskInfo_t::Task_Status.find(TaskMap[task]->status)->second);
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
   stop_msg.destEdgeIndex = 0;                 // ignore edge index. Unused.
   stop_msg.destPin = P_SUP_PIN_SYS_SHORT;     // goes to the system pin
   stop_msg.messageTag = P_MSG_TAG_STOP;       // with a stop message type
   DebugPrint("Stopping task %s\n",task.c_str());
   // go through each thread of the task,
   //vector<P_thread*> threads_for_task = TaskMap[task]->ThreadsForTask();
   P_addr threadAddress;
   WALKVECTOR(P_thread*, TaskMap[task]->ThreadsForTask(), R)
   {
     (*R)->get_hardware_address()->populate_a_software_address(&threadAddress);
     uint32_t destDevAddr = Mothership::GetHWAddr(threadAddress);
     DebugPrint("Stopping thread %d in task %s\n", destDevAddr, task.c_str());
     stop_msg.destDeviceAddr = DEST_BROADCAST; // issue the stop message to all devices
     // then issue the stop packet. Packet size 'future proofed' by computing
     // it rather than relying on a static constant (this should compile down
     // to a constant anyway)
     send(destDevAddr,(p_hdr_size()/(4 << TinselLogWordsPerFlit) +
                       p_hdr_size()%(4 << TinselLogWordsPerFlit) ? 1 : 0),
          &stop_msg, true);
   }
   TaskMap[task]->status = TaskInfo_t::TASK_END;
   // check to see if there are any other active tasks
   WALKMAP(string, TaskInfo_t*, TaskMap, tsk)
     if ((tsk->second->status == TaskInfo_t::TASK_BARR) ||
	 (tsk->second->status == TaskInfo_t::TASK_RUN)) return 0;
   // if not, shut down the Twig thread. For the moment, this also closes
   // supervisors. When we have multiple Supervisors running, it is probably
   // going to be better to shut each one down here.
   if (twig_running) StopTwig();
   return 0;
   }
   // any unexpected states generate an error.
   default:
   TaskMap[task]->status = TaskInfo_t::TASK_ERR;
   }
   Post(813,task,"stopped",
	TaskInfo_t::Task_Status.find(TaskMap[task]->status)->second);
   return 2;
}

//------------------------------------------------------------------------------

unsigned Mothership::ConfigDistribute(PMsg_p* msg, unsigned comm)
/* receive a broadcast core info block from the NameServer. This responds to
   the task /deploy command. Deploying a task has 2 effects: 1) it populates
   nameserver entries on this Mothership for the task; 2) it sets up a
   TaskInfo_t structure which contains basic info about the task such as what
   state it's in, where its files are, and what hardware resources it 'owns'.
*/
{
      CMsg_p task_info(*msg); // a deploy message has a special message type
      string TaskName = msg->Zname(0); // first static string is the name
      map<string, TaskInfo_t*>::iterator T;
      if ((T=TaskMap.find(TaskName)) == TaskMap.end()) // task already exists?
      {
         DebugPrint("Inserting new task %s from NameDist\n", TaskName.c_str());
	 TaskMap[TaskName] = new TaskInfo_t(TaskName);
      }
      DebugPrint("Task %s will have binaries in directory %s\n",
		 msg->Zname(0).c_str(),msg->Zname(1).c_str());
      // most of the deploy information is the core mappings
      vector<pair<unsigned,P_addr_t>> cores;
      task_info.Get(cores);
      DebugPrint("Task %s has %d cores\n",TaskName.c_str(),cores.size());
      // set up the cores
      for (vector<pair<unsigned,P_addr_t>>::iterator core = cores.begin(); core != cores.end(); core++)
          TaskMap[TaskName]->insertCore(core->first, core->second);
      DebugPrint("%d cores inserted into TaskInfo structure for %s\n",cores.size(),TaskName.c_str());
      unsigned err = AddressBook::SUCCESS;
      // populate the nameserver entries
      if ((err = SBase::ConfigDistribute(msg, comm)) != AddressBook::SUCCESS)
	  return err;
      DebugPrint("About to set binary directory %s for task %s\n",
                 msg->Zname(1).c_str(),msg->Zname(0).c_str());
      // set the directory for core binaries
      return ConfigDir(msg, comm);
}

//------------------------------------------------------------------------------

unsigned Mothership::ConfigRecall(PMsg_p* msg, unsigned comm)
// remove (recall) a core info block from the Mothership. This frees up all
// resources owned by a given task to be reallocatable to another task.
{
      string TaskName = msg->Zname(0);
      map<string, TaskInfo_t*>::iterator T;
      vector<string> taskNames; // list of tasks to recall
      if (TaskName.empty()) // no task name => recall all tasks
      {
	 WALKMAP(string, TaskInfo_t*, TaskMap, task)
	        taskNames.push_back(task->first);
      }
      else                 // otherwise remove the named task
      {
	 if ((T=TaskMap.find(TaskName)) == TaskMap.end()) // task exists?
         {
	    Post(607, TaskName); // No. Nothing to do in this Mothership.
	    return SBase::ConfigRecall(msg, comm); // but remove from base.
         }
	 taskNames.push_back(TaskName);
      }
      WALKVECTOR(string, taskNames, tName) // recall all tasks in the list
      {
         switch (TaskMap[*tName]->status)
         {
         case TaskInfo_t::TASK_IDLE:
         case TaskInfo_t::TASK_BOOT:
         case TaskInfo_t::TASK_STOP:
         case TaskInfo_t::TASK_END:
         break;
         case TaskInfo_t::TASK_RDY:
         {
	    // A task that is ready to run should not be recalled
	    // but stopped first.
	    Post(813, *tName, "recalled", "TASK_RDY");
            return AddressBook::ERR_INVALID_TASK;
         }
         case TaskInfo_t::TASK_BARR:
         case TaskInfo_t::TASK_RUN:
         CmStop(*tName); // stop any running tasks before recalling them.
         break;
         default:
         {
	    // Task in an unknown state - something very wrong has happened!
            TaskMap[*tName]->status = TaskInfo_t::TASK_ERR;
	    Post(812, *tName);
	    return AddressBook::ERR_INVALID_TASK;
         }
         } // ends switch (TaskMap[*tName]->status)
         system((string("rm -r -f ")+TaskMap[*tName]->BinPath).c_str());
         delete TaskMap[*tName]; // get rid of its TaskInfo object
         TaskMap.erase(*tName);  // and then remove it from the task map
      }
      return SBase::ConfigRecall(msg, comm); // remove from base class
}

//------------------------------------------------------------------------------

unsigned Mothership::ConfigDir(PMsg_p *msg, unsigned comm)
// set the path where binaries for a given task are to be found. This can be
// issued as a separate command: task /xpath, although usually task /deploy
// sets this up.
{
      string task = msg->Zname(0); // task name is in the first static field
      string dir = msg->Zname(1);  // and the binary path is in the second
      DebugPrint("Name config dir command received: task %s, directory %s\n",
		 task.c_str(), dir.c_str());
      map<string, TaskInfo_t*>::iterator T;
      if ((T=TaskMap.find(task)) == TaskMap.end()) // task already exists?
      {
	 // if not set it up.
         DebugPrint("Inserting new task %s from NameTdir\n", task.c_str());
         TaskMap[task] = new TaskInfo_t(task);
      }
      TaskMap[task]->BinPath = dir;
      return SBase::ConfigDir(msg, comm); // inform SBase about the path too.
}

//------------------------------------------------------------------------------

unsigned Mothership::ConfigState(PMsg_p *msg, unsigned comm)
{
     /* Overloaded function to deal with a command Q::NAME,Q::CFG,Q::STATE.
        A Mothership should never receive a state-configuration name message. It
        should only send them, or update the state internally and call the SBase
        method from the internal update.
     */
     // get the name to distinguish the type of error
     string taskName = msg->Zname(0);
     if (taskName.empty())
       Post(710, "ConfigState", int2str(Urank));       // no task name
     else if (TaskMap.find(taskName) == TaskMap.end())
       Post(715, taskName, int2str(Urank)); // task not loaded to this Mothership
     // find the task and warn the user that they are attempting something illegal
     else
     {
       map<unsigned char,string>::const_iterator st =
	  TaskInfo_t::Task_Status.find(TaskMap[taskName]->status);
       if (st == TaskInfo_t::Task_Status.end())
       {
          Post(550, int2str(Urank), msg->Zname(1), "UNKNOWN");
	  return AddressBook::ERR_INVALID_STATE;
       }
       else Post(550, int2str(Urank), msg->Zname(1), st->second);
     }
     return AddressBook::ERR_NONFATAL; // whatever the case we ignore the message
}

//------------------------------------------------------------------------------

unsigned Mothership::Connect(string svc)
{
   /* Connect deals with what happens when a new MPI universe successfully
      connects. This can happen, e.g. if a group of Motherships sit independent
      of the Root and its associated processes (perhaps physically separated as
      well), and the rest of the Orchestrator connects with the (already running)
      Motherships which have set themselves up in their own universe and
      published a service name by which they may be accessed. Essentially, we
      do everything in the constructor related to universe setup.
   */
   unsigned connErr = MPI_SUCCESS;
   // set up the connection in the base class
   if ((connErr = SBase::Connect(svc)) != MPI_SUCCESS) return connErr;
   FnMapx.push_back(new FnMap_t); // add another function table in the derived class
   int fIdx=FnMapx.size()-1;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::EXIT                        )] = &Mothership::OnExit; // overloads CommonBase
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DIST         )] = &Mothership::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::TDIR         )] = &Mothership::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::BLD          )] = &Mothership::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::RECL         )] = &Mothership::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DEL          )] = &Mothership::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::STATE        )] = &Mothership::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::CMND,Q::LOAD                )] = &Mothership::OnCmnd;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::CMND,Q::RUN                 )] = &Mothership::OnCmnd;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::CMND,Q::STOP                )] = &Mothership::OnCmnd;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::ALL )] = &Mothership::OnDump;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::NM  )] = &Mothership::OnDump;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::LIST        )] = &Mothership::OnDump;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::NM  )] = &Mothership::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::ID  )] = &Mothership::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::ALL )] = &Mothership::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::NGRP)] = &Mothership::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::IGRP)] = &Mothership::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::NSUP)] = &Mothership::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::ISUP)] = &Mothership::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::SUPV        )] = &Mothership::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::EXTN        )] = &Mothership::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVT,Q::NM  )] = &Mothership::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVT,Q::IN  )] = &Mothership::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVT,Q::OUT )] = &Mothership::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::ATTR        )] = &Mothership::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::SUPR                        )] = &Mothership::OnSuper;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::SYST,Q::HARD                )] = &Mothership::OnSyst;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::SYST,Q::KILL                )] = &Mothership::OnSyst;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::SYST,Q::SHOW                )] = &Mothership::OnSyst;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::SYST,Q::TOPO                )] = &Mothership::OnSyst;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::TINS                        )] = &Mothership::OnTinsel;
   return MPI_SUCCESS;
}


//------------------------------------------------------------------------------

void Mothership::Dump(FILE * fp, string task)
{
// Comprehensive dump. Includes both all the task information and the NameServer
// data.
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
WALKMAP(string,TaskInfo_t*,TaskMap,TaskI)
{
  fprintf(fp,"Task %s in state %s:\n",TaskI->second->TaskName.c_str(),
          TaskInfo_t::Task_Status.find(TaskI->second->status)->second.c_str());
  fprintf(fp,"Reading from binary path %s\n",TaskI->second->BinPath.c_str());
  fprintf(fp,"Task map dump:\n");
  TaskI->second->VirtualBox->Dump(fp);
  fprintf(fp,".......................................\n");
}
if (task.empty())
{
   fprintf(fp,"SBase summary dump___________________________\n");
   SBase::Dump(fp);
   fprintf(fp,"SBase summary dump---------------------------\n");
}
else if (task == "*")
{
   vector<string> tasks;
   unsigned err = AddressBook::SUCCESS;
   if ((err = ListTask(tasks)))            // get all the tasks
   {
      Post(710, "Dump", int2str(Urank));   // error: no tasks were found
      if (fp != stdout) fclose(fp);
      return;                              // so we don't need to do anything
   }
   fprintf(fp,"SBase dump of all tasks______________________\n");
   WALKVECTOR(string, tasks, task) SBase::Dump(fp, *task); // dump it all out
   fprintf(fp,"SBase dump of all tasks----------------------\n");
}
else
{
   fprintf(fp,"SBase dump___________________________________\n");
   SBase::Dump(fp,task);
   fprintf(fp,"SBase dump-----------------------------------\n");
}

fprintf(fp,"Mothership dump-----------------------------------\n");
fflush(fp);
CommonBase::Dump(fp);
}

//------------------------------------------------------------------------------

// Name Dump responses (from -n and -s switches)

unsigned Mothership::DumpAll(PMsg_p * msg, unsigned comm)
{
   /* Second static name string is the file name to dump to. The first static
      string should be empty. We use the second one so that the first field is
      used in a consistent way - for a task name. Filename must be treated as an
      absolute literal, because tasks could in
      general have different paths, therefore there is no way to infer what
      additional path string might be appended to the file name given.
   */
   string filename = msg->Zname(1);
   FILE* dumpFile;
   if (filename.empty()) dumpFile = stdout; // empty filename => dump to console
   else dumpFile = fopen(filename.c_str(), "a");
   Dump(dumpFile, "*");
   if (dumpFile != stdout) fclose(dumpFile);
   return SUCCESS; // all tasks dump can't fail badly
}

unsigned Mothership::DumpSummary(PMsg_p * msg, unsigned comm)
{
   // route through base to arrive (virtually) back at the Dump command
   // above
   return SBase::DumpSummary(msg, comm);
}

unsigned Mothership::DumpTask(PMsg_p * msg, unsigned comm)
{
   return SBase::DumpTask(msg, comm);
}

//------------------------------------------------------------------------------

long Mothership::LoadBoard(P_board* board)
{
    /* This is the unthreaded version of LoadBoard, the function that does
       the data transfer of binaries from files to SDRAM. Currently boards are
       loaded 1 core at a time because of hardware limitations. LoadBoard has
       2 separate binaries, a data binary and an executable image, loaded into
       different regions as specified in the linker maps generated by the
       softswitch builder. Each thread has its own private data space in SDRAM;
       each core has a private instruction SRAM, although depending upon
       configuration 2 cores - an even and odd-numbered pair, may share
       instruction memory (and have to have the same image)
     */
    long coresLoaded = 0;
    string task;

    // Find the task to load onto this board.
    WALKMAP(string, TaskInfo_t*, TaskMap, K)
    {
        if (K->second->status == TaskInfo_t::TASK_BOOT) task = K->first;
	DebugPrint("Found task to load %s\n", task.c_str());
        break;
    }
    if (!task.empty()) // anything to do?
    {
        TaskInfo_t* task_map = TaskMap[task];
        P_addr coreAddress, mailboxAddress;
        uint32_t mX, mY, core, thread;  // Intermediates for HostLink-side
                                        // address components.
	DebugPrint("Loading task %s\n", task.c_str());
	// go through all mapped mailboxes,
        WALKPDIGRAPHNODES(AddressComponent, P_mailbox*,
                          unsigned, P_link*,
                          unsigned, P_port*, board->G, MB)
        {
	    // some output is useful here to keep the operator aware that
            // we're doing something.
	    MB->second.data->get_hardware_address()->
              populate_a_software_address(&mailboxAddress);
	    Post(562, long2str(coresLoaded),
		      uint2str(mailboxAddress.A_box),
		      uint2str(mailboxAddress.A_board));
	    // and each mapped core on a mapped mailbox
            WALKMAP(AddressComponent, P_core*,
                    board->G.NodeData(MB)->P_corem, C)
            {
                // Grab this core's code and data file
                string code_f(task_map->BinPath + "/softswitch_code_" +
                              int2str(task_map->getCore(C->second)) + ".v");
                string data_f(task_map->BinPath + "/softswitch_data_" +
                              int2str(task_map->getCore(C->second)) + ".v");
                C->second->instructionBinary = new Bin(fopen(code_f.c_str(),
                                                             "r"));
                C->second->dataBinary = new Bin(fopen(data_f.c_str(), "r"));

                // Populate the P_addr coreAddress from the core's hardware
                // address object. Use coreAddress to define mX, mY, core, and
                // thread, for compatbilitity with HostLink's loading methods.
                C->second->get_hardware_address()->
                    populate_a_software_address(&coreAddress);
                DebugPrint("Loading core with virtual address Bx:%d, Bd:%d, "
                           "Mb: %d, Cr:%d\n",
                           coreAddress.A_box, coreAddress.A_board,
                           coreAddress.A_mailbox, coreAddress.A_core);
                fromAddr(Mothership::GetHWAddr(coreAddress), &mX, &mY, &core,
                         &thread);
                DebugPrint("Loading hardware thread 0x%X at x:%d y:%d c:%d\n",
                           Mothership::GetHWAddr(coreAddress), mX, mY, core);

                // Load instruction memory, then data memory.
                DebugPrint("* loadInstrsOntoCore(codeFilename=%s, meshX=%u, "
                           "meshY=%u, coreId=%u)\n",
                           code_f.c_str(), mX, mY, core);
                loadInstrsOntoCore(code_f.c_str(), mX, mY, core);
                DebugPrint("* loadDataViaCore(dataFilename=%s, meshX=%u, "
                           "meshY=%u, coreId=%u)\n",
                           data_f.c_str(), mX, mY, core);

                loadDataViaCore(data_f.c_str(), mX, mY, core);
                ++coresLoaded;
            }
        }
    }
    DebugPrint("Boot process for board %s finished %ld cores loaded\n",
               board->Name().c_str(), coresLoaded);
    return coresLoaded;
}


//------------------------------------------------------------------------------

unsigned Mothership::OnCfg(PMsg_p *msg, unsigned comm)
{
   // top-level instruction dispatch for Q::NAME, Q::CFG command subset
   switch(msg->L(2))
   {
   case Q::DIST:                // DIST and TDIR both set the binary directory;
   return ConfigDistribute(msg, comm); // DIST also loads the core map
   case Q::TDIR:
   return ConfigDir(msg, comm);
   case Q::BLD:
   return SBase::ConfigBuild(msg, comm); // BLD rebuilts SBase maps
   case Q::RECL:
   return ConfigRecall(msg, comm);
   case Q::DEL:
   return SBase::ConfigDelete(msg, comm);
   case Q::STATE:                     // STATE on a mothership will be ignored
   return ConfigState(msg, comm);     //  but issue a warning
   default:
   Post(700,uint2str(msg->Key()),int2str(Urank));
   return 0;
   }
}

//------------------------------------------------------------------------------

unsigned Mothership::OnCmnd(PMsg_p * Z, unsigned cIdx)
// Handler for a task command sent (probably) from the user. These handle the
// basic load, start, and stop operations.
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

unsigned Mothership::OnDump(PMsg_p *msg, unsigned comm)
// handlers for various forms of dump
{
   if (msg->L(2) == Q::LIST) return DumpSummary(msg, comm);
   if (msg->L(2) != Q::TASK)
   {
      Post(700,uint2str(msg->Key()),int2str(Urank));
      return 0;
   }
   switch(msg->L(3))
   {
   case Q::ALL:
     return DumpAll(msg, comm);
   case Q::NM:
     return DumpTask(msg, comm);
   default:
   Post(700,uint2str(msg->Key()),int2str(Urank));
   return 0;
   }
}

//------------------------------------------------------------------------------

unsigned Mothership::OnExit(PMsg_p * Z, unsigned cIdx)
// This is what happens when a user command to stop happens
{
   // We are going away. Shut down any active tasks.
   WALKMAP(string, TaskInfo_t*, TaskMap, tsk)
   {
       if ((tsk->second->status == TaskInfo_t::TASK_BOOT) ||
           (tsk->second->status == TaskInfo_t::TASK_END)) continue;
       // automatically run tasks at the barrier and immediately stop them;
       // this is the most expedient way of stopping tasks that have never
       // actually been started.
       if (tsk->second->status == TaskInfo_t::TASK_BARR) CmRun(tsk->first);
       if (tsk->second->status == TaskInfo_t::TASK_RUN)  CmStop(tsk->first);
   }
   // stop accepting Tinsel messages
   if (twig_running) StopTwig();
   return CommonBase::OnExit(Z,cIdx); // exit through CommonBase handler
}

//------------------------------------------------------------------------------

void Mothership::OnIdle()
// idle processing deals with forwarding packets from the managed Tinsel cores.
{
     // queues may be changing but we can deal with a static snapshot of the
     // actual queue because OnIdle will execute periodically.
     WALKMAP(uint32_t,deque<P_Msg_t>*,TwigExtMap,D)
     {
        int NameSrvComm = RootCIdx();
	// int NameSrvComm = NameSCIdx();
        PMsg_p W(Comms[NameSrvComm]); // return packets to be routed via the NameServer's comm
        W.Key(Q::TINS);               // it'll be a Tinsel packet
	W.Tgt(pPmap[NameSrvComm]->U.Root);     // temporary: dump external packets to root
        //W.Tgt(pPmap[NameSrvComm]->U.NameServer);     // directed to the NameServer (or UserIO, when we have it)
        W.Src(Urank);                   // coming from us
	/* well, this is awkward: the PMsg_p type has a Put method for vectors
           of objects, which is what we want. Our packet should have a vector of
           P_Msg_t's. But as things stand, the messages are trapped in a deque
           (because we want our twig process to be able to append to the vector of
           things to send). Which means copying them out into a vector. Again,
           this would be fine if we could copy them directly into a vector in the
           PMsg_p, but the interface doesn't allow it - it expects to copy from
           vector to vector. So we seem to be stuck with this odd bucket brigade
           approach. NOT the most efficient way to move messages.
	 */
	vector<P_Msg_t> packet;
	while (D->second->size())
	{
	      packet.push_back(D->second->front());
	      D->second->pop_front();
	}
        W.Put<P_Msg_t>(0,&packet); // stuff the Tinsel messages into the packet
	W.Send();                  // and away it goes.
     }
}

//------------------------------------------------------------------------------


// OnSend is stubbed for the moment. Will routing like this: though the base
// class, invoke the overloaded virtual functions in the derived Mothership
// class? To be tested when the time comes.
unsigned Mothership::OnSend(PMsg_p *msg, unsigned comm)
{
    return SBase::OnSend(msg, comm);
}

//------------------------------------------------------------------------------

unsigned Mothership::OnSuper(PMsg_p * Z, unsigned cIdx)
// Main hook for user-defined supervisor commands
{
// set up a return message if we are going to send some reply back
PMsg_p W(Comms[cIdx]);
W.Key(Q::SUPR);
W.Src(Z->Tgt());
int superReturn = 0;
 // Execute. Send a reply if one is expected
if ((superReturn = (*SupervisorCall)(Z,&W)) > 0)
{
   // either to Tinsels,
  if (!cIdx && (Z->Tgt() == Urank) && (Z->Src() == Urank)) OnTinsel(&W, 0);
  else W.Send(Z->Src());  // or to some external or internal process.
}
if (superReturn < 0) Post(530, int2str(Urank));
return 0;
}

//------------------------------------------------------------------------------

unsigned Mothership::OnSyst(PMsg_p * Z, unsigned cIdx)
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

unsigned Mothership::OnTinsel(PMsg_p * Z, unsigned cIdx)
// Handler for direct packets to be injected into the network from an external
// source
{
vector<P_Msg_t> msgs; // messages are packed in Tinsel message format
Z->Get(0, msgs);      // We assume they're directly placed in the message
WALKVECTOR(P_Msg_t, msgs, msg) // and they're sent blindly
{
   uint32_t Len = static_cast<uint32_t>(msg->header.messageLenBytes);
   uint32_t FlitLen = Len >> TinselLogBytesPerFlit;
   if (Len << (32-TinselLogBytesPerFlit)) ++FlitLen;
   // if we have to we can run OnIdle to empty receive buffers
   send(msg->header.destDeviceAddr, FlitLen, &(*msg), true);
}
return 0;
}

//------------------------------------------------------------------------------

unsigned Mothership::OnTinselOut(P_Sup_Msg_t * packet)
/* Deals with what happens when a Tinsel message is received. Generally we
   repack the message for delivery to the Supervisor handler and deal with
   it there. The Supervisor can do one of 2 things: A) process it itself,
   possibly generating another message; B) immediately export it over MPI to
   the user Executive or other external process.
*/
{
DebugPrint("Processing a command message 0x%x from Tinsel device %d\n",
           packet->header.command, packet->header.sourceDeviceAddr);
// handle the kill request from a tinsel core, which generally means an assert failed.
if ((packet->header.command == P_SUP_MSG_KILL)) return SystKill();
// output messages can simply be posted to the LogServer as an informational message.
if ((packet->header.command == P_SUP_MSG_LOG))
{
   DebugPrint("Received a handler_log message from device %d\n",
              packet->header.sourceDeviceAddr, p_sup_msg_size());
   // Just output the string (this will involve some rubbish at the end where
   // arguments would be; to be fixed later). Note that uint8_t*'s have to be
   // reinterpret_casted to char*s.
   unsigned msg_len = ((packet->header.cmdLenBytes%p_sup_msg_size()) &&
	(packet->header.seq == packet->header.cmdLenBytes/p_sup_msg_size())) ?
     packet->header.cmdLenBytes%p_sup_msg_size() : p_sup_msg_size()-p_sup_hdr_size();
   msg_len -= p_sup_hdr_size();
   Post(601, int2str(packet->header.sourceDeviceAddr),
	     int2str(packet->header.seq),
	     string(reinterpret_cast<const char*>(packet->data), msg_len));
   return 0;
}
DebugPrint("Message from device %d is a Supervisor call. Redirecting\n",
           packet->header.sourceDeviceAddr);
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

// Mothership sends. These forward packed groups of messages into the Tinsel
// network. Stubbed for the moment until we have external devices
// to test the send into a Mothership.

unsigned Mothership::SendAttr(PMsg_p *msg, unsigned comm)
{
   return SBase::SendAttr(msg, comm);
}

unsigned Mothership::SendDevIAll(PMsg_p *msg, unsigned comm)
{
   return SBase::SendDevIAll(msg, comm);
}

unsigned Mothership::SendDevIByName(PMsg_p *msg, unsigned comm)
{
   return SBase::SendDevIByName(msg, comm);
}

unsigned Mothership::SendDevIByID(PMsg_p *msg, unsigned comm)
{
   return SBase::SendDevIByID(msg, comm);
}

unsigned Mothership::SendDevT(PMsg_p *msg, unsigned comm)
{
   return SBase::SendDevT(msg, comm);
}

unsigned Mothership::SendExtn(PMsg_p *msg, unsigned comm)
{
   return SBase::SendExtn(msg, comm);
}

unsigned Mothership::SendSupv(PMsg_p *msg, unsigned comm)
{
   return SBase::SendSupv(msg, comm);
}

//------------------------------------------------------------------------------

void Mothership::StopTwig()
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

unsigned Mothership::SystHW(const vector<string>& args)
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

unsigned Mothership::SystKill()
// Kill some process in the local MPI universe. We had better be sure there is
// no traffic destined for this process after such a brutal command!
{
return 1;
}

//------------------------------------------------------------------------------

unsigned Mothership::SystShow()
// Report the list of processes (active Motherships) on this comm
{
return 0;
}

//------------------------------------------------------------------------------

unsigned Mothership::SystTopo()
// Report this Mothership's local topology information (boards/cores/connections)
{
return 0;
}

//------------------------------------------------------------------------------

void* Mothership::Twig(void* par)
// the Twig processing deals with receiving packets from the managed Tinsel cores.
// Ideally, Twig, OnIdle, and OnTinselOut would reside in a separate *process*.
// For the moment we run Twig in a separate *thread* because to run a separate
// process would require identifying the matching Branch's rank, which could be
// done, using a variety of methods, but none of them trivial, and so that's
// 'for later'.
{
     Mothership* parent = static_cast<Mothership*>(par);
     char* recv_buf = new char[p_msg_size()]; // buffer for one packet at a time
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
                 DebugPrint("Message received from a Device\n");
		 // this is the receive proper
		 // get the first word, which will be a device address
	         parent->recv(recv_buf);
	         uint32_t* device = static_cast<uint32_t*>(p_recv_buf);
	         if (!(*device & P_SUP_MASK)) // bound for an external?
	         {
	            P_Msg_Hdr_t* m_hdr = static_cast<P_Msg_Hdr_t*>(p_recv_buf);
                    DebugPrint("Message is bound for external device %d\n",
                               m_hdr->destDeviceAddr);
		 // has a message from this particular device been seen before?
		 // if not set up a new buffer for the device
	            if (parent->TwigExtMap[m_hdr->destDeviceAddr] == 0)
		       parent->TwigExtMap[m_hdr->destDeviceAddr] = new deque<P_Msg_t>;
	            parent->TwigExtMap[m_hdr->destDeviceAddr]->push_back(*(static_cast<P_Msg_t*>(p_recv_buf)));
		 }
	         else
		 // message to be consumed on this Mothership, either a system
		 // message or a Supervisor message
                 {
	            P_Sup_Hdr_t* s_hdr = static_cast<P_Sup_Hdr_t*>(p_recv_buf);
		    s_hdr->sourceDeviceAddr ^= P_SUP_MASK;

		    // debug message/watchdog: announces that this thread is
		    // still operational.
		    if (s_hdr->command == P_PKT_MSGTYP_ALIVE)
		    {
		       DebugPrint("Thread %d is still alive\n", s_hdr->sourceDeviceAddr >> P_THREAD_OS);
		    }
		    else
		    {
//------------------------------------------------------------------------------
		       // other messages are routed to the supervisor
		       DebugPrint("Message is a Supervisor request from device %d\n",
                                  s_hdr->sourceDeviceAddr);
		       // new device talking?
	               if (parent->TwigMap[s_hdr->sourceDeviceAddr] == 0)
		       {
			  // supervisors have their own internal device buffers
			  // (pins) for each device
		          DebugPrint("New device %d reporting to Supervisor\n",
                                     s_hdr->sourceDeviceAddr);
		          parent->TwigMap[s_hdr->sourceDeviceAddr] = new PinBuf_t;
		       }
		       // inactive pin for the device?
	               if ((*(parent->TwigMap[s_hdr->sourceDeviceAddr]))[s_hdr->destPin] == 0)
		       {
		          DebugPrint("New pin %d for device %d reporting to Supervisor\n",
				     s_hdr->destPin, s_hdr->sourceDeviceAddr);
                          (*(parent->TwigMap[s_hdr->sourceDeviceAddr]))[s_hdr->destPin] =
			     new char[MAX_P_SUP_MSG_BYTES]();
		       }
		       // convert the buffer into a supervisor message
                       P_Sup_Msg_t* recvdMsg =
			 static_cast<P_Sup_Msg_t*>(static_cast<void*>
			  ((*(parent->TwigMap[s_hdr->sourceDeviceAddr]))[s_hdr->destPin]));
		       // stuff message into the persistent buffer
	               memcpy(recvdMsg+s_hdr->seq,recv_buf,p_sup_msg_size());
	               if (super_buf_recvd(recvdMsg))
	               {
		          DebugPrint("Entire Supervisor message received of length %d\n", s_hdr->cmdLenBytes);
		          if (parent->OnTinselOut(recvdMsg))
			     parent->Post(530, int2str(parent->Urank));
		          super_buf_clr(recvdMsg);
	               }
		    }
                 }
           }
	   // Capture anything happening on the DebugLink - which is text output
	   // directed at a file.
	   bool updated = false;
	   if (OutFile)
	   {
	      fsetpos(OutFile, &writePos);
	      while (parent->pollStdOut(OutFile)) updated = true;
	      if (updated)
	      {
		 DebugPrint("Received a debug output message\n");
	      }
	      fgetpos(OutFile, &writePos);
	   }
	   // or possibly only dumped to the local console
	   else while (parent->pollStdOut());
	   // output the debug output buffer, which has to be done immediately
	   // because we are in a separate thread
           if (OutFile && updated)
           {
              fflush(OutFile);
	      fsetpos(OutFile, &readPos);
	      while (!feof(OutFile))
		     parent->Post(600, string(fgets(Line, 4*P_MSG_MAX_SIZE, OutFile)));
	      fgetpos(OutFile, &readPos);
	   }
     }
     DebugPrint("Exiting Twig thread\n");
     delete[] recv_buf;
     pthread_exit(par);
     return par;
}

//==============================================================================

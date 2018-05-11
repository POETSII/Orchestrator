//------------------------------------------------------------------------------

#include "TMoth.h"
#include <stdio.h>
#include <stream>
#include "Pglobals.h"
#include "flat.h"
#include "string.h"
#include "softswitch_common.h"
#include "limits.h"

const char* TMoth::MPISvc;
const int   TMoth::MPISrv;
const int   TMoth::NumBoards;
const unsigned char TMoth::TASK_BOOT;
const unsigned char TMoth::TASK_RDY;
const unsigned char TMoth::TASK_RUN;
const unsigned char TMoth::TASK_STOP;
const unsigned char TMoth::TASK_END;
const unsigned char TMoth::TASK_ERR;

//==============================================================================

TMoth::TMoth(int argc,char * argv[],string d) :
  OrchBase(argc,argv,d,string(__FILE__)), HostLink()
{
                                       // Load the incoming event map
(*FnMapx[0])[PMsg_p::KEY(Q::CMND        )] = &TMoth::OnCmnd;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME        )] = &TMoth::OnName;
(*FnMapx[0])[PMsg_p::KEY(Q::SUPR        )] = &TMoth::OnSuper;
(*FnMapx[0])[PMsg_p::KEY(Q::SYST        )] = &TMoth::OnSyst;
(*FnMapx[0])[PMsg_p::KEY(Q::TEST        )] = &TMoth::OnTest;
(*FnMapx[0])[PMsg_p::KEY(Q::TINS        )] = &TMoth::OnTinsel;

// mothership's address in POETS space will be its MPI rank for the box number,
// then the max board/core/thread ID and the supervisor flag.
PAddress = (Urank << P_BOX_OS) | P_BOARD_MASK | P_CORE_MASK | P_THREAD_MASK | P_SUP_MASK;
strcpy(MPIPort, "PORT_NULL");         // non-zero ranks don't have a port
if (!Urank)
{
   MPI_Open_port(MPI_INFO_NULL,MPIPort);   // Announce to MPI that we are open for business
   MPI_Publish_name(MPISvc,MPI_INFO_NULL,MPIPort); // and make us publicly available
}

void* args = this;             // spin off a thread to accept MPI connections
pthread_t MPI_accept_thread;   // from other universes
if (pthread_create(&MPI_accept_thread,NULL,Accept,args))
{
   // if this fails, the mothership will have to be contacted 'the hard way':
   // via direct SSH.
   fprintf(stdout,"Error creating thread to accept MPI connections\n");
   fflush(stdout);
}
MPISpinner();                          // Spin on *all* messages; exit on DIE
//printf("********* Root rank %d on the way out\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

TMoth::~TMoth()
{
//printf("********* Root rank %d destructor\n",Urank); fflush(stdout);

}

//------------------------------------------------------------------------------

void* TMoth::Accept(void* par)
// Blocking routine to connect to another MPI universe by publishing a port.
// The group of motherships is always considered the server side of the
// connection
{
TMoth* parent=static_cast<TMoth*>(par);
if (parent->URank==0) // set up the intercommunicator's leader on rank 0
{
MPI_Open_port(MPI_INFO_NULL,parent->MPIPort);
MPI_Publish_name(parent->MPISvc.c_str(),MPI_INFO_NULL,parent->MPIPort);
}
int* accepted = new int(parent->Connect(MPISrv,MPISvc));
pthread_exit(accepted)
return accepted;
}

//------------------------------------------------------------------------------

unsigned TMoth::Boot(string task)
{
   if (TaskMap.find(task) == TaskMap.end())
   {
       Post(107,task);
       return 1;
   }
   string taskStatus;
   switch(TaskMap[task]->status)
   {
   case TASK_BOOT:
   taskStatus = "TASK_BOOT";
   Post(513, task, taskStatus);
   return 2
   case TASK_RDY:
   uint32_t mX, mY, core, thread;
   // create a response bitmap to receive the various startup barrier messages.
   // do this before running 'go' so that we can receive the responses in one block.
   map<unsigned, vector<unsigned>*> t_start_bitmap;
   WALKMAP(unsigned,coreMap_t*,*(TaskMap[task]->boards),B)
   {
     WALKMAP(unsigned,unsigned,*(B->second),C)
     {
       t_start_bitmap[C->first] = new vector<unsigned>(ThreadMap[C->first]/(8*sizeof(unsigned)),UINT_MAX);
       unsigned remainder = ThreadMap[C->first]%(8*sizeof(unsigned));
       if (remainder) t_start_bitmap[C->first]->push_back(UINT_MAX >> ((8*sizeof(unsigned))-remainder));
     }
   }
   // actually boot the cores
   WALKMAP(unsigned,coreMap_t*,*(TaskMap[task]->boards),B)
   {
     WALKMAP(unsigned,unsigned,*(B->second),C)
     {
       fromAddr(C->first,&mX,&mY,&core,&thread);
       startOneCore(mX,mY,core,ThreadMap[C->first]);
       goOne(mX,mY,core);
     }
   }
   Ppkt_hdr_t barrier_msg;
   while (!t_start_bitmap.empty())
   {
     recvMsg(&barrier_msg, sizeof(Ppkt_hdr_t));
     if (barrier_msg.housekeeping[P_PKT_MSGTYP_OS] == P_PKT_MSGTYP_BARRIER)
     {
        fromAddr(barrier_msg.src,&mX,&mY,&core,&thread);
	unsigned core = toAddr(mx,mY,core,0);
	if (t_start_bitmap.find(core) != t_start_bitmap.end())
	{
	   (*t_start_bitmap[core])[thread/(8*sizeof(unsigned))] &= (~(1 << (thread%(8*sizeof(unsigned)))));
	   vector<unsigned>::iterator R;
	   for (R=t_start_bitmap[core]->begin(); R != t_start_bitmap[core]->end(); R++) if (*R) break;
	   if (R == t_start_bitmap[core]->end())
	   {
	      t_start_bitmap[core]->clear();
	      delete t_start_bitmap[core];
	      t_start_bitmap[core].erase();
	   }
	}
     }
   }
   MPI_Barrier(Comms[0]);         // barrier on the mothercore side
   TaskMap[task]->status = TASK_BARR; // now at barrier on the tinsel side.
   return 0;
   case TASK_RUN:
   taskStatus = "TASK_RUN";
   break;
   case TASK_STOP:
   taskStatus = "TASK_STOP";
   break;
   case TASK_END:
   taskStatus = "TASK_END";
   default:
   taskStatus = "TASK_ERR";
   }
   Post(511,task,"loaded",taskStatus);
   return 3;     
}

//------------------------------------------------------------------------------

unsigned TMoth::CmLoad(string task)
// Load a task to the system
{
   if (TaskMap.find(task) = TaskMap.end())
   {
      Post(515, task, int2str(Urank));
      return 0;
   }
   if (TaskMap.find(task) != TaskMap.end())
   {
      Post(511, task,"loaded",TaskMap[task]->status);
      return 0;
   }
   TaskMap[task]->status = TASK_BOOT;
   vector<pthread_t*> boot_threads;
   int coresThisTask = 0;
   int coresLoaded = 0;
   int coresPerBoard;
   int* pCoresPerBoard = &coresPerBoard;
   bool tOK = true;
   // for each board mapped to the task,
   WALKMAP(unsigned,coreMap_t*,*(TaskMap[task]->boards),B)
   {
      coresThisTask += B->second->size();
      // as long as threads can be successfully forked,
      if (tOK) 
      {
         // fork a thread to load the board 
         boot_threads.push_back(new pthread_t);
         if (pthread_create(boot_threads.back(),NULL,LoadBoard,this))
         {
	    // abort the load if a thread couldn't be forked 
	    delete boot_threads.back();
	    boot_threads.pop_back();
	    tOK = false;
         }
      }
   }
   // wait for everyone to finish
   WALKVECTOR(pthread_t*,boot_threads,H)
   {
      pthread_join(**H,&pCoresPerBoard);
      coresLoaded += coresPerBoard;
   }
   // abandoning the boot if load failed
   if (coresLoaded < coresThisTask)
   {
      TaskMap[task]->status = TASK_ERR;
      Post(516,int2str(B->first),task,int2str(coresLoaded));
      return 1;
   }
   TaskMap[task]->status = TASK_RDY;
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
   string taskStatus = "TASK_RDY";
   switch(TaskMap[task]->status)
   {
   case TASK_BOOT:
   taskStatus = "TASK_BOOT";
   case TASK_RDY:
   Post(513, task, taskStatus);
   return 1;
   case TASK_STOP:
   taskStatus = "TASK_STOP";
   break;
   case TASK_END:
   taskStatus = "TASK_END";
   break;
   case TASK_RUN:
   Post(511, task,"run","TASK_RUN");
   return 0;
   case TASK_BARR:
   uint32_t mX, mY, core, thread;
   Ppkt_hdr_t barrier_msg;
   barrier_msg.src = PAddress;
   barrier_msg.housekeeping[P_PKT_MSGTYP_OS] = P_PKT_MSGTYP_BARRIER;
   const unsigned barrier_base = (Urank << P_BOX_OS) | P_DEVICE_MASK;
   uint8_t flit[4 << TinselLogWordsPerFlit];
   // build a list of the threads in this task (that should be released from barrier)
   vector<pair<unsigned,unsigned>> threadsToRelease;
   // going through by board,
   WALKMAP(unsigned, coreMap_t*, *(TaskMap[task]->boards), B)
   {
      // core (assigned to the task),
      WALKMAP(unsigned, unsigned, *(B->second), C)
      {
         fromAddr(C->first,&mX,&mY,&core,&thread)
	 // and thread
	 for (thread = 0; thread < ThreadMap[C->first]; thread++)
	 {
	     threadsToRelease.push_back(pair(toAddr(mX,mY,core,thread),
		   barrier_base | (B->first << P_BOARD_OS) & P_BOARD_MASK |
		   (C->first << P_CORE_OS) & P_CORE_MASK |
		   (thread << P_THREAD_OS) & P_THREAD_MASK))
	 }  
      }
   }  					   
   while (!canSend()) if (canRecv()) recv(flit); // discard unexpected messages
   // and then issue the barrier release to the threads.
   WALKVECTOR(pair<unsigned,unsigned>, threadsToRelease, R)
   {
     barrier_msg.hdw = R->second;
     send(R->first,(sizeof(Ppkt_hdr_t)/(4 << TinselLogWordsPerFlit) + sizeof(Ppkt_hdr_t)%(4 << TinselLogWordsPerFlit) ? 1 : 0), &barrier_msg);
   }
   TaskMap[task]->status = TASK_RUN;
   return 0;
   default:
   taskStatus = "TASK_ERR";
   break;
   }
   Post(511,task,"run",taskStatus);
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
   string taskStatus = "TASK_RDY";
   switch(TaskMap[task]->status)
   {
   case TASK_IDLE:
   taskStatus = "TASK_IDLE";
   break;
   case TASK_BOOT:
   taskStatus = "TASK_BOOT";
   break;
   case TASK_RDY:
   Post(813, task, taskStatus);
   return 1;
   case TASK_BARR:
   // if we are at the barrier the simplest approach to
   // stop cleanly is simply to start and immediately stop.
   // thus as long as starting doesn't error, fall through
   // to the running condition immediately below.
   if (CmRun(task))
   {
      taskStatus = "TASK_ERR";
      break;
   }
   case TASK_RUN:
   TaskMap[task]->status = TASK_STOP;
   // set up for shutdown by creating a global stop message
   uint32_t mX, mY, core, thread;
   Ppkt_hdr_t stop_msg;
   stop_msg.src = PAddress;
   stop_msg.housekeeping[P_PKT_MSGTYP_OS] = P_PKT_MSGTYP_STOP;
   uint8_t flit[4 << TinselLogWordsPerFlit];
   // go through each board,
   WALKMAP(unsigned, coreMap_t*, *(TaskMap[task]->boards), B)
   {
      // core (assigned to the task),
      WALKMAP(unsigned, unsigned, *(B->second), C)
      {
         fromAddr(C->first,&mX,&mY,&core,&thread)
	 // and thread
	 for (thread = 0; thread < ThreadMap[C->first]; thread++)
	 {
	     stop_msg.hdw = toAddr(mX,mY,core,thread);
	     // swallow any stray packets
	     while (canRecv()) recv(flit);
	     while (!canSend()) if (canRecv()) recv(flit);
	     // then issue the stop packet
	     send(stop_msg.hdw,(sizeof(Ppkt_hdr_t)/(4 << TinselLogWordsPerFlit) + sizeof(Ppkt_hdr_t)%(4 << TinselLogWordsPerFlit) ? 1 : 0),&stop_msg);
	 }
      }
   }
   TaskMap[task]->status = TASK_END;
   return 0;
   case TASK_STOP:
   taskStatus = "TASK_STOP";
   break;
   case TASK_END:
   taskStatus = "TASK_END";
   break;
   default:
   taskStatus = "TASK_ERR";
   }
   Post(811,task,"stopped",taskStatus);
   return 2;                              
 }

//------------------------------------------------------------------------------

unsigned TMoth::CmTopo(Cli * pC)
{
pP->Cm(pC);                            // Hand the command line to node graph


return 0;
}

//------------------------------------------------------------------------------

void TMoth::Dump(FILE * fp)
{
fprintf(fp,"Root dump+++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"Event handler table:\n");
fprintf(fp,"Key        Method\n");
WALKMAP(unsigned,pMeth,FnMapx,i)
  fprintf(fp,"%#010x 0x%#010p\n",(*i).first,(*i).second);
fprintf(fp,"prompt    = %s\n",prompt);

fprintf(fp,"Root dump-----------------------------------\n");
fflush(fp);
CommonBase::Dump(fp);
}

//------------------------------------------------------------------------------

void* TMoth::LoadBoard(void* par)
{
      TMoth* parent = static_cast<TMoth*>(par);
      unsigned BIdx = NumBoards;
      int* numLoaded = new int(0);
      string task;
      WALKVECTOR(string,unsigned char,parent->TaskMap,K) // find our task
      {
	if (K->second->status == TASK_BOOT) task = K->first;
	break;
      }
      if (!task.empty()) // anything to do?
      {
         WALKMAP(unsigned,pthread_t*,parent->BootMap,B) // find our thread
         {
	    if (*(B->second) == pthread_self())
	    {
	       BIdx = B->first;
	       break;
	    }
         }
         if (BIdx >= NumBoards) // thread out of range
         {
	    Post(701,int2str(BIdx));
	    *numLoaded = -1;
         }
	 else if (parent->BoardMap.find(BIdx) == parent->BoardMap.end()) // board out of range
         {
	    Post(702,int2str(BIdx));
	    *numLoaded = -2;
         }
         if (*numLoaded < 0) // if we or some other thread errored, abandon the boot.
	 {
	    if (parent->TaskMap[task]->boards->find(BIdx) != parent->TaskMap[task]->boards->end()) // is this board mapped to this task?
	    {
	       WALKMAP(unsigned,unsigned,*((*parent->TaskMap[task]->boards)[BIdx]),C) // grab each core's code and data file
               {
		  string code_f(parent->BinPath[task] + "softswitch_code_" + int2str(C->second) + ".v");
	          string data_f(parent->BinPath[task] + "softswitch_data_" + int2str(C->second) + ".v");
	          uint32_t mX, mY, core, thread;
	          fromAddr(C->first, &mX, &mY, &core, &thread);
	          loadOne(code_f.c_str(), data_f.c_str(),mX, mY, core); // and boot the core
	          ++(*numLoaded);
               }
	    }
	 }
      }
      pthread_exit(numLoaded);
      return numLoaded;
}

//------------------------------------------------------------------------------

unsigned TMoth::MPISpinLoop()
{
   P_Pkt tinsel_packet;
   unsigned numFlits = 0;
   while (canRecv())
   {
     while (numFlits < ((sizeof(P_Pkt)>>TinselLogBytesPerFlit)) + ((sizeof(P_Pkt) & ~((~0) << TinselLogBytesPerFlit)) ? 1 : 0)) 
   }
}  

//------------------------------------------------------------------------------

unsigned TMoth::NameDist()
// Handler for a task command sent (probably) from the user.
{
switch (Z->Key())
{
// get the task that the command is going to operate on 
string task;
Z->get(0,task);
case PMsg_p::KEY(Q::CMND,Q::LOAD        ):
return CmLoad(task);
case PMsg_p::KEY(Q::CMND,Q::RUN         ):
return CmRun(task);
case PMsg_p::KEY(Q::CMND,Q::STOP        ):
return CmStop(task);
default:
Post(510,"nameserver",int2str(Urank));
return 0;
}
}

//------------------------------------------------------------------------------

unsigned TMoth::OnCmnd(PMsg_p * Z, unsigned cIdx)
// Handler for a task command sent (probably) from the user.
{
// get the task that the command is going to operate on 
string task;
Z->get(0,task);  
switch (Z->Key())
{
case PMsg_p::KEY(Q::CMND,Q::LOAD        ):
return CmLoad(task);
case PMsg_p::KEY(Q::CMND,Q::RUN         ):
return CmRun(task);
case PMsg_p::KEY(Q::CMND,Q::STOP        ):
return CmStop(task);
default:
Post(510,"task", int2str(Urank));
return 0;
}
}

//------------------------------------------------------------------------------

unsigned TMoth::OnName(PMsg_p * Z, unsigned cIdx)
// This handles what happens when the NameServer dumps a name subblock to the
// mothership
{
   if (Z->Key() == PMsg_p::KEY(Q::NAME,Q::TDIR))
   {
      string task, dir;
      Z->get(0,task);
      Z->get(1,dir);
      BinPath[task]=dir;
   }
   else if (Z->Key() == PMsg_p::KEY(Q::NAME,Q::DIST))
   {
   }
   else Post(510,"nameserver",int2str(Urank));
   return 0;
}

//------------------------------------------------------------------------------

unsigned TMoth::OnExit(PMsg_p * Z, unsigned cIdx)
// This is what happens when a user command to stop happens 
{
// We are going away. Shut down any active tasks.  
WALKMAP(string, unsigned char, TaskMap, Tsk)
{
       if ((tsk->second->status == TASK_BOOT) || (tsk->second->status == TASK_END)) continue;
       if (tsk->second->status == TASK_BARR) CmRun(tsk->first);
       if (tsk->second->status == TASK_RUN) CmStop(tsk->first);       
}
return 1;
}

//------------------------------------------------------------------------------

unsigned TMoth::OnSuper(PMsg_p * Z, unsigned cIdx)
// Main hook for user-defined supervisor commands
{
// set up a return message if we are going to send some reply back
PMsg_p W(Comms[cIdx]);
W.Key(Q::SUPR);
W.Src(Z.Tgt());
if (SupervisorCall(Z,&W) < 0) W.Send(Z.Src()); // Execute. Send a reply if one is expected
return 0;
}

//------------------------------------------------------------------------------

unsigned TMoth::OnSyst(PMsg_p * Z, unsigned cIdx)
// Handler for system commands sent by the user or operator
{
switch (Z->Key())
{
case PMsg_p::KEY(Q::SYST,Q::HARD        ):
vector<string> args;
Z->Get<vector<string>>(0,args);
if (SystHW(args))
{
   string cmd;
   WALKVECTOR(string,args,arg)
   {
     cmd+=(arg);
     cmd+=(' ');    
   }
   Post(520,int2str(Urank),cmd);
   return 0;
}
return 0;
case PMsg_p::KEY(Q::SYST,Q::KILL        ):
int kr;
Z->Get<int>(0,kr);
return 0;  
case PMsg_p::KEY(Q::SYST,Q::SHOW        ):
return 0;
case PMsg_p::KEY(Q::SYST,Q::TOPO        ):
return 0;
default:
Post(510,"system",in2str(Urank));
return 0;
}
}

//------------------------------------------------------------------------------

unsigned TMoth::OnTinsel(PMsg_p * Z, unsigned cIdx)
// Handler for an expanded full posted message back from the LogServer
//      LOG|FULL|   -|   - (1:int)message_id,
//                         (2:char)message_type,
//                         (3:string)full_message
{
string sfull;                          // Full message
Z->Get(3,sfull);
int id = 0;                            // Message id
int * pid = 0;
int cnt;
pid = Z->Get<int>(1,cnt);
if (pid!=0) id = *pid;
char ty = 0;                           // Message type
char * pty = 0;
pty = Z->Get<char>(2,cnt);
if (pty!=0) ty = *pty;
string t = GetTime();
printf(" %s: %3d(%c) %s\n",t.c_str(),id,ty,sfull.c_str());
fflush(stdout);
Prompt();
return 0;
}

//------------------------------------------------------------------------------

unsigned TMoth::ProcCmnd(Cli * pC)
// Take the monkey command and point it to the right place
// All known monkey input is tabulated in Pglobals.h,
// (along with all known message layouts).
{
if (pC==0) return 1;                   // Paranoia
pC->Trim();
string scmnd = pC->Co;                 // Pull out the command string
// Note you can't use .size() here, because some strings have been resized to 4,
// so for them the size includes the '\0', whereas the others, it doesn't.....
if (strcmp(scmnd.c_str(),"exit")==0) return CmExit(pC);
if (strcmp(scmnd.c_str(),"test")==0) return CmTest(pC);
if (strcmp(scmnd.c_str(),"syst")==0) return CmSyst(pC);
if (strcmp(scmnd.c_str(),"rtcl")==0) return CmRTCL(pC);
if (strcmp(scmnd.c_str(),"topo")==0) return CmTopo(pC);
if (strcmp(scmnd.c_str(),"task")==0) return CmTask(pC);
return CmDrop(pC);
}

//==============================================================================




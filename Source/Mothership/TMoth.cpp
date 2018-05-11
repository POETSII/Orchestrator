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

bool TMoth::CmSystPing(Cli::Cl_t * pCl)
//
{
if (pCl->Pa_v.size()==0) return (Post(48,"ping","system","1"));
for(unsigned k=0;k<4;k++) {
//Post(27,uint2str(k));
  WALKVECTOR(Cli::Pa_t,pCl->Pa_v,i) {  // Walk the parameter (ping) list
//    printf("%s\n",(*i).Val.c_str()); fflush(stdout);
    string tgt = (*i).Val;             // Class name to be pinged
    if (Cli::StrEq(tgt,Sderived)) continue;           // Can't ping yourself
    WALKVECTOR(ProcMap::ProcMap_t,pPmap->vPmap,j) {   // Walk the process list
      if (Cli::StrEq((*j).P_class,Sderived)) continue;// Still can't ping self
      if ((Cli::StrEq(tgt,(*j).P_class))||(tgt=="*")) {
        /* Qt whinges about the following 2 lines - doesn't like taking the
           address of a temporary. Presumably it is being more strict about
           compliance with standards. In any case, this means annoyingly creating
           2 silly extra string objects just to get their addresses.
           Pkt.Put(2,&string(GetDate())); // Never actually used these
           Pkt.Put(3,&string(GetTime()));
        */
        string tD(GetDate());
        string tT(GetTime());
        PMsg_p Pkt;
        Pkt.Put(1,&((*j).P_class));    // Target process name
        Pkt.Put(2,&tD); // Never actually used these
        Pkt.Put(3,&tT);
        Pkt.Put<unsigned>(4,&k);       // Ping attempt
        Pkt.Src(Urank);                // Sending rank
        Pkt.Key(Q::SYST,Q::PING,Q::REQ);
        Pkt.Send((*j).P_rank);
      }
    }
  }
}
return true;                              // Legitimate command exit
}

//------------------------------------------------------------------------------

bool TMoth::CmSystShow(Cli::Cl_t * pCl)
// Monkey wants the list of processes
{
vector<ProcMap::ProcMap_t> vprocs;
if (pPmap!=0) pPmap->GetProcs(vprocs);
Post(29,uint2str(vprocs.size()));
Post(30,Sproc);
printf("\n");
WALKVECTOR(ProcMap::ProcMap_t,vprocs,i) printf("Rank %d: %s\n",(*i).P_rank, (*i).P_proc.c_str());
printf("\n");
fflush(stdout);
return true;                              // Legitimate command exit
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
fprintf(fp,"Mothership dump+++++++++++++++++++++++++++++++++++\n");
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

unsigned TMoth::NameDist()
{
      return 0;
}

//------------------------------------------------------------------------------

unsigned TMoth::NameTdir(const string& task, const string& dir)
{
      BinPath[task] = dir;
      return 0;
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
Post(510,"Control",int2str(Urank));
return 0;
}
}

//------------------------------------------------------------------------------
void TMoth::OnIdle()
// idle processing deals with receiving packets from the managed Tinsel cores.
{
     const uint32_t szFlit = (1<<TinselLogBytesPerFlit);
     P_Pkt_t recv_buf; // buffer for one packet at a time
     char* recv_b_pos = static_cast<char*>(&recv_buf);
     // receive all available traffic. Should this be done or only one packet
     // and then try again for MPI? We don't expect MPI traffic to be intensive
     // and tinsel messsages 
     while (canRecv)
     {
           recv(recv_b_pos);
	   recv_b_pos+= szFlit;
	   uint32_t len = static_cast<uint32_t>(recv_buf.housekeeping[P_LEN_OS]);
	   if (len > szFlit && (len < sizeof(P_Pkt_t))) recvMsg(recv_b_pos, (len-szFlit)); // get the whole message
	   if (OnTinsel(recv_buf,0)) Post(530, int2str(Urank)); // may need to change if OnTinsel responds to MPI
	   recv_b_pos = static_cast<char*>(&recv_buf);
     }
}

//------------------------------------------------------------------------------

unsigned TMoth::OnName(PMsg_p * Z, unsigned cIdx)
// This handles what happens when the NameServer dumps a name subblock to the
// mothership
{
switch (Z->Key())
{
case PMsg_p::KEY(Q::NAME,Q::DIST         ):
return NameDist();
case PMsg_p::KEY(Q::NAME,Q::TDIR         ):
string task,dir;
Z->get(0,task);
Z->get(1,dir);
return NameTdir(task,dir)
default:
Post(510,"Name",int2str(Urank));
return 0;
}
}

//------------------------------------------------------------------------------

unsigned TMoth::OnExit(PMsg_p * Z, unsigned cIdx)
// This is what happens when a user command to stop happens 
{
// We are going away. Shut down any active tasks.  
WALKMAP(string, unsigned char, TaskMap, tsk)
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
if (SupervisorCall(this,Z,&W) < 0) W.Send(Z.Src()); // Execute. Send a reply if one is expected
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
Post(524,int2str(Z->Key()));
return 0;
}
}

//------------------------------------------------------------------------------

unsigned TMoth::OnTinsel(void * M, unsigned cIdx)
// Deals with what happens when a Tinsel message is received. Generally we
// repack the message for delivery to the Supervisor handler and deal with
// it there. The Supervisor can do one of 2 things: A) process it itself,
// possibly generating another message; B) immediately export it over MPI to
// the user Executive or other external process.
{
PMsg_p W(Comms[NameSCIdx()]);   // return packets to be routed via the NameServer's comm
W.Key(Q::SUPR);                 // it'll be a Supervisor packet
W.Src(pPmap->U.NameServer);     // coming from the NameServer
W.Tgt(Urank);                   // and directed at us
W.Put<P_Pkt_t>(0,M)             // stuff the Tinsel message into the packet
return OnSuper(W, NameSCIdx());
}

//------------------------------------------------------------------------------

unsigned TMoth::SystHW(const vector<string>& args)
// Execute some command directly on the Mothership itself. Unlike a Supervisor
// message, which triggers the Supervisor function but doesn't change the
// functionality of the Mothership, this can directly interact with the
// running Mothership to do certain tasks
{
return 0;
}

//------------------------------------------------------------------------------

unsigned TMoth::SystKill(unsigned rank)
// Kill some process in the local MPI universe. We had better be sure there is
// no traffic destined for this process after such a brutal command!
{
return 0;
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




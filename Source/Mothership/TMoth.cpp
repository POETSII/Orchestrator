//------------------------------------------------------------------------------

#include "TMoth.h"
#include <stdio.h>
#include <sstream>
#include "Pglobals.h"
#include "flat.h"
#include "string.h"
#include "limits.h"

const char* TMoth::MPISvc = "POETS_Box_Master";
const int   TMoth::MPISrv;
const int   TMoth::NumBoards;

//==============================================================================

TMoth::TMoth(int argc,char * argv[],string d) :
  CommonBase(argc,argv,d,string(__FILE__)), HostLink()
{
                                       // Load the incoming event map
(*FnMapx[0])[PMsg_p::KEY(Q::CMND        )] = &TMoth::OnCmnd;
(*FnMapx[0])[PMsg_p::KEY(Q::NAME        )] = &TMoth::OnName;
(*FnMapx[0])[PMsg_p::KEY(Q::SUPR        )] = &TMoth::OnSuper;
(*FnMapx[0])[PMsg_p::KEY(Q::SYST        )] = &TMoth::OnSyst;
(*FnMapx[0])[PMsg_p::KEY(Q::TINS        )] = &TMoth::OnTinsel;

// mothership's address in POETS space is the host thread ID: X coordinate 0,
// Y coordinate max.
PAddress = TinselMeshYLen << (TinselMeshXBits+TinselLogCoresPerBoard+TinselLogThreadsPerCore);
strcpy(MPIPort, "PORT_NULL");         // non-zero ranks don't have a port

void* args = this;             // spin off a thread to accept MPI connections
pthread_t MPI_accept_thread;   // from other universes
AcceptConns = true;
if (pthread_create(&MPI_accept_thread,NULL,Accept,args))
{
   // if this fails, the mothership will have to be contacted 'the hard way':
   // via direct SSH.
   AcceptConns = false;
   printf("Error creating thread to accept MPI connections\n");
   fflush(stdout);
}
// and create a thread (later will be a process) to deal with packets from
// tinsel cores.
pthread_t Twig_thread;
ForwardMsgs = true;
if (pthread_create(&Twig_thread,NULL,Twig,args))
{
   ForwardMsgs = false;
   printf("Couldn't create Twig thread on Mothership rank %d. Communication from POETS processors to external is disabled\n", Urank);
   fflush(stdout);
}
MPISpinner();                          // Spin on *all* messages; exit on DIE
//printf("********* Root rank %d on the way out\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

TMoth::~TMoth()
{
//printf("********* Mothership rank %d destructor\n",Urank); fflush(stdout);
WALKMAP(uint32_t,PinBuf_t*,TwigMap,D)
{
  WALKMAP(uint16_t,char*,(*(D->second)),P)
    delete[] P->second;
  delete D->second;
}
WALKMAP(string,TaskInfo_t*,TaskMap,T)
  delete T->second;
}

//------------------------------------------------------------------------------

void* TMoth::Accept(void* par)
// Blocking routine to connect to another MPI universe by publishing a port.
// The group of motherships is always considered the server side of the
// connection
{
TMoth* parent=static_cast<TMoth*>(par);
if (!parent->Urank) // set up the intercommunicator's leader on rank 0
{
MPI_Open_port(MPI_INFO_NULL,parent->MPIPort); // Announce to MPI that we are open for business
MPI_Publish_name(parent->MPISvc,MPI_INFO_NULL,parent->MPIPort); // and make us publicly available
}
while (parent->AcceptConns)
{
if (!parent->Connect(parent->MPISrv,parent->MPISvc))
{
   // Might try Post() but failure could indicate an unreliable MPI universe
   // so all bets are off about whether any MPI communication would actually succeed.
   printf("Error: attempt to connect to another MPI universe failed\n");
   parent->AcceptConns = false;
}
}
MPI_Unpublish_name(parent->MPISvc,MPI_INFO_NULL,parent->MPIPort);
MPI_Close_port(parent->MPIPort);
pthread_exit(par);
return par;
}

//------------------------------------------------------------------------------

unsigned TMoth::Boot(string task)
{
   if (TaskMap.find(task) == TaskMap.end())
   {
       Post(107,task);
       return 1;
   }
   switch(TaskMap[task]->status)
   {
   case TaskInfo_t::TASK_BOOT:
   {
   Post(513, task, TaskInfo_t::Task_Status.find(TaskMap[task]->status)->second);
   TaskMap[task]->status = TaskInfo_t::TASK_ERR;
   return 2;
   }
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
       t_start_bitmap[TMoth::GetHWAddr((*C)->addr)] = new vector<unsigned>(numThreads/(8*sizeof(unsigned)),UINT_MAX);
       unsigned remainder = numThreads%(8*sizeof(unsigned));
       if (remainder) t_start_bitmap[TMoth::GetHWAddr((*C)->addr)]->push_back(UINT_MAX >> ((8*sizeof(unsigned))-remainder));
   }
   // actually boot the cores
   WALKVECTOR(P_core*,taskCores,C)
   {
     for (vector<P_thread*>::iterator R = (*C)->P_threadv.begin(); R != (*C)->P_threadv.end(); R++)
     {
       fromAddr(TMoth::GetHWAddr((*R)->addr),&mX,&mY,&core,&thread);
       startOneCore(mX,mY,core,(*C)->P_threadv.size());
       goOne(mX,mY,core);
     }
   }
   P_Sup_Msg_t barrier_msg;
   while (!t_start_bitmap.empty())
   {
     recvMsg(&barrier_msg, sizeof(P_Sup_Hdr_t));
     if (barrier_msg.header.command == P_PKT_MSGTYP_BARRIER)
     {
        fromAddr(barrier_msg.header.sourceDeviceAddr,&mX,&mY,&core,&thread);
	unsigned core = toAddr(mX,mY,core,0);
	if (t_start_bitmap.find(core) != t_start_bitmap.end())
	{
	   (*t_start_bitmap[core])[thread/(8*sizeof(unsigned))] &= (~(1 << (thread%(8*sizeof(unsigned)))));
	   vector<unsigned>::iterator S;
	   for (S = t_start_bitmap[core]->begin(); S != t_start_bitmap[core]->end(); S++) if (*S) break;
	   if (S == t_start_bitmap[core]->end())
	   {
	      t_start_bitmap[core]->clear();
	      delete t_start_bitmap[core];
	   }
	}
     }
   }
   MPI_Barrier(Comms[0]);         // barrier on the mothercore side
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
   if (TaskMap.find(task) == TaskMap.end())
   {
      Post(515, task, int2str(Urank));
      return 0;
   }
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
   if (!((TaskMap[task]->status == TaskInfo_t::TASK_IDLE) || (TaskMap[task]->status == TaskInfo_t::TASK_END)))
   {
      Post(511,task,"loaded to hardware",TaskInfo_t::Task_Status.find(TaskMap[task]->status)->second);
      return 0;
   }
   TaskMap[task]->status = TaskInfo_t::TASK_BOOT;
   int coresThisTask = 0;
   int coresLoaded = 0;
   void* pCoresPerBoard;
   bool tOK = true;
   if (BootMap.size()) BootMap.clear(); // clear any detritus (danger: possible memory leak)
   // for each board mapped to the task,
   WALKVECTOR(P_board*,TaskMap[task]->BoardsForTask(),B)
   {
      // less work to compute as we go along than to call CoresForTask.size()
      coresThisTask += (*B)->P_corev.size();
      // as long as threads can be successfully forked,
      if (tOK) 
      {
         // fork a thread to load the board 
         BootMap.push_back(new pthread_t);
         if (pthread_create(BootMap.back(),NULL,LoadBoard,this))
         {
	    // abort the load if a thread couldn't be forked 
	    delete BootMap.back();
	    BootMap.pop_back();
	    Post(516,int2str((*B)->addr.A_board),task);
	    tOK = false;
         }
      }
   }
   // wait for everyone to finish. 
   WALKVECTOR(pthread_t*,BootMap,H)
   {
      pthread_join(**H,&pCoresPerBoard);
      int* iPCoresPerBoard = static_cast<int*>(pCoresPerBoard);
      coresLoaded += *iPCoresPerBoard;
      delete iPCoresPerBoard;
      
   }
   // abandoning the boot if load failed
   if (coresLoaded < coresThisTask)
   {
      TaskMap[task]->status = TaskInfo_t::TASK_ERR;
      Post(518,task,int2str(coresLoaded),int2str(coresThisTask));
      return 1;
   }
   TaskMap[task]->status = TaskInfo_t::TASK_RDY;
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
   uint32_t mX, mY, core, thread;
   P_Msg_Hdr_t barrier_msg;
   barrier_msg.messageLenBytes = sizeof(P_Msg_Hdr_t); // barrier is only a header. No payload.
   barrier_msg.destEdgeIndex = 0;                     // no edge index necessary.
   barrier_msg.destPin = P_SUP_PIN_SYS;               // it goes to the system pin
   barrier_msg.messageTag = P_MSG_TAG_INIT;           // and is of message type __init__.
   const unsigned barrier_base = (Urank << P_BOX_OS) | P_DEVICE_MASK;
   uint8_t flit[4 << TinselLogWordsPerFlit];
   // build a list of the threads in this task (that should be released from barrier)
   vector<unsigned> threadsToRelease;
   WALKVECTOR(P_thread*,TaskMap[task]->ThreadsForTask(),R)
     threadsToRelease.push_back(TMoth::GetHWAddr((*R)->addr));  					   
   while (!canSend()) if (canRecv()) recv(flit); // discard unexpected messages
   // and then issue the barrier release to the threads.
   WALKVECTOR(unsigned,threadsToRelease,R)
   {
     barrier_msg.destDeviceAddr = *R; // might be *R | P_SUP_MASK to identify a supervisor message
     send(*R,(sizeof(P_Msg_Hdr_t)/(4 << TinselLogWordsPerFlit) + sizeof(P_Msg_Hdr_t)%(4 << TinselLogWordsPerFlit) ? 1 : 0), &barrier_msg);
   }
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
   uint32_t mX, mY, core, thread;
   P_Msg_Hdr_t stop_msg;
   stop_msg.destEdgeIndex = 0;           // ignore edge index. Unused.
   stop_msg.destPin = P_SUP_PIN_SYS;     // goes to the system pin 
   stop_msg.messageTag = P_MSG_TAG_STOP; // with a stop message type
   uint8_t flit[4 << TinselLogWordsPerFlit];
   // go through each thread of the task,
   WALKVECTOR(P_thread*, TaskMap[task]->ThreadsForTask(),R)
   {
     stop_msg.destDeviceAddr = TMoth::GetHWAddr((*R)->addr); // see note under cmRun about possible | with P_SUP_MASK
     // swallow any stray packets
     while (canRecv()) recv(flit);
     while (!canSend()) if (canRecv()) recv(flit);
     // then issue the stop packet
     send(stop_msg.destDeviceAddr,(sizeof(P_Msg_Hdr_t)/(4 << TinselLogWordsPerFlit) + sizeof(P_Msg_Hdr_t)%(4 << TinselLogWordsPerFlit) ? 1 : 0),&stop_msg);
   }
   TaskMap[task]->status = TaskInfo_t::TASK_END;
   return 0;
   }
   case TaskInfo_t::TASK_STOP:
   case TaskInfo_t::TASK_END:
   break;
   default:
   TaskMap[task]->status = TaskInfo_t::TASK_ERR;
   }
   Post(811,task,"stopped",TaskInfo_t::Task_Status.find(TaskMap[task]->status)->second);
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
  fprintf(fp,"%#010x 0x%#010p\n",(*i).first,(*i).second);
}

fprintf(fp,"Mothership dump-----------------------------------\n");
fflush(fp);
CommonBase::Dump(fp);
}

//------------------------------------------------------------------------------

void* TMoth::LoadBoard(void* par)
{
      TMoth* parent = static_cast<TMoth*>(par);
      unsigned BIdx;
      int* numLoaded = new int(0);
      string task;
      WALKMAP(string,TaskInfo_t*,parent->TaskMap,K) // find our task
      {
	if (K->second->status == TaskInfo_t::TASK_BOOT) task = K->first;
	break;
      }
      if (!task.empty()) // anything to do?
      {
	 TaskInfo_t* task_map = parent->TaskMap[task];
	 for (BIdx = 0; BIdx < task_map->BoardsForTask().size(); BIdx++) // find our thread
         {
	     // only one task can be booted at once, and its bootloader indices correspond to the
	     // boards mapped to the box.
	     if (*(parent->BootMap[BIdx]) == pthread_self()) break;
         }
         if ((BIdx >= task_map->BoardsForTask().size()) || (BIdx >= NumBoards)) // thread out of range
         {
	    parent->Post(501,int2str(BIdx));
         }
         else
	 {
	    WALKVECTOR(P_core*,task_map->BoardsForTask()[BIdx]->P_corev,C) // grab each core's code and data file
            {
	       string code_f(task_map->BinPath + "softswitch_code_" + int2str((*(task_map->CoreMap))[*C]) + ".v");
	       string data_f(task_map->BinPath + "softswitch_data_" + int2str((*(task_map->CoreMap))[*C]) + ".v");
	       uint32_t mX, mY, core, thread;
	       parent->fromAddr(TMoth::GetHWAddr((*C)->addr), &mX, &mY, &core, &thread);
	       parent->loadOne(code_f.c_str(), data_f.c_str(),mX, mY, core); // and boot the core
	       ++(*numLoaded);
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
     char recv_buf[sizeof(P_Msg_t)]; // buffer for one packet at a time
     void* p_recv_buf = static_cast<void*>(recv_buf);
     while (parent->ForwardMsgs) // until told otherwise,
     {
           // receive all available traffic. Should this be done or only one packet
           // and then try again for MPI? We don't expect MPI traffic to be intensive
           // and tinsel messsages might be time-critical.
           while (parent->canRecv())
           {
                 parent->recv(recv_buf);
	         uint32_t* device = static_cast<uint32_t*>(p_recv_buf);
	         if (*device != (parent->PAddress & (~P_BOX_MASK))) // bound for an external?
	         {
	            P_Msg_Hdr_t* m_hdr = static_cast<P_Msg_Hdr_t*>(p_recv_buf);
	            if (parent->TwigExtMap[m_hdr->destDeviceAddr] == 0)
		       parent->TwigExtMap[m_hdr->destDeviceAddr] = new deque<P_Msg_t>;
	            if (m_hdr->messageLenBytes > szFlit)
		       parent->recvMsg(recv_buf+szFlit, m_hdr->messageLenBytes-szFlit);
	            parent->TwigExtMap[m_hdr->destDeviceAddr]->push_back(*(static_cast<P_Msg_t*>(p_recv_buf)));
		 }
	         else
                 {
	            P_Sup_Hdr_t* s_hdr = static_cast<P_Sup_Hdr_t*>(p_recv_buf);
	            if (parent->TwigMap[s_hdr->sourceDeviceAddr] == 0) // new device talking?
		       parent->TwigMap[s_hdr->sourceDeviceAddr] = new PinBuf_t;
	            if ((*(parent->TwigMap[s_hdr->sourceDeviceAddr]))[s_hdr->destPin] == 0) // inactive pin for the device?
                       (*(parent->TwigMap[s_hdr->sourceDeviceAddr]))[s_hdr->destPin] = new char[s_hdr->cmdLenBytes]();
                    P_Sup_Msg_t* recvdMsg = static_cast<P_Sup_Msg_t*>(static_cast<void*>((*(parent->TwigMap[s_hdr->sourceDeviceAddr]))[s_hdr->destPin])); 
	            memcpy(recvdMsg+s_hdr->seq,s_hdr,sizeof(P_Sup_Hdr_t)); // stuff header into the persistent buffer
                    uint32_t len = s_hdr->seq == s_hdr->cmdLenBytes/sizeof(P_Sup_Msg_t) ?  s_hdr->cmdLenBytes%sizeof(P_Sup_Msg_t) : sizeof(P_Sup_Msg_t); // more message to receive?
	            if (len > szFlit) parent->recvMsg(((recvdMsg+s_hdr->seq)->data), len-szFlit); // get the whole message
	            if (super_buf_recvd(recvdMsg))
	            {
		       if (parent->OnTinselOut(recvdMsg))
		          parent->Post(530, int2str(parent->Urank));
		       super_buf_clr(recvdMsg);
	            }
                 }
           }
     }
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
        int NameSrvComm = NameSCIdx();
        PMsg_p W(Comms[NameSrvComm]);   // return packets to be routed via the NameServer's comm
        W.Key(Q::TINS);                 // it'll be a Tinsel packet
        W.Tgt(pPmap[NameSrvComm]->U.NameServer);     // directed to the NameServer (or UserIO, when we have it)
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
return NameDist();
if (key ==  PMsg_p::KEY(Q::NAME,Q::TDIR         ))
{
   string task,dir;
   Z->Get(0,task);
   Z->Get(1,dir);
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
       if (tsk->second->status == TaskInfo_t::TASK_RUN) CmStop(tsk->first);       
}
// stop accepting new MPI connections and Tinsel messages
AcceptConns = false;
ForwardMsgs = false;
return 1;
}

//------------------------------------------------------------------------------

unsigned TMoth::OnSuper(PMsg_p * Z, unsigned cIdx)
// Main hook for user-defined supervisor commands
{
// set up a return message if we are going to send some reply back
PMsg_p W(Comms[cIdx]);
W.Key(Q::SUPR);
W.Src(Z->Tgt());
if (SupervisorCall(Z,&W) < 0) W.Send(Z->Src()); // Execute. Send a reply if one is expected
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
}

//------------------------------------------------------------------------------

unsigned TMoth::OnTinselOut(P_Sup_Msg_t * packet)
// Deals with what happens when a Tinsel message is received. Generally we
// repack the message for delivery to the Supervisor handler and deal with
// it there. The Supervisor can do one of 2 things: A) process it itself,
// possibly generating another message; B) immediately export it over MPI to
// the user Executive or other external process.
{
// handle the kill request from a tinsel core, which generally means an assert failed.
if ((packet->header.command == P_SUP_MSG_KILL)) return SystKill();
int NameSComm = NameSCIdx();
PMsg_p W(Comms[NameSComm]);   // return packets to be routed via the NameServer's comm
W.Key(Q::SUPR);                            // it'll be a Supervisor packet
W.Src(pPmap[NameSComm]->U.NameServer);     // coming from the NameServer
W.Tgt(Urank);                              // and directed at us
unsigned last_index = packet->header.cmdLenBytes/sizeof(P_Sup_Msg_t) + (packet->header.cmdLenBytes%sizeof(P_Sup_Msg_t) ? 1 : 0);
vector<P_Sup_Msg_t> pkt_v(packet,&packet[last_index]); // maybe slightly more efficient using the constructor
W.Put<P_Sup_Msg_t>(0,&pkt_v);    // stuff the Tinsel message into the packet
W.Send();                       // away it goes.
return 0;
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




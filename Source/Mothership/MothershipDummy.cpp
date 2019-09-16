#include "MothershipDummy.h"
#include "CMsg_p.h"
#include "Pglobals.h"
#include <stdio.h>
#include <climits>


MothershipDummy::MothershipDummy(int argc,char * argv[], string d) :
  SBase(argc,argv,d,string(__FILE__))
{
   FnMapx.push_back(new FnMap_t);       // create a default function table
                                        // Load the message map
  
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DIST         )] = &MothershipDummy::OnCfg;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::TDIR         )] = &MothershipDummy::OnCfg;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::BLD          )] = &MothershipDummy::OnCfg;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::RECL         )] = &MothershipDummy::OnCfg;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DEL          )] = &MothershipDummy::OnCfg;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::STATE        )] = &MothershipDummy::OnCfg;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::ALL )] = &MothershipDummy::OnDump;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::NM  )] = &MothershipDummy::OnDump;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::LIST        )] = &MothershipDummy::OnDump;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::NM  )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::ID  )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::ALL )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::NGRP)] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::IGRP)] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::NSUP)] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::ISUP)] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::NF  )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::TNF )] = &MothershipDummy::OnReply;  
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::SUPV        )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::SUPV,Q::NF  )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::SUPV,Q::TNF )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::EXTN        )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::EXTN,Q::NF  )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::EXTN,Q::TNF )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVT,Q::NM  )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVT,Q::IN  )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVT,Q::OUT )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVT,Q::NF  )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVT,Q::TNF )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::ATTR        )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::ATTR,Q::NF  )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::ATTR,Q::TNF )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::LIST        )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::LIST,Q::TNF )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::TASK        )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::TASK,Q::TNF )] = &MothershipDummy::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::LOAD                )] = &MothershipDummy::OnCmnd;
   (*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::RUN                 )] = &MothershipDummy::OnCmnd;
   (*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::STOP                )] = &MothershipDummy::OnCmnd;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DIST                )] = &MothershipDummy::OnName;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RECL                )] = &MothershipDummy::OnName;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::TDIR                )] = &MothershipDummy::OnName;
   (*FnMapx[0])[PMsg_p::KEY(Q::SUPR                        )] = &MothershipDummy::OnSuper;
   (*FnMapx[0])[PMsg_p::KEY(Q::SYST,Q::HARD                )] = &MothershipDummy::OnSyst;
   (*FnMapx[0])[PMsg_p::KEY(Q::SYST,Q::KILL                )] = &MothershipDummy::OnSyst;
   (*FnMapx[0])[PMsg_p::KEY(Q::SYST,Q::SHOW                )] = &MothershipDummy::OnSyst;
   (*FnMapx[0])[PMsg_p::KEY(Q::SYST,Q::TOPO                )] = &MothershipDummy::OnSyst;

   MPISpinner();                        // Spin on MPI messages; exit only on DIE
}

MothershipDummy::~MothershipDummy()
{                     
   WALKVECTOR(FnMap_t*,FnMapx,F)          // WALKVECTOR and WALKMAP are in macros.h (long include chain)
     delete *F;                           // get rid of derived class function tables
}

unsigned MothershipDummy::Connect(string svc)
{
   unsigned connErr = MPI_SUCCESS;
   // set up the connection in the base class
   if ((connErr = SBase::Connect(svc)) != MPI_SUCCESS) return connErr; 
   FnMapx.push_back(new FnMap_t); // add another function table in the derived class
   int fIdx=FnMapx.size()-1;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DIST         )] = &MothershipDummy::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::TDIR         )] = &MothershipDummy::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::BLD          )] = &MothershipDummy::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::RECL         )] = &MothershipDummy::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DEL          )] = &MothershipDummy::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::STATE        )] = &MothershipDummy::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::ALL )] = &MothershipDummy::OnDump;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::NM  )] = &MothershipDummy::OnDump;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::LIST        )] = &MothershipDummy::OnDump;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::NM  )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::ID  )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::ALL )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::NGRP)] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::IGRP)] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::NSUP)] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::ISUP)] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::NF  )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::TNF )] = &MothershipDummy::OnReply;  
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::SUPV        )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::SUPV,Q::NF  )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::SUPV,Q::TNF )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::EXTN        )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::EXTN,Q::NF  )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::EXTN,Q::TNF )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVT,Q::NM  )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVT,Q::IN  )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVT,Q::OUT )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVT,Q::NF  )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVT,Q::TNF )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::ATTR        )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::ATTR,Q::NF  )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::ATTR,Q::TNF )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::LIST        )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::LIST,Q::TNF )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::TASK        )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::TASK,Q::TNF )] = &MothershipDummy::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::CMND,Q::LOAD                )] = &MothershipDummy::OnCmnd;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::CMND,Q::RUN                 )] = &MothershipDummy::OnCmnd;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::CMND,Q::STOP                )] = &MothershipDummy::OnCmnd;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DIST                )] = &MothershipDummy::OnName;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RECL                )] = &MothershipDummy::OnName;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::TDIR                )] = &MothershipDummy::OnName;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::SUPR                        )] = &MothershipDummy::OnSuper;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::SYST,Q::HARD                )] = &MothershipDummy::OnSyst;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::SYST,Q::KILL                )] = &MothershipDummy::OnSyst;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::SYST,Q::SHOW                )] = &MothershipDummy::OnSyst;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::SYST,Q::TOPO                )] = &MothershipDummy::OnSyst;
   return MPI_SUCCESS;
}

unsigned MothershipDummy::CmLoad(string task)
{
   return 0;
}

unsigned MothershipDummy::CmRun(string task)
{
   return 0;
}

unsigned MothershipDummy::CmStop(string task)
{
   return 0;
}

unsigned MothershipDummy::OnCfg(PMsg_p *msg, unsigned comm)
{
   switch(msg->L(2))
   {
   case Q::DIST:
   return ConfigDistribute(msg, comm);
   case Q::TDIR:
   return SBase::ConfigDir(msg, comm);
   case Q::BLD:
   return SBase::ConfigBuild(msg, comm);
   case Q::RECL:
   return ConfigRecall(msg, comm);
   case Q::DEL:
   return SBase::ConfigDelete(msg, comm);
   case Q::STATE:
   return ConfigState(msg, comm);
   default:
   Post(700,uint2str(msg->Key()),int2str(Urank));
   return 0;
   }
}

unsigned MothershipDummy::OnCmnd(PMsg_p *msg, unsigned comm)
{
   // co-opt the task /init (which sends LOAD) command to trigger some
   // test queries.
   if (msg->L(1) == Q::LOAD)
   {
       PMsg_p dummyState(Comms[0]);
       string taskName;
       msg->Get(0,taskName);
       if (taskName.empty())
       {
	  Post(510,"Load",int2str(Urank));
	  return 0;
       }
       dummyState.Zname(0,taskName);
       dummyState.Zname(1,"Init");
       return ConfigState(&dummyState, 0);	 
   }
   // any other command is silently ignored.
   return 0;
}

unsigned MothershipDummy::OnDump(PMsg_p *msg, unsigned comm)
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

unsigned MothershipDummy::OnName(PMsg_p *msg, unsigned comm)
{
   return 0;
}

unsigned MothershipDummy::OnReply(PMsg_p *msg, unsigned comm)
{
   if ((msg->L(3) == Q::NF) || (msg->L(3) == Q::TNF)) return ReplyNotFound(msg,comm);
   switch(msg->L(2))
   {
   case Q::DEVI:  
   switch(msg->L(3))
   {
   case Q::NM:
   case Q::ID:
   return ReplyDevice(msg,comm);
   case Q::NSUP:
   case Q::ISUP:
   return ReplyDevSuper(msg,comm);
   case Q::ALL:
   case Q::NGRP:
   case Q::IGRP:
   return ReplyDevices(msg,comm);
   }
   break;
   case Q::SUPV:
   return ReplySupers(msg,comm);
   case Q::EXTN:
   return ReplyDevices(msg,comm);
   case Q::DEVT:
   return ReplyDevTypes(msg,comm);
   case Q::ATTR:
   return ReplyAttrs(msg,comm);
   case Q::LIST:
   return ReplyList(msg,comm);
   case Q::TASK:
   return ReplyTask(msg,comm);
   }
   Post(700,uint2str(msg->Key()),int2str(Urank));
   return ERR_NONFATAL;
}

unsigned MothershipDummy::OnSuper(PMsg_p *msg, unsigned comm)
{
   return 0;
}

unsigned MothershipDummy::OnSyst(PMsg_p *msg, unsigned comm)
{
   return 0;
}

unsigned MothershipDummy::SystHW(const vector<string>& args)
{
   return 0;
}

unsigned MothershipDummy::SystKill()
{
   return 0;
}

unsigned MothershipDummy::SystShow()
{
   return 0;
}

unsigned MothershipDummy::SystTopo()
{
   return 0;
}

void MothershipDummy::Dump(FILE *fp, string task)
{
   if (task.empty())
   {
      fprintf(fp,"MothershipDummy summary dump+++++++++++++++++++++++++++\n");
      SBase::Dump(fp);
      fprintf(fp,"MothershipDummy summary dump---------------------------\n");
   }
   else
   {
      fprintf(fp,"MothershipDummy dump+++++++++++++++++++++++++++++++++++\n");
      SBase::Dump(fp,task);
      fprintf(fp,"Mothership dump-----------------------------------\n");
   }
   CommonBase::Dump(fp);
}

unsigned MothershipDummy::ConfigDistribute(PMsg_p *msg, unsigned comm)
{
   unsigned err = SUCCESS;
   if ((err = SBase::ConfigDir(msg,comm))) return err;
   if ((err = SBase::ConfigDistribute(msg,comm))) return err;
   string taskName = msg->Zname(0);
   // temporarily we dump everything out to a file ---------------------------------------------
   FILE *dumpFile = fopen("MothershipDummy_dump.txt","w");
   fprintf(dumpFile, "------------------------------Dummy Mothership Dump-----------------------------\n");
   int aCount = 0;
   int* mAddr = msg->Get<int>(0,aCount);
   // SymAddr_t* mAddr = msg->Get<SymAddr_t>(0,aCount);
   if (!mAddr) Post(730,"NAME::CFG::DIST",int2str(Urank),"Mothership address");
   fprintf(dumpFile, "Mothership Address: %d\n", mAddr ? *mAddr : 0);
   CMsg_p distMsg(*msg);
   distMsg.Dump(dumpFile);
   Dump(dumpFile,taskName); 
   fclose(dumpFile);
   //--------------------------------------------------------------------------------------------
   PMsg_p stateSet(Comms[0]);      // set up state directly
   stateSet.Src(Urank);
   stateSet.Tgt(Urank);
   stateSet.Zname(0, taskName);
   stateSet.Zname(1, "Deployed"); // by creating a message to ourselves
   ConfigState(&stateSet, 0);     // and calling the state-transition function with it
   return err;
}

unsigned MothershipDummy::ConfigRecall(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0);
   vector<string> tasks;
   unsigned err = SUCCESS;
   if (taskName.empty())
   {
      if ((err = ListTask(tasks)))
      {
	 Post(721,"gett","list",int2str(Urank));
	 return err;
      }
   }
   else tasks.push_back(taskName);
   vector<string>::iterator task;
   for (task = tasks.begin(); task != tasks.end(); task++)
   {
     if ((err = DelTask(*task)))
       {
	  Post(721,"delet",*task,int2str(Urank));
	  err = ERR_NONFATAL;
       }
   }
   return err;
}

// for testing purposes we will coopt ConfigState on Dummy motherships
// to query the NameServer for data.
unsigned MothershipDummy::ConfigState(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0);
   TaskData_t task;
   unsigned err = GetTask(taskName, task);
   if (err) return SUCCESS; // No such task yet. We can ignore.
   if (task.State == Deployed)
   {
      string nextState = msg->Zname(1);
      const vector<Record_t>* deployedDevs;
      if (nextState == "Init") // Init state change triggers queries
      {
	 err = GetDevices(taskName, deployedDevs); // but only if devices are loaded
         if (!err) 
         {
	    // pick a subsample of 10 devices to run single queries on (or, at least,
	    // as many devices as we have if it's < 10)
	    unsigned devStep = deployedDevs->size()/10 == 0 ? 1 : deployedDevs->size()/10;
	    PMsg_p qry;
	    // find the NameServer's rank and comm
	    for (unsigned comm = 0; comm < pPmap.size(); comm++)
	    {	     
	        if (pPmap[comm]->U.NameServer != Q::NAP) // use the first match
		{
		   qry.comm = Comms[comm];
		   qry.Src(Urank);
		   qry.Tgt(pPmap[comm]->U.NameServer);
		   break;
	        } 
	    }
            if (comm == pPmap.size()) // no NameServer found. A severe error.
	    {
	       Post(711);
	       return ERR_INVALID_STATE;
	    }
	    // now send some dummy queries. Get a task list first.
	    qry.Key(Q::NAME,Q::QRY,Q::LIST);
	    // be sure to set up tags for each query sent so it can be matched
	    int tag = 1; // stay out of standard Orchestrator-land (this is an MPI tag)
	    if (qryMap.size()) tag = qryMap.rbegin()->first + 1;
	    if (tag > MPI_TAG_UB) tag = 1;
	    qry.Tag(tag);
	    qryMap[tag]=qry.Key();	    
	    qry.Send();
	    qry.L(2,Q::TASK); // now ask about the specific task we are on 
	    qry.Zname(0,taskName);
	    qry.Tag(++tag);        // these 3 lines are standard boilerplate to send 
	    qryMap[tag]=qry.Key(); // a query and update the tag map.
	    qry.Send();
	    qry.L(2,Q::DEVI);
	    // try all the device queries for a sample of devices (max 10)
	    for (unsigned dev = 0; dev < deployedDevs->size(); dev+=devStep)
	    {
	        qry.L(3,Q::NM); // by name
		qry.Zname(1,(*deployedDevs)[dev].Name);
		qry.Tag(++tag);
		qryMap[tag]=qry.Key();
	        qry.Send();
		qry.L(3,Q::NGRP); // in same group as named device
		qry.Tag(++tag);
		qryMap[tag]=qry.Key();
		qry.Send();
		qry.L(3,Q::NSUP); // supervisor of named device
		qry.Tag(++tag);
		qryMap[tag]=qry.Key();
		qry.Send();
		qry.L(3,Q::ID); // by ID
		qry.Zname(1,"");
		// another case where we have to copy into a temporary because
		// the device records are const-qualified in the return from
		// GetDevices.
		SymAddr_t devAddr = (*deployedDevs)[dev].Address;
		qry.Put<SymAddr_t>(0,&devAddr);
		qry.Tag(++tag);
		qryMap[tag]=qry.Key();
		qry.Send();
		qry.L(3,Q::IGRP); // in same group as device ID
		qry.Tag(++tag);
		qryMap[tag]=qry.Key();
                qry.Send();
		qry.L(3,Q::ISUP); // supervisor of device ID
		qry.Tag(++tag);
		qryMap[tag]=qry.Key();
		qry.Send();
	    }
	    qry.L(3,Q::ALL); // all devices
	    qry.Tag(++tag);
	    qryMap[tag]=qry.Key();
	    qry.Send();
	    qry.L(3,0);
	    qry.L(2,Q::SUPV); // all supervisors
            qry.Tag(++tag);
	    qryMap[tag]=qry.Key();
	    qry.Send();
	    qry.L(2,Q::EXTN); // all externals (should be none for now)
	    qry.Tag(++tag);
	    qryMap[tag]=qry.Key();
	    qry.Send();
	    if (task.AttributeTypes.size()) 
	    {
	       qry.L(2,Q::ATTR); // matching attributes, if any exist 
	       qry.PutX(2,&(task.AttributeTypes)); // just use all attributes
	       qry.Tag(++tag);
	       qryMap[tag]=qry.Key();
	       qry.Send();
	    }
	    if (!task.DeviceTypes.size()) // device types *should* exist,
	    {
	       // but if there are none send us a message and exit.
	       Post(706,taskName,int2str(Urank));
	       return ERR_NONFATAL;
	    }
	    qry.L(2,Q::DEVT); // of same device type
	    qry.L(3,Q::NM);
	    qry.Zname(2,task.DeviceTypes.front().Name);
	    qry.Tag(++tag);
	    qryMap[tag]=qry.Key();
	    qry.Send();
	    qry.L(3,Q::IN); // sharing inputs with device type
	    qry.Tag(++tag);
	    qryMap[tag]=qry.Key();
	    qry.Send();
	    qry.L(3,Q::OUT); // sharing outputs with device type
	    qry.Tag(++tag);
	    qryMap[tag]=qry.Key();
	    qry.Send();
	 } // if (!err)
      } // if (nextState == "Init")
   } // if (task.State == Deployed)
   return SBase::ConfigState(msg,comm); // (always) update state through base class
}

unsigned MothershipDummy::DumpAll(PMsg_p * msg, unsigned comm)
{
   return SBase::DumpAll(msg, comm);
}

unsigned MothershipDummy::DumpSummary(PMsg_p * msg, unsigned comm)
{
   return SBase::DumpSummary(msg, comm);
}

unsigned MothershipDummy::DumpTask(PMsg_p * msg, unsigned comm)
{
   return SBase::DumpTask(msg, comm);
}

void MothershipDummy::MapValue2Key(unsigned value, vector<byte>* key)
{
   uint32_t val = static_cast<uint32_t>(value);
   void* valPtr = static_cast<void*>(&val);
   // another reason to hate endianness: although the Key method in Msg_p
   // computes the key by successive left-shifting, the value as an
   // unsigned is stored little-endian. Which means that left shift
   // really means byte-wise right shift, from the POV of the underlying
   // representation in memory. So it's necessary to count back through
   // the unsigned value. 
   for (int b=sizeof(uint32_t)-1; b >= 0; b--)
       key->push_back(static_cast<byte*>(valPtr)[b]);
}

string MothershipDummy::QueryType(unsigned keyVal)
{
   vector<byte> key;
   MapValue2Key(keyVal, &key);
   if ((key[0] != Q::NAME) || ((key[1] != Q::QRY) && (key[1] != Q::RPLY))) return "Not a Query";
   switch (key[2])
   {
   case Q::DEVI:
   switch (key[3])
   {
   case Q::NM:
   return "Device By Name";
   case Q::ID:
   return "Device By ID";
   case Q::ALL:
   return "All Devices";
   case Q::NGRP:
   return "Devices in Same Group as Named Device";
   case Q::IGRP:
   return "Devices in Same Group as Device ID";
   case Q::NSUP:
   return "Supervisor of Named Device";
   case Q::ISUP:
   return "Supervisor of Device ID";
   default:
   return "Unrecognised Device Query";
   }
   case Q::SUPV:
   return "Supervisors";
   case Q::EXTN:
   return "External Devices";
   case Q::DEVT:
   switch (key[3])
   {
   case Q::NM:
   return "Devices of Type";
   case Q::IN:
   return "Devices With Input Message Type";
   case Q::OUT:
   return "Devices With Output Message Type";
   default:
   return "Unrecognised Type Query";
   }
   case Q::ATTR:
   return "Devices by Attribute";
   case Q::LIST:
   return "Task List";
   case Q::TASK:
   return "Task Info";
   default:
   return "Unrecognised Query";
   }			    
}

unsigned MothershipDummy::ReplyAttrs(PMsg_p *msg, unsigned comm)
{
   FILE *dumpFile = fopen("MothershipDummy_dump.txt","a");
   int qryTag = msg->Tag();
   map<int, unsigned>::iterator qryIt = qryMap.find(qryTag);
   if (qryIt == qryMap.end())
   {
      fprintf(dumpFile, "ERROR: reply to unknown query %x\n", qryTag);
      return ERR_NONFATAL;
   }
   vector<SymAddr_t> devAddrs;
   vector<string> devNames;
   vector<string> devAttrs;
   msg->Get<SymAddr_t>(0, devAddrs);
   msg->GetX(1, devNames);
   msg->GetX(2, devAttrs);
   unsigned numDevs = devAddrs.size();
   unsigned numNames = devNames.size();
   if (numDevs != numNames)
   {
      Post(716, int2str(Urank), uint2str(numNames), uint2str(numDevs), "addresse");
      numDevs = (numDevs > numNames) ? numNames : numDevs;
   }
   fprintf(dumpFile, "--------------------- Devices By Attribute -------------------------------------\n");
   for (unsigned dev = 0; dev < numDevs; dev++)
       fprintf(dumpFile, "Device %s: address 0x%llx\n", devNames[dev].c_str(), devAddrs[dev]);
   fprintf(dumpFile, "\n");
   fprintf(dumpFile, "Matched attributes:\n");
   for (unsigned attr = 0; attr < devAttrs.size(); attr++)
       fprintf(dumpFile, "%s\n", devAttrs[attr].c_str());
   fprintf(dumpFile, "________________________________________________________________________________\n");
   fclose(dumpFile);
   qryMap.erase(qryTag); // Reply handled; remove the query request
   return SUCCESS;	   
}

unsigned MothershipDummy::ReplyDevice(PMsg_p *msg, unsigned comm)
{
   FILE *dumpFile = fopen("MothershipDummy_dump.txt","a");
   int qryTag = msg->Tag();
   map<int, unsigned>::iterator qryIt = qryMap.find(qryTag);
   if (qryIt == qryMap.end())
   {
      fprintf(dumpFile, "ERROR: reply to unknown query %x\n", qryTag);
      return ERR_NONFATAL;
   }
   int numDevs;
   SymAddr_t* devAddr = msg->Get<SymAddr_t>(0, numDevs);
   if (!devAddr)
   {
      fprintf(dumpFile, "ERROR: device address missing for device %s query\n", msg->Zname(1).c_str());
      return ERR_NONFATAL;
   }
   string qryType = QueryType(msg->Key());
   fprintf(dumpFile, "--------------------- %s ----------------------------\n", qryType.c_str());
   fprintf(dumpFile, "Device %s: address 0x%llx\n", msg->Zname(1).c_str(), *devAddr);
   fprintf(dumpFile, "________________________________________________________________________________\n");
   fclose(dumpFile);
   qryMap.erase(qryTag); // Reply handled; remove the query request
   return SUCCESS;   
}

unsigned MothershipDummy::ReplyDevices(PMsg_p *msg, unsigned comm)
{
   FILE *dumpFile = fopen("MothershipDummy_dump.txt","a");
   int qryTag = msg->Tag();
   map<int, unsigned>::iterator qryIt = qryMap.find(qryTag);
   if (qryIt == qryMap.end())
   {
      fprintf(dumpFile, "ERROR: reply to unknown query %x\n", qryTag);
      return ERR_NONFATAL;
   }
   vector<SymAddr_t> devAddrs;
   vector<string> devNames;
   msg->Get<SymAddr_t>(0, devAddrs);
   msg->GetX(1, devNames);
   unsigned numDevs = devAddrs.size();
   unsigned numNames = devNames.size();
   if (numDevs != numNames)
   {
      Post(716, int2str(Urank), uint2str(numNames), uint2str(numDevs), "addresse");
      numDevs = (numDevs > numNames) ? numNames : numDevs;
   }
   string devName = msg->Zname(1);
   if (msg->L(2) == Q::EXTN) devName = "{Externals}";
   else if (msg->L(3) == Q::ALL) devName = "{All}";
   string qryType = QueryType(msg->Key());
   fprintf(dumpFile, "-----------  %s ----------------------------\n", qryType.c_str());
   fprintf(dumpFile, "Matching device %s\n", devName.c_str());
   for (unsigned dev = 0; dev < numDevs; dev++)
       fprintf(dumpFile, "Device %s: address 0x%llx\n", devNames[dev].c_str(), devAddrs[dev]);
   fprintf(dumpFile, "________________________________________________________________________________\n");
   fclose(dumpFile);
   qryMap.erase(qryTag); // Reply handled; remove the query request
   return SUCCESS;	   
}

unsigned MothershipDummy::ReplyDevSuper(PMsg_p *msg, unsigned comm)
{
   FILE *dumpFile = fopen("MothershipDummy_dump.txt","a");
   int qryTag = msg->Tag();
   map<int, unsigned>::iterator qryIt = qryMap.find(qryTag);
   if (qryIt == qryMap.end())
   {
      fprintf(dumpFile, "ERROR: reply to unknown query %x\n", qryTag);
      return ERR_NONFATAL;
   }
   int numDevs;
   SymAddr_t* devAddr = msg->Get<SymAddr_t>(0, numDevs);
   SymAddr_t* superAddr = msg->Get<SymAddr_t>(1, numDevs);
   if (!devAddr)
   {
      fprintf(dumpFile, "ERROR: device address missing for device %s supervisor query\n", msg->Zname(1).c_str());
      return ERR_NONFATAL;
   }
   if (!superAddr)
   {
      fprintf(dumpFile, "ERROR: device address missing for device %s supervisor query\n", msg->Zname(1).c_str());
      return ERR_NONFATAL;
   }
   string qryType = QueryType(msg->Key());
   fprintf(dumpFile, "--------------------- %s ----------------------------\n", qryType.c_str());
   fprintf(dumpFile, "Device %s: address 0x%llx, supervisor 0x%llx\n", msg->Zname(1).c_str(),*devAddr,*superAddr);
   fprintf(dumpFile, "________________________________________________________________________________\n");
   fclose(dumpFile);
   qryMap.erase(qryTag); // Reply handled; remove the query request
   return SUCCESS;   
}

unsigned MothershipDummy::ReplyDevTypes(PMsg_p *msg, unsigned comm)
{
   FILE *dumpFile = fopen("MothershipDummy_dump.txt","a");
   int qryTag = msg->Tag();
   map<int, unsigned>::iterator qryIt = qryMap.find(qryTag);
   if (qryIt == qryMap.end())
   {
      fprintf(dumpFile, "ERROR: reply to unknown query %x\n", qryTag);
      return ERR_NONFATAL;
   }
   vector<SymAddr_t> devAddrs;
   vector<string> devNames;
   msg->Get<SymAddr_t>(0, devAddrs);
   msg->GetX(1, devNames);
   unsigned numDevs = devAddrs.size();
   unsigned numNames = devNames.size();
   if (numDevs != numNames)
   {
      Post(716, int2str(Urank), uint2str(numNames), uint2str(numDevs), "addresse");
      numDevs = (numDevs > numNames) ? numNames : numDevs;
   }
   string matchName = msg->Zname(2);
   string matchType = "device";
   if (msg->L(3) == Q::IN) matchType = "input message";
   if (msg->L(3) == Q::OUT) matchType = "output message";
   string qryType = QueryType(msg->Key());
   fprintf(dumpFile, "-----------  %s ----------------------------\n", qryType.c_str());
   fprintf(dumpFile, "Matching %s type %s\n", matchType.c_str(), matchName.c_str());
   for (unsigned dev = 0; dev < numDevs; dev++)
       fprintf(dumpFile, "Device %s: address 0x%llx\n", devNames[dev].c_str(), devAddrs[dev]);
   fprintf(dumpFile, "________________________________________________________________________________\n");
   fclose(dumpFile);
   qryMap.erase(qryTag); // Reply handled; remove the query request
   return SUCCESS;	   
}

unsigned MothershipDummy::ReplyList(PMsg_p *msg, unsigned comm)
{
   FILE *dumpFile = fopen("MothershipDummy_dump.txt","a");
   int qryTag = msg->Tag();
   map<int, unsigned>::iterator qryIt = qryMap.find(qryTag);
   if (qryIt == qryMap.end())
   {
      fprintf(dumpFile, "ERROR: reply to unknown query %x\n", qryTag);
      return ERR_NONFATAL;
   }
   vector<string> tasks;
   msg->GetX(1,tasks);
   fprintf(dumpFile, "--------------------- Task List -------------------------------------\n");
   WALKVECTOR(string, tasks, task)
      fprintf(dumpFile, "%s\n", task->c_str());
   fprintf(dumpFile, "________________________________________________________________________________\n");
   fclose(dumpFile);
   qryMap.erase(qryTag); // Reply handled; remove the query request
   return SUCCESS;
}

unsigned MothershipDummy::ReplyNotFound(PMsg_p *msg, unsigned comm)
{
   FILE *dumpFile = fopen("MothershipDummy_dump.txt","a");
   int qryTag = msg->Tag();
   map<int, unsigned>::iterator qryIt = qryMap.find(qryTag);
   if (qryIt == qryMap.end())
   {
      fprintf(dumpFile, "ERROR: reply to unknown query %x\n", qryTag);
      return ERR_NONFATAL;
   }
   fprintf(dumpFile,"Reply to query %s on task %s: %s\n", QueryType(qryIt->second).c_str(),
	   msg->Zname(0).c_str(), msg->L(3) == Q::NF ? "Device Not Found" : "Task Not Found");
   fclose(dumpFile);
   qryMap.erase(qryTag); // Reply handled; remove the query request
   return SUCCESS;
}

unsigned MothershipDummy::ReplySupers(PMsg_p *msg, unsigned comm)
{
   FILE *dumpFile = fopen("MothershipDummy_dump.txt","a");
   int qryTag = msg->Tag();
   map<int, unsigned>::iterator qryIt = qryMap.find(qryTag);
   if (qryIt == qryMap.end())
   {
      fprintf(dumpFile, "ERROR: reply to unknown query %x\n", qryTag);
      return ERR_NONFATAL;
   }
   vector<SymAddr_t> supAddrs;
   vector<string> supNames;
   vector<unsigned long> supRanks;
   msg->Get<SymAddr_t>(0, supAddrs);
   msg->GetX(1, supNames);
   msg->Get<unsigned long>(2, supRanks);
   unsigned numDevs = supAddrs.size();
   unsigned numNames = supNames.size();
   unsigned numRanks = supRanks.size();
   if ((numDevs != numNames) || (numDevs != numRanks))
   {
      Post(737, int2str(Urank), uint2str(numNames), uint2str(numDevs), uint2str(numRanks));
      numDevs = (numDevs > numNames) ? numNames : numDevs;
      numDevs = (numDevs > numRanks) ? numRanks : numDevs;
   }
   fprintf(dumpFile, "--------------------- Supervisors -------------------------------------\n");
   for (unsigned dev = 0; dev < numDevs; dev++)
       fprintf(dumpFile, "Supervisor %s: address 0x%llx, rank %d\n",supNames[dev].c_str(),supAddrs[dev],supRanks[dev]);
   fprintf(dumpFile, "________________________________________________________________________________\n");
   fclose(dumpFile);
   qryMap.erase(qryTag); // Reply handled; remove the query request
   return SUCCESS;	   
}

unsigned MothershipDummy::ReplyTask(PMsg_p *msg, unsigned comm)
{
   FILE *dumpFile = fopen("MothershipDummy_dump.txt","a");
   int qryTag = msg->Tag();
   map<int, unsigned>::iterator qryIt = qryMap.find(qryTag);
   if (qryIt == qryMap.end())
   {
      fprintf(dumpFile, "ERROR: reply to unknown query %x\n", qryTag);
      return ERR_NONFATAL;
   }
   string task = msg->Zname(0);
   string state = msg->Zname(1);
   string tdir = msg->Zname(2);
   string fname = msg->Zname(3);
   vector<string> msgTypes;
   vector<string> attrTypes;
   vector<unsigned long> counts;
   msg->GetX(0,msgTypes);
   msg->GetX(1, attrTypes);
   msg->Get<unsigned long>(2, counts);
   if (counts.size() != 3)
   {
      fprintf(dumpFile, "ERROR: count mismatch for task info: expected 3 counts, received %u\n", counts.size());
      return ERR_NONFATAL;
   }
   fprintf(dumpFile, "-------------------------------- Task Info -------------------------------------\n");
   fprintf(dumpFile, "Task: %s\n", task.c_str());
   fprintf(dumpFile, "In state: %s\n", state.c_str());
   fprintf(dumpFile, "Task directory: %s\n", tdir.c_str());
   fprintf(dumpFile, "Source file name: %s\n", fname.c_str());
   fprintf(dumpFile, "---- Message types ----\n");
   WALKVECTOR(string, msgTypes, msgTyp)
      fprintf(dumpFile, "%s\n", msgTyp->c_str());
   fprintf(dumpFile, "_______________________\n");
   WALKVECTOR(string, attrTypes, attrTyp)
      fprintf(dumpFile, "%s\n", attrTyp->c_str());
   fprintf(dumpFile, "Number of devices: %lu, externals: %lu, supervisors %lu\n",counts[0],counts[1],counts[2]);
   fprintf(dumpFile, "________________________________________________________________________________\n");
   fclose(dumpFile);
   qryMap.erase(qryTag); // Reply handled; remove the query request
   return SUCCESS;
}


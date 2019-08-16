
#include "SBase.h"
#include "Pglobals.h"
#include <cstdio> // debug

SBase::SBase(int argc,char ** argv,string d,string s): CommonBase(argc,argv,d,s), AddressBook(d)
{
   FnMapx.push_back(new FnMap_t); // create a default function table (for the *nameserver base* class!)
   // Load event handler map
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::NM  )] = &SBase::OnSend;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::ID  )] = &SBase::OnSend;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::ALL )] = &SBase::OnSend;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::NGRP)] = &SBase::OnSend;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::IGRP)] = &SBase::OnSend;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::NSUP)] = &SBase::OnSend;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::ISUP)] = &SBase::OnSend;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::SUPV        )] = &SBase::OnSend;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::EXTN        )] = &SBase::OnSend;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVT,Q::NM  )] = &SBase::OnSend;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVT,Q::IN  )] = &SBase::OnSend;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVT,Q::OUT )] = &SBase::OnSend;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::ATTR        )] = &SBase::OnSend;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::NM   )] = &SBase::OnQuery;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::ID   )] = &SBase::OnQuery;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::ALL  )] = &SBase::OnQuery;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::NGRP )] = &SBase::OnQuery;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::IGRP )] = &SBase::OnQuery;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::NSUP )] = &SBase::OnQuery;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::ISUP )] = &SBase::OnQuery;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::LIST         )] = &SBase::OnQuery;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::TASK         )] = &SBase::OnQuery;   
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::SUPV         )] = &SBase::OnQuery;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::EXTN         )] = &SBase::OnQuery;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVT,Q::NM   )] = &SBase::OnQuery;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVT,Q::IN   )] = &SBase::OnQuery;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVT,Q::OUT  )] = &SBase::OnQuery;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::ATTR         )] = &SBase::OnQuery;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DIST         )] = &SBase::OnCfg;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::TDIR         )] = &SBase::OnCfg;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::BLD          )] = &SBase::OnCfg;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::RECL         )] = &SBase::OnCfg;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DEL          )] = &SBase::OnCfg;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::STATE        )] = &SBase::OnCfg;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI        )] = &SBase::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::NF  )] = &SBase::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::SUPV        )] = &SBase::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::SUPV,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::EXTN        )] = &SBase::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::EXTN,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVT        )] = &SBase::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVT,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::ATTR        )] = &SBase::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::ATTR,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::LIST        )] = &SBase::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::TASK        )] = &SBase::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::TASK,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DATA,Q::TASK        )] = &SBase::OnData;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DATA,Q::DEVT        )] = &SBase::OnData;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DATA,Q::DEVI        )] = &SBase::OnData;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DATA,Q::DEVE        )] = &SBase::OnData;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DATA,Q::EXTN        )] = &SBase::OnData;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DATA,Q::SUPV        )] = &SBase::OnData;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CMDC,Q::MONI,Q::ON  )] = &SBase::OnCmd;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CMDC,Q::MONI,Q::OFF )] = &SBase::OnCmd;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CMDC,Q::LOGN,Q::ON  )] = &SBase::OnCmd;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CMDC,Q::LOGN,Q::OFF )] = &SBase::OnCmd;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::ALL )] = &SBase::OnDump;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::NM  )] = &SBase::OnDump;
   (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::LIST        )] = &SBase::OnDump;
}
									   
SBase::~SBase()
{
   WALKVECTOR(FnMap_t*,FnMapx,F)          // WALKVECTOR and WALKMAP are in macros.h (long include chain)
     delete *F;                           // get rid of derived class function tables
}

unsigned SBase::OnCfg(PMsg_p *msg, unsigned comm)
{
   switch(msg->L(2))
   {
   case Q::DIST:
   return ConfigDistribute(msg, comm);
   case Q::TDIR:
   return ConfigDir(msg, comm);
   case Q::BLD:
   return ConfigBuild(msg, comm);
   case Q::RECL:
   return ConfigRecall(msg, comm);
   case Q::DEL:
   return ConfigDelete(msg, comm);
   case Q::STATE:
   return ConfigState(msg, comm);
   default:
   Post(700,uint2str(msg->Key()),int2str(Urank));
   return 0;
   }
}

unsigned SBase::OnCmd(PMsg_p *msg, unsigned comm)
{
   switch(msg->L(2))
   {
   case Q::INTG:
   return CmdIntegrity(msg);
   case Q::MONI:
   return 0;
   case Q::LOG:
   return 0;
   default:
   Post(700,uint2str(msg->Key()),int2str(Urank));
   return 0;
   }
}

unsigned SBase::OnData(PMsg_p *msg, unsigned comm)
{
   switch(msg->L(2))
   {
   case Q::TASK:
   return DataTask(msg);
   case Q::DEVT:
   return DataDevType(msg);
   case Q::DEVI:
   return DataDevice(msg);
   case Q::DEVE:
   return DataDeviceExternal(msg);
   case Q::EXTN:
   return DataExternal(msg);
   case Q::SUPV:
   return DataSupervisor(msg);
   default:
   Post(700,uint2str(msg->Key()),int2str(Urank));
   return 0;
   }
}

unsigned SBase::OnDump(PMsg_p *msg, unsigned comm)
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

unsigned SBase::OnQuery(PMsg_p *msg, unsigned comm)
{
   switch(msg->L(2))
   {
   case Q::DEVI:
   switch(msg->L(3))
   {
   case Q::NM:
   case Q::NGRP:
   case Q::NSUP:
   return QueryDevIByName(msg, comm);
   case Q::ID:
   case Q::IGRP:
   case Q::ISUP:
   return QueryDevIByID(msg, comm);
   case Q::ALL:
   return QueryDevIAll(msg, comm);
   default:
   Post(702,uint2str(static_cast<unsigned>(msg->L(3))),int2str(Urank));
   return 0;
   }
   case Q::SUPV:
   return QuerySupv(msg, comm);
   case Q::EXTN:
   return QueryExtn(msg, comm);
   case Q::DEVT:
   return QueryDevT(msg, comm);
   case Q::ATTR:
   return QueryAttr(msg, comm);
   case Q::LIST:
   return QueryList(msg, comm);
   case Q::TASK:
   return QueryTask(msg, comm);
   default:
   Post(700,uint2str(msg->Key()),int2str(Urank));
   return 0;
   }
}

unsigned SBase::OnReply(PMsg_p *msg, unsigned comm)
{
   Post(703,uint2str(msg->Key()),int2str(Urank));
   return 0;
}

unsigned SBase::OnSend(PMsg_p *msg, unsigned comm)
{
   switch(msg->L(2))
   {
   case Q::DEVI:
   switch(msg->L(3))
   {
   case Q::NM:
   case Q::NGRP:
   case Q::NSUP:
   return SendDevIByName(msg, comm);
   case Q::ID:
   case Q::IGRP:
   case Q::ISUP:
   return SendDevIByID(msg, comm);
   case Q::ALL:
   return SendDevIAll(msg, comm);
   default:
   Post(702,uint2str(static_cast<unsigned>(msg->L(3))),int2str(Urank));
   return 0;
   }
   case Q::SUPV:
   return SendSupv(msg, comm);
   case Q::EXTN:
   return SendExtn(msg, comm);
   case Q::DEVT:
   return SendDevT(msg, comm);
   case Q::ATTR:
   return SendAttr(msg, comm);
   default:
   Post(700,uint2str(msg->Key()),int2str(Urank));
   return 0;
   }
}

unsigned SBase::Connect(string svc)
{
   unsigned connErr = MPI_SUCCESS;
   if ((connErr = CommonBase::Connect(svc)) != MPI_SUCCESS) return connErr;
   FnMapx.push_back(new FnMap_t); // add another function table in the nameserver base class
   int fIdx=FnMapx.size()-1;
   // Load event handler map for the new comm
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::NM  )] = &SBase::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::ID  )] = &SBase::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::ALL )] = &SBase::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::NGRP)] = &SBase::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::IGRP)] = &SBase::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::NSUP)] = &SBase::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::ISUP)] = &SBase::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::SUPV        )] = &SBase::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::EXTN        )] = &SBase::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVT,Q::NM  )] = &SBase::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVT,Q::IN  )] = &SBase::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::DEVT,Q::OUT )] = &SBase::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::SEND,Q::ATTR        )] = &SBase::OnSend;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::NM   )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::ID   )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::ALL  )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::NGRP )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::IGRP )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::NSUP )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::ISUP )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::LIST         )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::TASK         )] = &SBase::OnQuery;  
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::SUPV         )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::EXTN         )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVT,Q::NM   )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVT,Q::IN   )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::DEVT,Q::OUT  )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::QRY,Q::ATTR         )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DIST         )] = &SBase::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::TDIR         )] = &SBase::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::BLD          )] = &SBase::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::RECL         )] = &SBase::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DEL          )] = &SBase::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::STATE        )] = &SBase::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI        )] = &SBase::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::NF  )] = &SBase::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::SUPV        )] = &SBase::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::SUPV,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::EXTN        )] = &SBase::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::EXTN,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVT        )] = &SBase::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::DEVT,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::ATTR        )] = &SBase::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::ATTR,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::LIST        )] = &SBase::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::TASK        )] = &SBase::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RPLY,Q::TASK,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DATA,Q::TASK        )] = &SBase::OnData;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DATA,Q::DEVT        )] = &SBase::OnData;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DATA,Q::DEVI        )] = &SBase::OnData;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DATA,Q::DEVE        )] = &SBase::OnData;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DATA,Q::EXTN        )] = &SBase::OnData;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DATA,Q::SUPV        )] = &SBase::OnData;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CMDC,Q::MONI,Q::ON  )] = &SBase::OnCmd;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CMDC,Q::MONI,Q::OFF )] = &SBase::OnCmd;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CMDC,Q::LOGN,Q::ON  )] = &SBase::OnCmd;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CMDC,Q::LOGN,Q::OFF )] = &SBase::OnCmd;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::ALL )] = &SBase::OnDump;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::NM  )] = &SBase::OnDump;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::LIST        )] = &SBase::OnDump;
   return MPI_SUCCESS;
}

unsigned SBase::CmdIntegrity(PMsg_p *msg)
{
   return 0;
}

unsigned SBase::ConfigBuild(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0);
   vector<string> tasks;
   unsigned err = SUCCESS;
   if (taskName.empty())
   {
      if ((err = ListTask(tasks)))
      {
	 Post(721,"rebuild","list",int2str(Urank));
	 return err;
      }
   }
   else tasks.push_back(taskName);
   vector<string>::iterator task;
   for (task = tasks.begin(); task != tasks.end(); task++)
   {
       if (!(err = RebuildTask(*task)))
       {
          if (!(err = BuildMaps(*task)))
             err = BuildLink(*task);
       }
       if (err)
       {
	  Post(721,"rebuild",*task,int2str(Urank));
          return err;
       }
   }
   return err;	  
}
  
unsigned SBase::ConfigDelete(PMsg_p *msg, unsigned comm)
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
          return err;
       }
   }
   return err;	
}

unsigned SBase::ConfigDistribute(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0);
   if (TaskState(taskName,TaskState_t(Deployed)))
   {
      Post(724,taskName,int2str(Urank));
      return ERR_TASK_NOT_FOUND;
   }
   return SUCCESS;
}

unsigned SBase::ConfigDir(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0);
   string taskPath = msg->Zname(1);
   if (taskPath.empty()) Post(723,int2str(Urank));
   if (TaskExecPath(taskName,taskPath))
   {
      Post(724,taskName,int2str(Urank)); 
      return ERR_TASK_NOT_FOUND; 
   }
   return SUCCESS;			      
}

unsigned SBase::ConfigRecall(PMsg_p *msg, unsigned comm)
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
       if ((err = ClearTask(*task)))
       {
	  Post(721,"clear",*task,int2str(Urank));
          return err;
       }
       if ((err = TaskState(*task,Unknown)))
       {
	  Post(725,*task,int2str(Urank));
	  return err;
       }
   }
   return err;
}

unsigned SBase::ConfigState(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0);
   // State is an enum, which makes it more reliable to send it as a string.
   // Otherwise there is no easy way to be certain the value isn't out of range
   // (e.g. what if the enum is given non-contiguous values?)
   string newStateStr = msg->Zname(1);
   TaskState_t newState(Unknown);
   // Thus after creating a default unknown state we test each viable possibility
   if (newStateStr == "Loaded") newState = Loaded;
   else if (newStateStr == "Linked") newState = Linked;
   else if (newStateStr == "Built") newState = Built;
   else if (newStateStr == "Deployed") newState = Deployed;
   else if (newStateStr == "Init") newState = Init;
   else if (newStateStr == "Running") newState = Running;
   else if (newStateStr == "Finished") newState = Finished;
   if (TaskState(taskName,newState))
   {
      Post(724,taskName,int2str(Urank));
      return ERR_NONFATAL; // 
      // return ERR_TASK_NOT_FOUND;
   }
   return SUCCESS;
}

unsigned SBase::DataTask(PMsg_p *msg)
{
   TaskData_t task;
   task.Name = msg->Zname(0);
   task.Path = msg->Zname(1);
   task.XML = msg->Zname(2);
   if (task.Name == "")
   {
      Post(704, int2str(Urank));
      return AddressBook::ERR_INVALID_TASK;
   }
   msg->GetX(0, task.MessageTypes);
   msg->GetX(1, task.AttributeTypes);
   vector<unsigned long> counts;
   msg->Get(2, counts);
   if (counts.size() != 2) Post(720,uint2str(counts.size()),"2",task.Name,int2str(Urank));
   else
   {
      task.DeviceCount = counts[0];
      task.ExternalCount = counts[1];
   }
   return AddTask(task.Name, task);
}

unsigned SBase::DataDevType(PMsg_p *msg)
{
   DevTypeRecord_t deviceType;
   string taskName = msg->Zname(0);
   deviceType.Name = msg->Zname(2);
   if (taskName == "")
   {
      Post(705, int2str(Urank));
      return AddressBook::ERR_INVALID_TASK;
   }
   if (deviceType.Name == "")
   {
      Post(706, taskName, int2str(Urank));
      return AddressBook::ERR_INVALID_DEVTYPE;
   }
   msg->Get(0, deviceType.InMsgs);
   msg->Get(1, deviceType.OuMsgs);
   return AddDeviceType(taskName, deviceType);
}

unsigned SBase::DataDevice(PMsg_p *msg)
{
   // Post(715,int2str(Urank),int2str(msg->Src()),msg->Zname(0));
   string taskName = msg->Zname(0);
   if (taskName == "")
   {
      Post(705, int2str(Urank));
      return AddressBook::ERR_INVALID_TASK;
   }
   TaskData_t task;
   if (GetTask(taskName, task))
   {
      Post(708,"records",taskName,int2str(Urank));
      return AddressBook::ERR_TASK_NOT_FOUND;
   }
   string devTypName = msg->Zname(2);
   DTypeIdx devIdx = 0;
   while ((task.DeviceTypes[devIdx].Name != devTypName) && ++devIdx < task.DeviceTypes.size());
   if (devIdx >= task.DeviceTypes.size())
   {
      Post(718,taskName,int2str(Urank),devTypName);
      return AddressBook::ERR_INVALID_DEVTYPE;
   }
   vector<string> devNames;
   vector<SymAddr_t> devAddrs;
   int numSupers = 1; // get method expects a *reference* to the count
   SymAddr_t* devSuper = msg->Get<SymAddr_t>(3,numSupers);
   if (!devSuper)
   {
      // no supervisor found
      Post(718,taskName,int2str(Urank));
      return ERR_INVALID_SUPERVISOR;
   }
   vector<AttrIdx> devAttrs;
   msg->GetX(0, devNames);
   msg->Get(1, devAddrs);
   if (devNames.size() != devAddrs.size())
   {
      Post(716,int2str(Urank),uint2str(devNames.size()),uint2str(devAddrs.size()),"addresse");
      return ERR_DEVICE_DATA_MISMATCH;	   
   }
   msg->Get(2, devAttrs);
   if (devNames.size() != devAttrs.size())
   {
      Post(716,int2str(Urank),uint2str(devNames.size()),uint2str(devAttrs.size()),"attribute");
      return ERR_DEVICE_DATA_MISMATCH;	   
   }
   unsigned err = AddressBook::SUCCESS;
   for (unsigned d = 0; d < devNames.size(); d++)
   {
       Record_t device(devNames[d],
		       devAddrs[d],
		       *devSuper,
		       devIdx,
		       RecordType_t(Device),
		       devAttrs[d]);   
       if ((err = AddDevice(taskName, device)) != AddressBook::SUCCESS)
       {
	  if (ClearTask(taskName) != AddressBook::SUCCESS)
	  {
	     Post(708, device.Name, taskName, int2str(Urank));
	     return err;
	  }
	  Post(707, device.Name, taskName, int2str(Urank));
	  return err;
       }
   }
   if (TaskState(taskName) < Linked)
   {
      if ((err = TaskState(taskName,Linked)))
      {
	 Post(724,taskName,int2str(Urank));
	 return err;
      }
   }
   return AddressBook::SUCCESS;
}

// -----------------------------------------------------------------------------

unsigned SBase::DataDeviceExternal(PMsg_p *msg)
{
   return 0;
}

// -----------------------------------------------------------------------------

unsigned SBase::DataExternal(PMsg_p *msg)
{
   return 0;
}

// -----------------------------------------------------------------------------

unsigned SBase::DataSupervisor(PMsg_p *msg)
{
   // Post(715,int2str(Urank),int2str(msg->Src()),msg->Zname(0));
   string taskName = msg->Zname(0);
   if (taskName == "")
   {
      Post(705, int2str(Urank));
      return AddressBook::ERR_INVALID_TASK;
   }
   TaskData_t task;
   if (GetTask(taskName, task))
   {
      Post(708,"records",taskName,int2str(Urank));
      return AddressBook::ERR_TASK_NOT_FOUND;
   }
   string devTypName = msg->Zname(2);
   DTypeIdx devIdx = 0;
   while ((task.DeviceTypes[devIdx].Name != devTypName) && ++devIdx < task.DeviceTypes.size());
   if (devIdx >= task.DeviceTypes.size())
   {
      Post(718,taskName,int2str(Urank),devTypName);
      return AddressBook::ERR_INVALID_DEVTYPE;
   }
   vector<string> devNames;;
   vector<SymAddr_t> devAddrs;
   vector<unsigned long> devRanks;
   vector<AttrIdx> devAttrs;
   msg->GetX(0, devNames);
   // msg->Get(1, devData);
   msg->Get(1, devAddrs);
   if (devNames.size() != devAddrs.size())
   {
      Post(716,int2str(Urank),uint2str(devNames.size()),uint2str(devAddrs.size()),"address");
      return ERR_DEVICE_DATA_MISMATCH;	   
   }
   msg->Get(2, devAttrs);
   if (devNames.size() != devAttrs.size())
   {
      Post(716,int2str(Urank),uint2str(devNames.size()),uint2str(devAttrs.size()),"attribute");
      return ERR_DEVICE_DATA_MISMATCH;	   
   }
   msg->Get(3, devRanks);
   if (devNames.size() != devRanks.size())
   {
      Post(716,int2str(Urank),uint2str(devNames.size()),uint2str(devRanks.size()),"rank");
      return ERR_DEVICE_DATA_MISMATCH;	   
   } 
   for (unsigned d = 0; d < devNames.size(); d++)
   {
       Record_t device(devNames[d],
		       devAddrs[d],
		       static_cast<uint64_t>(devRanks[d]),
		       devIdx,
		       RecordType_t(Supervisor),
		       devAttrs[d]);   
       unsigned err = AddressBook::SUCCESS;
       if ((err = AddDevice(taskName, device)) != AddressBook::SUCCESS)
       {
	  /*
	  if (ClearTask(taskName) != AddressBook::SUCCESS)
	  {
	     Post(708, device.Name, taskName, int2str(Urank));
	     return err;
	  }
          */
	  Post(707, device.Name, taskName, int2str(Urank));
	  // return err;
       }
   }
   // fclose(dbgSBase);
   return AddressBook::SUCCESS;
}

// -----------------------------------------------------------------------------

unsigned SBase::DumpAll(PMsg_p *msg, unsigned comm)
{
   // filename must be treated as an absolute literal, because tasks could in
   // general have different paths, therefore there is no way to infer what
   // additional path string might be appended to the file name give.
   string filename = msg->Zname(1);
   FILE* dumpFile;
   if (filename.empty()) dumpFile = stdout;
   else dumpFile = fopen(filename.c_str(), "a");
   vector<string> tasks;
   unsigned err = AddressBook::SUCCESS;
   if ((err = ListTask(tasks)))
   {
      Post(709, int2str(Urank));
      if (dumpFile != stdout) fclose(dumpFile);
      return err;
   }
   WALKVECTOR(string, tasks, task) Dump(dumpFile, *task);
   return err;
}

unsigned SBase::DumpSummary(PMsg_p *msg, unsigned comm)
{
   string filename = msg->Zname(1);
   FILE* dumpFile;
   if (filename.empty()) dumpFile = stdout;
   else dumpFile = fopen(filename.c_str(), "a");
   Dump(dumpFile);
   if (dumpFile != stdout) fclose(dumpFile);
   return AddressBook::SUCCESS;
}

unsigned SBase::DumpTask(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0);
   if (taskName.empty())
   {
     Post(710, "Dump", int2str(Urank));
      return AddressBook::ERR_INVALID_TASK; 
   }
   string filename = msg->Zname(1);
   FILE* dumpFile;
   if (filename.empty()) dumpFile = stdout;
   else dumpFile = fopen(filename.c_str(), "a");
   Dump(dumpFile, taskName);
   if (dumpFile != stdout) fclose(dumpFile);
   return AddressBook::SUCCESS;  
}

unsigned SBase::QueryAttr(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0);
   if (taskName.empty())
   {
      Post(710, "QueryByAttribute", int2str(Urank));
      return AddressBook::ERR_INVALID_TASK; 
   }
   unsigned err = SUCCESS;
   vector<string> attrs;
   msg->GetX(2,attrs);
   int srcProc = msg->Src();
   msg->comm = Comms[comm];
   msg->Src(msg->Tgt());
   msg->L(1,Q::RPLY);
   vector<SymAddr_t> devAddrs;
   vector<string> devNames;
   vector<string> devAttrs;
   WALKVECTOR(string, attrs, a)
   {
      const RecordVect_t* devRecords;
      if ((err = FindByAttribute(taskName, *a, devRecords)))
      {
	  if (err == ERR_TASK_NOT_FOUND || err == ERR_INVALID_MAP)
	  {
             msg->L(3, err == ERR_TASK_NOT_FOUND ? Q::TNF : Q::NF);
             msg->Send(srcProc);
             return SUCCESS;
	  }
	  continue;
      }
      devAttrs.push_back(*a);
      for (vector<const Record_t*>::const_iterator device = devRecords->begin(); device != devRecords->end(); device++)
      {
	  devAddrs.push_back((*device)->Address);
	  devNames.push_back((*device)->Name);
      }
   }	
   if (!devAddrs.size()) msg->L(3, Q::NF);
   else
   {
      msg->Put<SymAddr_t>(0,&devAddrs);
      msg->PutX(1,&devNames);
      msg->PutX(2,&devAttrs);
   }
   msg->Send(srcProc);
   return SUCCESS;
}

// Gets all devices for a task. We return every device in a single PMsg_p. This could get
// VERY big, but since there are no certainties the receiving device knows how many devices
// it's expecting, chunking the reply into bite-sized messages makes it impossible for the
// receiver to know when it can stop receiving. An alternative would be to insert the
// device count in the message, along with possibly a sequence number, but for a first
// revision this is just adding complication for possibly little benefit. To be tested.
unsigned SBase::QueryDevIAll(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0);
   if (taskName.empty())
   {
      Post(710, "QueryAllDevices", int2str(Urank));
      return AddressBook::ERR_INVALID_TASK; 
   }
   unsigned err = SUCCESS;
   int srcProc = msg->Src();
   msg->comm = Comms[comm];
   msg->Src(msg->Tgt());
   msg->L(1,Q::RPLY);
   vector<SymAddr_t> devAddrs;
   vector<string> devNames;
   const vector<Record_t>* devRecords;
   if ((err = GetDevices(taskName, devRecords)))
   {
      msg->L(3, err == ERR_TASK_NOT_FOUND ? Q::TNF : Q::NF);
      msg->Send(srcProc);
      return SUCCESS;
   }
   for (vector<Record_t>::const_iterator device = devRecords->begin(); device != devRecords->end(); device++)
   {
       devAddrs.push_back(device->Address);
       devNames.push_back(device->Name);
   }	
   if (!devAddrs.size()) msg->L(3, Q::NF);
   else
   {
      msg->Put<SymAddr_t>(0,&devAddrs);
      msg->PutX(1,&devNames);
   }
   msg->Send(srcProc);
   return SUCCESS;
}

// Query device by name. A question remains here - how do we deal with the queries
// asking for a group? 
unsigned SBase::QueryDevIByName(PMsg_p *msg, unsigned comm)
{
   if (!((msg->L(3) == Q::NGRP) || (msg->L(3) == Q::NM) || (msg->L(3) == Q::NSUP)))
   {
      Post(702,uint2str(msg->Key()),int2str(Urank));
      return ERR_NONFATAL;
   }
   string taskName = msg->Zname(0);
   if (taskName.empty())
   {
      Post(710, "QueryDeviceByName", int2str(Urank));
      return AddressBook::ERR_INVALID_TASK; 
   }
   string deviceName = msg->Zname(1);
   if (deviceName.empty())
   {
      Post(731, int2str(Urank));
      return ERR_INVALID_DEVICE;
   }
   unsigned err = SUCCESS;
   int srcProc = msg->Src();
   msg->comm = Comms[comm];
   msg->Src(msg->Tgt());
   msg->L(1,Q::RPLY);
   const Record_t* device;
   if ((err = FindDevice(taskName, deviceName, device)))
   {
      msg->L(3, err == ERR_TASK_NOT_FOUND ? Q::TNF : Q::NF);
      msg->Send(srcProc);
      return SUCCESS;
   }
   if (msg->L(3) == Q::NGRP)
   {
      vector<SymAddr_t> devAddrs;
      vector<string> devNames;
      const RecordVect_t* devRecords;
      if ((err = FindBySuper(taskName, device->Supervisor, devRecords)))
      {
	 // given that at least the lookup device must belong to its Supervisor, finding
	 // no devices attached to the Supervisor in question would be a serious error!
	 Post(732,int2str(Urank),uint2str(device->Supervisor),device->Name,err == ERR_TASK_NOT_FOUND ? "task" : "devices");
	 msg->L(3, err == ERR_TASK_NOT_FOUND ? Q::TNF : Q::NF);
         msg->Send(srcProc);
	 return ERR_DEVICE_DATA_MISMATCH;
      }
      for (vector<const Record_t*>::const_iterator deviceIt = devRecords->begin(); deviceIt != devRecords->end(); deviceIt++)
      {
          devAddrs.push_back((*deviceIt)->Address);
          devNames.push_back((*deviceIt)->Name);
      }
      if (!devAddrs.size()) msg->L(3, Q::NF);
      else
      {
         msg->Put<SymAddr_t>(0,&devAddrs);
         msg->PutX(1,&devNames);
      }
   }
   else
   {
      // these have to be copied into a temporary (and then recopied into the message)
      // because Put expects a type match, while the iterator returns constant records,
      // and Put itself must overwrite the actual value in its own data structure (not
      // the original) - but if its internal type is <const T>, it can't overwrite.
      SymAddr_t nonConstDevAddr = device->Address;
      SymAddr_t nonConstSuperAddr = device->Supervisor;
      msg->Put<SymAddr_t>(0,&nonConstDevAddr);
      if (msg->L(3) == Q::NSUP) msg->Put<SymAddr_t>(1,&nonConstSuperAddr);
   }
   msg->Send(srcProc);
   return SUCCESS;
}

unsigned SBase::QueryDevIByID(PMsg_p *msg, unsigned comm)
{
   if (!((msg->L(3) == Q::IGRP) || (msg->L(3) == Q::ID) || (msg->L(3) == Q::ISUP)))
   {
      Post(702,uint2str(msg->Key()),int2str(Urank));
      return ERR_NONFATAL;
   }
   string taskName = msg->Zname(0);
   if (taskName.empty())
   {
      Post(710, "QueryDeviceByID", int2str(Urank));
      return AddressBook::ERR_INVALID_TASK; 
   }
   int devCount = 0;
   SymAddr_t* deviceAddr = msg->Get<SymAddr_t>(0,devCount);
   if (!devCount)
   {
      Post(731, int2str(Urank));
      return ERR_INVALID_DEVICE;
   }
   unsigned err = SUCCESS;
   int srcProc = msg->Src();
   msg->comm = Comms[comm];
   msg->Src(msg->Tgt());
   msg->L(1,Q::RPLY);
   const Record_t* device;
   if ((err = FindDevice(taskName, *deviceAddr, device)))
   {
      msg->L(3, err == ERR_TASK_NOT_FOUND ? Q::TNF : Q::NF);
      msg->Send(srcProc);
      return SUCCESS;
   }
   if (msg->L(3) == Q::IGRP)
   {
      vector<SymAddr_t> devAddrs;
      vector<string> devNames;
      const RecordVect_t* devRecords;
      if ((err = FindBySuper(taskName, device->Supervisor, devRecords)))
      {
	 // given that at least the lookup device must belong to its Supervisor, finding
	 // no devices attached to the Supervisor in question would be a serious error!
	 Post(732,int2str(Urank),uint2str(device->Supervisor),device->Name,err == ERR_TASK_NOT_FOUND ? "task" : "devices");
	 msg->L(3, err == ERR_TASK_NOT_FOUND ? Q::TNF : Q::NF);
         msg->Send(srcProc);
	 return ERR_DEVICE_DATA_MISMATCH;
      }
      for (vector<const Record_t*>::const_iterator deviceIt = devRecords->begin(); deviceIt != devRecords->end(); deviceIt++)
      {
          devAddrs.push_back((*deviceIt)->Address);
          devNames.push_back((*deviceIt)->Name);
      }
      if (!devAddrs.size()) msg->L(3, Q::NF);
      else
      {
         msg->Put<SymAddr_t>(0,&devAddrs);
         msg->PutX(1,&devNames);
      }
   }
   else
   {
      msg->Zname(1,device->Name);
      if (msg->L(3) == Q::ISUP)
      {
	 SymAddr_t nonConstSuperAddr = device->Supervisor;
	 msg->Put<SymAddr_t>(1,&nonConstSuperAddr);
      }
   }
   msg->Send(srcProc);
   return SUCCESS;
}

unsigned SBase::QueryDevT(PMsg_p *msg, unsigned comm)
{
   if (!((msg->L(3) == Q::IN) || (msg->L(3) == Q::NM) || (msg->L(3) == Q::OUT)))
   {
      Post(702,uint2str(msg->Key()),int2str(Urank));
      return ERR_NONFATAL;
   }
   string taskName = msg->Zname(0);
   if (taskName.empty())
   {
      Post(710, "QueryDeviceByName", int2str(Urank));
      return AddressBook::ERR_INVALID_TASK; 
   }
   string lookupType = msg->Zname(2);
   if (lookupType.empty())
   {
      if (msg->L(3) == Q::NM)
      {
         Post(706, "find",taskName,int2str(Urank));
         return ERR_INVALID_DEVTYPE;
      }
      else
      {
	 Post(733,int2str(Urank));
	 return ERR_INVALID_MESSAGE_TYPE;
      }
       
   }
   unsigned err = SUCCESS;
   int srcProc = msg->Src();
   msg->comm = Comms[comm];
   msg->Src(msg->Tgt());
   msg->L(1,Q::RPLY);
   vector<SymAddr_t> devAddrs;
   vector<string> devNames;
   const RecordVect_t* devRecords;
   if (msg->L(3) == Q::NM) err = FindByType(taskName,lookupType,devRecords);
   else if (msg->L(3) == Q::IN) err = FindByInMsg(taskName,lookupType,devRecords);
   else err = FindByOuMsg(taskName,lookupType,devRecords);
   if (err)
   {
      msg->L(3, err == ERR_TASK_NOT_FOUND ? Q::TNF : Q::NF);
      msg->Send(srcProc);
      return SUCCESS;
   }
   for (vector<const Record_t*>::const_iterator device = devRecords->begin(); device != devRecords->end(); device++)
   {
       devAddrs.push_back((*device)->Address);
       devNames.push_back((*device)->Name);
   }	
   if (!devAddrs.size()) msg->L(3, Q::NF);
   else
   {
      msg->Put<SymAddr_t>(0,&devAddrs);
      msg->PutX(1,&devNames);
   }
   msg->Send(srcProc);
   return SUCCESS;
   
}

unsigned SBase::QueryExtn(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0);
   if (taskName.empty())
   {
      Post(710, "QueryAllExternals", int2str(Urank));
      return AddressBook::ERR_INVALID_TASK; 
   }
   unsigned err = SUCCESS;
   int srcProc = msg->Src();
   msg->comm = Comms[comm];
   msg->Src(msg->Tgt());
   msg->L(1,Q::RPLY);
   vector<SymAddr_t> devAddrs;
   vector<string> devNames;
   const vector<Record_t>* devRecords;
   if ((err = GetExternals(taskName, devRecords)))
   {
      msg->L(3, err == ERR_TASK_NOT_FOUND ? Q::TNF : Q::NF);
      msg->Send(srcProc);
      return SUCCESS;
   }
   for (vector<Record_t>::const_iterator device = devRecords->begin(); device != devRecords->end(); device++)
   {
       devAddrs.push_back(device->Address);
       devNames.push_back(device->Name);
   }	
   if (!devAddrs.size()) msg->L(3, Q::NF);
   else
   {
      msg->Put<SymAddr_t>(0,&devAddrs);
      msg->PutX(1,&devNames);
   }
   msg->Send(srcProc);
   return SUCCESS;
}

// ask for a list of tasks on this SBase. Can't fail: either we have tasks or not
// so we just return a list if we have them, a not found message if not.
unsigned SBase::QueryList(PMsg_p *msg, unsigned comm)
{
   unsigned err = SUCCESS;
   int srcProc = msg->Src();
   msg->comm = Comms[comm];
   msg->Src(msg->Tgt());
   msg->L(1,Q::RPLY);
   vector<string> taskNames;
   if ((err = ListTask(taskNames)) || (taskNames.size() == 0)) msg->L(3,Q::TNF);
   else msg->PutX(1,&taskNames);
   msg->Send(srcProc);
   return SUCCESS;
}

// Get all supervisors for a task. We have one more list in addition to the
// address and the name: the list of ranks, because the requesting process
// may wish to identify a Supervisor by MPI rank rather than POETS address.
unsigned SBase::QuerySupv(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0);
   if (taskName.empty())
   {
      Post(710, "QueryAllSupervisors", int2str(Urank));
      return AddressBook::ERR_INVALID_TASK; 
   }
   unsigned err = SUCCESS;
   int srcProc = msg->Src();
   msg->comm = Comms[comm];
   msg->Src(msg->Tgt());
   msg->L(1,Q::RPLY);
   vector<SymAddr_t> devAddrs;
   vector<string> devNames;
   vector<unsigned long> devRanks; 
   const vector<Record_t>* devRecords;
   if ((err = GetSupervisors(taskName, devRecords)))
   {
      msg->L(3, err == ERR_TASK_NOT_FOUND ? Q::TNF : Q::NF);
      msg->Send(srcProc);
      return SUCCESS;
   }
   for (vector<Record_t>::const_iterator device = devRecords->begin(); device != devRecords->end(); device++)
   {
       devAddrs.push_back(device->Address);
       devNames.push_back(device->Name);
       devRanks.push_back(device->Rank);
   }	
   if (!devAddrs.size()) msg->L(3, Q::NF);
   else
   {
      msg->Put<SymAddr_t>(0,&devAddrs);
      msg->PutX(1,&devNames);
      msg->Put<unsigned long>(2,&devRanks);
   }
   msg->Send(srcProc);
   return SUCCESS; 
}

// A task query might end up being some set of requests with further internal details
// in future, but for the moment we simply return summary task info.
unsigned SBase::QueryTask(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0);
   if (taskName.empty())
   {
      Post(710, "QueryTask", int2str(Urank));
      return AddressBook::ERR_INVALID_TASK; 
   }
   unsigned err = SUCCESS;
   int srcProc = msg->Src();
   msg->comm = Comms[comm];
   msg->Src(msg->Tgt());
   msg->L(1,Q::RPLY);
   TaskData_t task;
   if ((err = GetTask(taskName, task)))
   {
      msg->L(3,Q::TNF);
      msg->Send(srcProc);
      return SUCCESS;
   }
   switch (task.State)
   {
   case Loaded:
   msg->Zname(1,"Loaded");
   break;
   case Linked:
   msg->Zname(1,"Linked");
   break;
   case Built:
   msg->Zname(1,"Built");
   break;
   case Deployed:
   msg->Zname(1,"Deployed");
   break;
   case Init:
   msg->Zname(1,"Init");
   break;
   case Running:
   msg->Zname(1,"Running");
   break;
   case Finished:
   msg->Zname(1,"Finished");
   break;
   default:
   Post(734,taskName,int2str(Urank));
   msg->Zname(1,"Unknown");  
   }
   msg->Zname(2,task.Path);
   msg->Zname(3,task.XML);
   msg->PutX(0,&task.MessageTypes);
   msg->PutX(1,&task.AttributeTypes);
   vector<unsigned long> counts;
   counts.push_back(task.DeviceCount);
   counts.push_back(task.ExternalCount);
   counts.push_back(task.SupervisorCount);
   msg->Put<unsigned long>(2,&counts);
   msg->Send(srcProc);
   return SUCCESS;
}

unsigned SBase::SendAttr(PMsg_p *msg, unsigned comm)
{
   return 0;
}

unsigned SBase::SendDevIAll(PMsg_p *msg, unsigned comm)
{
   return 0;
}

unsigned SBase::SendDevIByName(PMsg_p *msg, unsigned comm)
{
   return 0;
}

unsigned SBase::SendDevIByID(PMsg_p *msg, unsigned comm)
{
   return 0;
}

unsigned SBase::SendDevT(PMsg_p *msg, unsigned comm)
{
   return 0;
}

unsigned SBase::SendExtn(PMsg_p *msg, unsigned comm)
{
   return 0;
}

unsigned SBase::SendSupv(PMsg_p *msg, unsigned comm)
{
   return 0;
}

string SBase::State2Str(TaskState_t state)
{
   switch (state)
   {
   case  Loaded:
   return "Loaded";
   case Linked:
   return "Linked";
   case Built:
   return "Built";
   case Deployed:
   return "Deployed";
   case Init:
   return "Init";
   case Running:
   return "Running";
   case Finished:
   return "Finished";
   }
   return "Unknown";
}

TaskState_t SBase::Str2State(const string& state)
{

   if (state == "Loaded") return Loaded;
   if (state == "Linked") return Linked;
   if (state == "Built") return Built;
   if (state == "Deployed") return Deployed;
   if (state == "Init") return Init;
   if (state == "Running") return Running;
   if (state == "Finished") return Finished;
   return Unknown;
}

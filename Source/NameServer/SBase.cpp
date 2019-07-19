#include "SBase.h"
#include "Pglobals.h"
#include <cstdio> // debug

SBase::SBase(int argc,char ** argv,string d,string s): CommonBase(argc,argv,d,s), AddressBook(d)
{
   FnMapx.push_back(new FnMap_t); // create a default function table (for the *nameserver base* class!)
   // Load event handler map
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::NM  )] = &SBase::OnSend;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::ID  )] = &SBase::OnSend;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::ALL )] = &SBase::OnSend;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::NGRP)] = &SBase::OnSend;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::IGRP)] = &SBase::OnSend;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::NSUP)] = &SBase::OnSend;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::ISUP)] = &SBase::OnSend;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::SEND,Q::SUPV        )] = &SBase::OnSend;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::SEND,Q::EXTN        )] = &SBase::OnSend;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVT,Q::NM  )] = &SBase::OnSend;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVT,Q::IN  )] = &SBase::OnSend;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVT,Q::OUT )] = &SBase::OnSend;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::SEND,Q::ATTR        )] = &SBase::OnSend;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::NM   )] = &SBase::OnQuery;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::ID   )] = &SBase::OnQuery;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::ALL  )] = &SBase::OnQuery;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::NGRP )] = &SBase::OnQuery;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::IGRP )] = &SBase::OnQuery;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::NSUP )] = &SBase::OnQuery;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::ISUP )] = &SBase::OnQuery;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::QRY,Q::SUPV         )] = &SBase::OnQuery;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::QRY,Q::EXTN         )] = &SBase::OnQuery;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVT,Q::NM   )] = &SBase::OnQuery;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVT,Q::IN   )] = &SBase::OnQuery;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVT,Q::OUT  )] = &SBase::OnQuery;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::QRY,Q::ATTR         )] = &SBase::OnQuery;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::CFG,Q::DIST         )] = &SBase::OnCfg;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::CFG,Q::TDIR         )] = &SBase::OnCfg;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::CFG,Q::BLD          )] = &SBase::OnCfg;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::CFG,Q::RECL         )] = &SBase::OnCfg;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::CFG,Q::DEL          )] = &SBase::OnCfg;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::CFG,Q::STATE        )] = &SBase::OnCfg;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI        )] = &SBase::OnReply;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::NF  )] = &SBase::OnReply;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::SUPV        )] = &SBase::OnReply;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::SUPV,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::EXTN        )] = &SBase::OnReply;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::EXTN,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::DEVT        )] = &SBase::OnReply;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::DEVT,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::ATTR        )] = &SBase::OnReply;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::ATTR,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::LIST        )] = &SBase::OnReply;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::TASK        )] = &SBase::OnReply;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::TASK,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::DATA,Q::TASK        )] = &SBase::OnData;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::DATA,Q::DEVT        )] = &SBase::OnData;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::DATA,Q::DEVI        )] = &SBase::OnData;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::DATA,Q::DEVE        )] = &SBase::OnData;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::DATA,Q::EXTN        )] = &SBase::OnData;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::DATA,Q::SUPV        )] = &SBase::OnData;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::CMDC,Q::MONI,Q::ON  )] = &SBase::OnCmd;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::CMDC,Q::MONI,Q::OFF )] = &SBase::OnCmd;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::CMDC,Q::LOGN,Q::ON  )] = &SBase::OnCmd;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::CMDC,Q::LOGN,Q::OFF )] = &SBase::OnCmd;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::ALL )] = &SBase::OnDump;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::NM  )] = &SBase::OnDump;
   (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::DUMP,Q::LIST        )] = &SBase::OnDump;
}
									   
SBase::~SBase()
{
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
   if (msg->L(2) == Q::LIST) return DumpSummary(msg);
   if (msg->L(2) != Q::TASK)
   {
      Post(700,uint2str(msg->Key()),int2str(Urank));
      return 0;
   }
   switch(msg->L(3))
   {
   case Q::ALL:
   return DumpAll(msg);
   case Q::NM:
   return DumpTask(msg);
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
   int fIdx=FnMapx.size()-1;
   // Load event handler map for the new comm
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::NM  )] = &SBase::OnSend;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::ID  )] = &SBase::OnSend;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::ALL )] = &SBase::OnSend;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::NGRP)] = &SBase::OnSend;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::IGRP)] = &SBase::OnSend;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::NSUP)] = &SBase::OnSend;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVI,Q::ISUP)] = &SBase::OnSend;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::SEND,Q::SUPV        )] = &SBase::OnSend;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::SEND,Q::EXTN        )] = &SBase::OnSend;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVT,Q::NM  )] = &SBase::OnSend;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVT,Q::IN  )] = &SBase::OnSend;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::SEND,Q::DEVT,Q::OUT )] = &SBase::OnSend;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::SEND,Q::ATTR        )] = &SBase::OnSend;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::NM   )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::ID   )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::ALL  )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::NGRP )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::IGRP )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::NSUP )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVI,Q::ISUP )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::QRY,Q::SUPV         )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::QRY,Q::EXTN         )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVT,Q::NM   )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVT,Q::IN   )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::QRY,Q::DEVT,Q::OUT  )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::QRY,Q::ATTR         )] = &SBase::OnQuery;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::CFG,Q::DIST         )] = &SBase::OnCfg;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::CFG,Q::TDIR         )] = &SBase::OnCfg;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::CFG,Q::BLD          )] = &SBase::OnCfg;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::CFG,Q::RECL         )] = &SBase::OnCfg;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::CFG,Q::DEL          )] = &SBase::OnCfg;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::CFG,Q::STATE        )] = &SBase::OnCfg;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI        )] = &SBase::OnReply;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::NF  )] = &SBase::OnReply;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::DEVI,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::SUPV        )] = &SBase::OnReply;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::SUPV,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::EXTN        )] = &SBase::OnReply;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::EXTN,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::DEVT        )] = &SBase::OnReply;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::DEVT,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::ATTR        )] = &SBase::OnReply;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::ATTR,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::LIST        )] = &SBase::OnReply;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::TASK        )] = &SBase::OnReply;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::RPLY,Q::TASK,Q::TNF )] = &SBase::OnReply;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::DATA,Q::TASK        )] = &SBase::OnData;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::DATA,Q::DEVT        )] = &SBase::OnData;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::DATA,Q::DEVI        )] = &SBase::OnData;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::DATA,Q::DEVE        )] = &SBase::OnData;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::DATA,Q::EXTN        )] = &SBase::OnData;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::DATA,Q::SUPV        )] = &SBase::OnData;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::CMDC,Q::MONI,Q::ON  )] = &SBase::OnCmd;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::CMDC,Q::MONI,Q::OFF )] = &SBase::OnCmd;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::CMDC,Q::LOGN,Q::ON  )] = &SBase::OnCmd;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::CMDC,Q::LOGN,Q::OFF )] = &SBase::OnCmd;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::ALL )] = &SBase::OnDump;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::NM  )] = &SBase::OnDump;
   (*FnMapx[fIdx])[Msg_p::KEY(Q::NAME,Q::DUMP,Q::LIST        )] = &SBase::OnDump;
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
      if (err = ListTask(tasks))
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
      if (err = ListTask(tasks))
      {
	 Post(721,"gett","list",int2str(Urank));
	 return err;
      }
   }
   else tasks.push_back(taskName);
   vector<string>::iterator task;
   for (task = tasks.begin(); task != tasks.end(); task++)
   {
       if (err = DelTask(*task))
       {
	  Post(721,"delet",*task,int2str(Urank));
          return err;
       }
   }
   return err;	
}

unsigned SBase::ConfigDistribute(PMsg_p *msg, unsigned comm)
{
   return 0;
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
      if (err = ListTask(tasks))
      {
	 Post(721,"gett","list",int2str(Urank));
	 return err;
      }
   }
   else tasks.push_back(taskName);
   vector<string>::iterator task;
   for (task = tasks.begin(); task != tasks.end(); task++)
   {
       if (err = ClearTask(*task))
       {
	  Post(721,"clear",*task,int2str(Urank));
          return err;
       }
       if (err = TaskState(*task,Unknown))
       {
	  Post(725,*task,int2str(Urank));
	  return err;
       }
   }
   return err;
}

unsigned SBase::ConfigState(PMsg_p *msg, unsigned comm)
{
   return 0;
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
   // FILE* dbgSBase = fopen("Root2NS_SBase_devices.txt","a"); // DEBUG ONLY
   vector<string> devNames;
   // vector<RecordData_t> devData;
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
   // if (devNames.size() != devData.size())
   // {
   //    Post(716,int2str(Urank),uint2str(devNames.size()),uint2str(devData.size()));
   //   return ERR_DEVICE_DATA_MISMATCH;	   
   // }
   /*
   fprintf(dbgSBase, "Nameserver received device info for %u devices\n", devNames.size());
   fprintf(dbgSBase, "\n");
   fprintf(dbgSBase, "________________________________________________________________________________\n");
   fprintf(dbgSBase, "\n");
   */
   unsigned err = AddressBook::SUCCESS;
   for (unsigned d = 0; d < devNames.size(); d++)
   {
       Record_t device(devNames[d],
		       devAddrs[d],
		       *devSuper,
		       devIdx,
		       RecordType_t(Device),
		       devAttrs[d]);   
       // Record_t device(devNames[d],
       //                 devData[d].Address,
       //                 devData[d].RecordType == Supervisor ? static_cast<uint64_t>(devData[d].Rank) : devData[d].Supervisor,
       //                 devData[d].DeviceType,
       //                 devData[d].RecordType,
       //                 devData[d].Attribute);
       /*
       fprintf(dbgSBase, "Device %s:\n", device.Name.c_str());
       fprintf(dbgSBase, "Address: 0x%.16llx\n", device.Address);
       if (device.RecordType == Supervisor)
	  fprintf(dbgSBase, "Supervisor Rank: %d\n", device.Rank);
       else fprintf(dbgSBase, "Supervisor Address: 0x%.16llx\n", device.Supervisor);
       fprintf(dbgSBase, "Device type index: %d\n", device.DeviceType);
       fprintf(dbgSBase, "Attributes: %d\n", device.Attribute);
       fprintf(dbgSBase, "Record type: %d\n", static_cast<unsigned>(device.RecordType));
       fprintf(dbgSBase, "\n");
       */
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
      if (err = TaskState(taskName,Linked))
      {
	 Post(724,taskName,int2str(Urank));
	 return err;
      }
   }
   // fclose(dbgSBase);
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
   // FILE* dbgSBase = fopen("Root2NS_SBase_devices.txt","a"); // DEBUG ONLY
   vector<string> devNames;
   // vector<RecordData_t> devData;
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
   // if (devNames.size() != devData.size())
   // {
   //    Post(716,int2str(Urank),uint2str(devNames.size()),uint2str(devData.size()));
   //   return ERR_DEVICE_DATA_MISMATCH;	   
   // }
   /*
   fprintf(dbgSBase, "Nameserver received supervisor info for %u supervisors\n", devNames.size());
   fprintf(dbgSBase, "\n");
   fprintf(dbgSBase, "________________________________________________________________________________\n");
   fprintf(dbgSBase, "\n");
   */
   for (unsigned d = 0; d < devNames.size(); d++)
   {
       Record_t device(devNames[d],
		       devAddrs[d],
		       static_cast<uint64_t>(devRanks[d]),
		       devIdx,
		       RecordType_t(Supervisor),
		       devAttrs[d]);   
       // Record_t device(devNames[d],
       //                 devData[d].Address,
       //                 devData[d].RecordType == Supervisor ? static_cast<uint64_t>(devData[d].Rank) : devData[d].Supervisor,
       //                 devData[d].DeviceType,
       //                 devData[d].RecordType,
       //                 devData[d].Attribute);
       /*
       fprintf(dbgSBase, "Device %s:\n", device.Name.c_str());
       fprintf(dbgSBase, "Address: 0x%.16llx\n", device.Address);
       if (device.RecordType == Supervisor)
	  fprintf(dbgSBase, "Supervisor Rank: %d\n", device.Rank);
       else fprintf(dbgSBase, "Supervisor Address: 0x%.16llx\n", device.Supervisor);
       fprintf(dbgSBase, "Device type index: %d\n", device.DeviceType);
       fprintf(dbgSBase, "Attributes: %d\n", device.Attribute);
       fprintf(dbgSBase, "Record type: %d\n", static_cast<unsigned>(device.RecordType));
       fprintf(dbgSBase, "\n");
       */
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

unsigned SBase::DumpAll(PMsg_p *msg)
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
   if (err = ListTask(tasks))
   {
      Post(709, int2str(Urank));
      if (dumpFile != stdout) fclose(dumpFile);
      return err;
   }
   WALKVECTOR(string, tasks, task) Dump(dumpFile, *task);
   return err;
}

unsigned SBase::DumpSummary(PMsg_p *msg)
{
   string filename = msg->Zname(1);
   FILE* dumpFile;
   if (filename.empty()) dumpFile = stdout;
   else dumpFile = fopen(filename.c_str(), "a");
   Dump(dumpFile);
   if (dumpFile != stdout) fclose(dumpFile);
   return AddressBook::SUCCESS;
}

unsigned SBase::DumpTask(PMsg_p *msg)
{
   string taskName = msg->Zname(0);
   if (taskName.empty())
   {
      Post(710, int2str(Urank));
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
   return 0;
}

unsigned SBase::QueryDevIAll(PMsg_p *msg, unsigned comm)
{
   return 0;
}

unsigned SBase::QueryDevIByName(PMsg_p *msg, unsigned comm)
{
   return 0;
}

unsigned SBase::QueryDevIByID(PMsg_p *msg, unsigned comm)
{
   return 0;
}

unsigned SBase::QueryDevT(PMsg_p *msg, unsigned comm)
{
   return 0;
}

unsigned SBase::QueryExtn(PMsg_p *msg, unsigned comm)
{
   return 0;
}

unsigned SBase::QueryList(PMsg_p *msg, unsigned comm)
{
   return 0;
}

unsigned SBase::QuerySupv(PMsg_p *msg, unsigned comm)
{
   return 0;
}

unsigned SBase::QueryTask(PMsg_p *msg, unsigned comm)
{
   return 0;
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

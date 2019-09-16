
#include "SBase.h"
#include "Pglobals.h"
#include <cstdio> // debug


// constructor. Arguments will almost always come from higher-level
// constructors since we expect to derive from SBase.
SBase::SBase(int argc,char ** argv,string d,string s): CommonBase(argc,argv,d,s), AddressBook(d)
{
   FnMapx.push_back(new FnMap_t); // create a default function table (for the *nameserver base* class!)
   // Load event handler map (we need to handle a fair number of different SBase requests!)
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

// destructor only needs to get rid of function tables.
SBase::~SBase()
{
   WALKVECTOR(FnMap_t*,FnMapx,F)          // WALKVECTOR and WALKMAP are in macros.h (long include chain)
     delete *F;                           // get rid of derived class function tables
}

//==============================================================================
/*
  OnCfg - handle Mothership configuration commands
*/
//==============================================================================
unsigned SBase::OnCfg(PMsg_p *msg, unsigned comm)
{
   switch(msg->L(2))
   {
   case Q::DIST:
   return ConfigDistribute(msg, comm); // send core data to Motherships 
   case Q::TDIR:
   return ConfigDir(msg, comm);        // send binary data directory to Motherships
   case Q::BLD:
   return ConfigBuild(msg, comm);      // rebuild this SBase's name database
   case Q::RECL:
   return ConfigRecall(msg, comm);     // recall (remove) a task from Motherships
   case Q::DEL:
   return ConfigDelete(msg, comm);     // delete a task from this SBase
   case Q::STATE:
   return ConfigState(msg, comm);      // Update task running state
   default:
   Post(700,uint2str(msg->Key()),int2str(Urank));
   return 0;
   }
}

//==============================================================================
/*
  OnCmd - handle low-level SBase commands
*/
//==============================================================================
unsigned SBase::OnCmd(PMsg_p *msg, unsigned comm)
{
   switch(msg->L(2))
   {
   case Q::INTG:
   return CmdIntegrity(msg); // integrity check (a long operation)
   case Q::MONI:
   return 0;                 // monitor function not yet implemented
   case Q::LOG:
   return 0;                 // nameserver logging not yet implemented
   default:
   Post(700,uint2str(msg->Key()),int2str(Urank));
   return 0;
   }
}

//==============================================================================
/*
  OnCmd - load name data to this SBase
*/
//==============================================================================
unsigned SBase::OnData(PMsg_p *msg, unsigned comm)
{
   switch(msg->L(2))
   {
   case Q::TASK:
   return DataTask(msg); // general task data. Needed before we can load devices.
   case Q::DEVT:
   return DataDevType(msg); // information on device types
   case Q::DEVI:
   return DataDevice(msg);  // device name-address mappings
   case Q::DEVE:
   return DataDeviceExternal(msg); // devices with an external connection
   case Q::EXTN:
   return DataExternal(msg);       // external device name-address mappings
   case Q::SUPV:
   return DataSupervisor(msg);     // supervisor address-MPI rank mappings
   default:
   Post(700,uint2str(msg->Key()),int2str(Urank));
   return 0;
   }
}

//==============================================================================
/*
  OnDump - dump out this SBase's database info, probably to a file
*/
//==============================================================================
unsigned SBase::OnDump(PMsg_p *msg, unsigned comm)
{
   if (msg->L(2) == Q::LIST) return DumpSummary(msg, comm); // just a task summary
   if (msg->L(2) != Q::TASK)
   {
      Post(700,uint2str(msg->Key()),int2str(Urank));
      return 0;
   }
   switch(msg->L(3))
   {
   case Q::ALL:
     return DumpAll(msg, comm);  // comprehensive info on all tasks
   case Q::NM:
     return DumpTask(msg, comm); // info only on a named task
   default:
   Post(700,uint2str(msg->Key()),int2str(Urank));
   return 0;
   }  
}

//==============================================================================
/*
  OnQuery - respond to name services queries (most of the functionality is here)
*/
//==============================================================================
unsigned SBase::OnQuery(PMsg_p *msg, unsigned comm)
{
   switch(msg->L(2))  // messages have several subkeys for queries
   {
   case Q::DEVI:                      // queries relating to devices
   switch(msg->L(3))
   {
   case Q::NM:                        // a single device
   case Q::NGRP:                      // devices in the same group as a device
   case Q::NSUP:                      // supervisor of device
   return QueryDevIByName(msg, comm); // query by name
   case Q::ID:                        // a single device
   case Q::IGRP:                      // devices in the same group as a device
   case Q::ISUP:                      // supervisor of device
   return QueryDevIByID(msg, comm);   // query by device ID
   case Q::ALL:                       // just get all (normal) devices
   return QueryDevIAll(msg, comm);
   default:
   Post(702,uint2str(static_cast<unsigned>(msg->L(3))),int2str(Urank));
   return 0;
   }
   case Q::SUPV:                     // query supervisors
   return QuerySupv(msg, comm);
   case Q::EXTN:                     // query externals
   return QueryExtn(msg, comm);
   case Q::DEVT:                     // query devices by a named type
   return QueryDevT(msg, comm);
   case Q::ATTR:                     // query devices by a labelled attribute
   return QueryAttr(msg, comm);     
   case Q::LIST:                     // get a basic list of tasks
   return QueryList(msg, comm);
   case Q::TASK:                     // get summary info on a task or all tasks
   return QueryTask(msg, comm);
   default:
   Post(700,uint2str(msg->Key()),int2str(Urank));
   return 0;
   }
}

//==============================================================================
/*
  OnReply - receive a name services response. SBase just immediately returns,
  because we expect this to be sent FROM an SBase TO some querying process.
*/
//==============================================================================
unsigned SBase::OnReply(PMsg_p *msg, unsigned comm)
{
   Post(703,uint2str(msg->Key()),int2str(Urank));
   return 0;
}

//==============================================================================
/*
  OnSend - forward packets to some set of devices. Not implemented to devices yet.
*/
//==============================================================================
unsigned SBase::OnSend(PMsg_p *msg, unsigned comm)
{
   // OnSend uses the same subtree as OnQuery, except we don't need to handle a
   // Q::TASK subkey. It will in practice execute a query and then forward
   // the message to a Mothership for further handling.
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

//==============================================================================
/*
  Connect - set up new function tables for handling messages from another MPI
  universe, such as, e.g. might happen if a group of Motherships were started
  and connected to the main Orchestrator universe as a service.
*/
//==============================================================================
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

//==============================================================================
/*
  CmdIntegrity - handle an integrity check request. Not implemented yet.
*/
//==============================================================================
unsigned SBase::CmdIntegrity(PMsg_p *msg)
{
   return 0;
}

//==============================================================================
/*
  ConfigBuild - rebuild SBase, probably because device data has changed
*/
//==============================================================================
unsigned SBase::ConfigBuild(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0); // get the task to be rebuilt
   vector<string> tasks;
   unsigned err = SUCCESS;
   if (taskName.empty())           // No task name => rebuild everything
   {
      if ((err = ListTask(tasks))) // Some tasks exist to rebuild?
      {
	 Post(721,"rebuild","list",int2str(Urank)); // No. Warn the user.
	 return err;
      }
   }
   else tasks.push_back(taskName); // a named task. Only rebuild that one.
   vector<string>::iterator task;
   // traverse the found task list,
   for (task = tasks.begin(); task != tasks.end(); task++) 
   {
       if (!(err = RebuildTask(*task))) // try to rebuild both the task
       {
	  if (!(err = BuildMaps(*task))) // and its crosslinks
             err = BuildLink(*task);
       }
       if (err) // something went wrong in the rebuild
       {
	  Post(721,"rebuild",*task,int2str(Urank)); // so warn the user.
          return err;
       }
   }
   return err;	  
}

//==============================================================================
/*
  ConfigDelete - remove a task from this SBase
*/
//==============================================================================
unsigned SBase::ConfigDelete(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0); // get task name
   vector<string> tasks;
   unsigned err = SUCCESS;
   if (taskName.empty())             // no task name => delete all tasks     
   {
      if ((err = ListTask(tasks)))   // so get all tasks that have been loaded
      {
	 Post(721,"gett","list",int2str(Urank)); // warning if there are none
	 return err;
      }
   }
   else tasks.push_back(taskName); // otherwise delete only the named task
   vector<string>::iterator task;
   for (task = tasks.begin(); task != tasks.end(); task++)
   {
     if ((err = DelTask(*task)))    // try to delete all tasks we asked for
       {
	  Post(721,"delet",*task,int2str(Urank)); // warning on failure
          return err;
       }
   }
   return err;	
}

//==============================================================================
/*
  ConfigDistribute - send core mappings to Motherships. Normally this will
  be handled from Root so SBase itself needs to do very little, only make
  sure the task state is changed to Deployed to reflect the new state of
  affairs.
*/
//==============================================================================
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

//==============================================================================
/*
  ConfigDir - send task binary directory path (mostly to Motherships). SBase
  keeps a copy of this, though, so this function does update it
*/
//==============================================================================
unsigned SBase::ConfigDir(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0); // first static name string is the task name
   string taskPath = msg->Zname(1); // and second name string is the directory path 
   if (taskPath.empty()) Post(723,int2str(Urank));
   if (TaskExecPath(taskName,taskPath))
   {
      Post(724,taskName,int2str(Urank)); 
      return ERR_TASK_NOT_FOUND; 
   }
   return SUCCESS;			      
}

//==============================================================================
/*
  ConfigRecall - remove knowledge of a task from a Mothership. This does not
  entirely delete the task from SBase or name services, but it does mean that]
  a task should be redeployed if it is to be run again.
*/
//==============================================================================
unsigned SBase::ConfigRecall(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0); // as usual, get the task name
   vector<string> tasks;
   unsigned err = SUCCESS;
   if (taskName.empty())            // where no name => recall all tasks
   {
      if ((err = ListTask(tasks)))  // which we should get if they exist
      {
	 Post(721,"gett","list",int2str(Urank)); // and warn if not
	 return err;
      }
   }
   else tasks.push_back(taskName);
   vector<string>::iterator task;
   for (task = tasks.begin(); task != tasks.end(); task++)
   {
       if ((err = ClearTask(*task))) // easiest way to reset the task on SBase 
	 {                           // is to clear it and reset it
	  Post(721,"clear",*task,int2str(Urank));
          return err;
       }
       if ((err = TaskState(*task,Unknown))) // a recalled task is in unknown state
       {                                     // until we redeploy or rebuild
	  Post(725,*task,int2str(Urank));
	  return err;
       }
   }
   return err;
}

//==============================================================================
/*
  ConfigState - SBases maintain copies of the task run state. After a task has
  been deployed, this corresponds to the Mothership's state for that task. 
  Before it has been deployed, it corresponds to the stage in the build 
  process that has been reached at Root.
*/
//==============================================================================
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

//==============================================================================
/*
  DataTask - Load this SBase with basic task data. A task is loaded from a 
  PMsg_p containing its name, path, XML filename, a vector of 
  message types (strings), a vector of attribute types (also strings), and 
  a pair of counts (how many devices/externals there are for the ENTIRE task).
*/
//==============================================================================
unsigned SBase::DataTask(PMsg_p *msg)
{
   TaskData_t task;
   task.Name = msg->Zname(0);  // extract all the basic information
   task.Path = msg->Zname(1);
   task.XML = msg->Zname(2);
   if (task.Name == "")        // tasks must have a name
   {
      Post(704, int2str(Urank));
      return AddressBook::ERR_INVALID_TASK;
   }
   msg->GetX(0, task.MessageTypes); // then get the message and attribute types
   msg->GetX(1, task.AttributeTypes);
   vector<unsigned long> counts;
   msg->Get(2, counts);             // and the device counts
   if (counts.size() != 2) Post(720,uint2str(counts.size()),"2",task.Name,int2str(Urank));
   else
   {
      task.DeviceCount = counts[0];
      task.ExternalCount = counts[1];
   }
   return AddTask(task.Name, task); // add the task (through AddressBook)
}

//==============================================================================
/*
  DataDevType - Load this task with DeviceType info. This is done as a 
  separate message to the rest of the basic task data because a DeviceType
  has more information about pins and messages than a simple string, so it's
  easier to separate it into its own message rather than serialising the 
  complexity into the basic task info message. Message types for in and out
  pins are sent as indices into the message types which were received in the
  previous task message, and which should also match the order of declaration
  in the XML.
*/
//==============================================================================
unsigned SBase::DataDevType(PMsg_p *msg)
{
   DevTypeRecord_t deviceType;
   string taskName = msg->Zname(0); // first static field is the task
   deviceType.Name = msg->Zname(2); // and third is the device type
   if (taskName == "")              // no task name is an error
   {
      Post(705, int2str(Urank));
      return AddressBook::ERR_INVALID_TASK;
   }
   if (deviceType.Name == "")       // similarly for no device type
   {
      Post(706, "load", taskName, int2str(Urank));
      return AddressBook::ERR_INVALID_DEVTYPE;
   }
   msg->Get(0, deviceType.InMsgs); // otherwise get the message type indices
   msg->Get(1, deviceType.OuMsgs);
   return AddDeviceType(taskName, deviceType);
}

//==============================================================================
/*
  DataDevice - Load this task with Device info. This is the main function to 
  populate the database. Device records may be sent in several such messages,
  although it is standard to send them all in one messaeg.
*/
//==============================================================================
unsigned SBase::DataDevice(PMsg_p *msg)
{
   // Post(715,int2str(Urank),int2str(msg->Src()),msg->Zname(0));
   string taskName = msg->Zname(0); // as usual, first static field is the task 
   if (taskName == "")
   {
      Post(705, int2str(Urank));
      return AddressBook::ERR_INVALID_TASK;
   }
   TaskData_t task;
   if (GetTask(taskName, task))     // does the task exist?
   {
      Post(708,"records",taskName,int2str(Urank)); // no. Abort.
      return AddressBook::ERR_TASK_NOT_FOUND;
   }
   string devTypName = msg->Zname(2); // device type name is in the third field
   DTypeIdx devIdx = 0;
   // search through loaded task info for the device type
   while ((task.DeviceTypes[devIdx].Name != devTypName) &&
	  (++devIdx < task.DeviceTypes.size()));
   if (devIdx >= task.DeviceTypes.size()) // this device type was never loaded
   {
      Post(718,taskName,int2str(Urank),devTypName); // so error.
      return AddressBook::ERR_INVALID_DEVTYPE;
   }
   vector<string> devNames;     // names and addresses will be in packed vectors
   vector<SymAddr_t> devAddrs;
   int numSupers = 1;           // get method expects a *reference* to the count
   // supervisor of this device is in the fourth dynamic field
   SymAddr_t* devSuper = msg->Get<SymAddr_t>(3,numSupers); 
   if (!devSuper)
   {
      // no supervisor found
      Post(718,taskName,int2str(Urank));
      return ERR_INVALID_SUPERVISOR;
   }
   vector<AttrIdx> devAttrs;
   msg->GetX(0, devNames); // grab device names and addresses out of the first
   msg->Get(1, devAddrs);  // and second dynamic fields.
   if (devNames.size() != devAddrs.size()) // mismatching counts is an error
   {
      Post(716,int2str(Urank),uint2str(devNames.size()),uint2str(devAddrs.size()),"addresse");
      return ERR_DEVICE_DATA_MISMATCH;	   
   }
   msg->Get(2, devAttrs); // get the device attributes
   // for which there should be one for each device
   if (devNames.size() != devAttrs.size()) 
   {
      Post(716,int2str(Urank),uint2str(devNames.size()),uint2str(devAttrs.size()),"attribute");
      return ERR_DEVICE_DATA_MISMATCH;	   
   }
   unsigned err = AddressBook::SUCCESS;
   // everything checks out. Build device objects.
   for (unsigned d = 0; d < devNames.size(); d++) 
   {
       Record_t device(devNames[d],
		       devAddrs[d],
		       *devSuper,
		       devIdx,
		       RecordType_t(Device),
		       devAttrs[d]);
       // and add them to the database
       if ((err = AddDevice(taskName, device)) != AddressBook::SUCCESS)
       {
	  // try to clean out the task if a device couldn't be inserted 
	  if (ClearTask(taskName) != AddressBook::SUCCESS)
	  {
	     Post(708, device.Name, taskName, int2str(Urank));
	     return err;
	  }
	  Post(707, device.Name, taskName, int2str(Urank));
	  return err;
       }
   }
   if (TaskState(taskName) < Linked) // adding the devices auto-links the records
   {
      if ((err = TaskState(taskName,Linked)))
      {
	 Post(724,taskName,int2str(Urank));
	 return err;
      }
   }
   return AddressBook::SUCCESS;
}

//==============================================================================
/*
  DataDeviceExternal - Load this task with any devices having External 
  connections. Not implemented for now, awaiting definition of Externals.
*/
//==============================================================================
unsigned SBase::DataDeviceExternal(PMsg_p *msg)
{
   return 0;
}


//==============================================================================
/*
  DataExternal - Load this task with External device info. Not implemented for
  now.
*/
//==============================================================================
unsigned SBase::DataExternal(PMsg_p *msg)
{
   return 0;
}


//==============================================================================
/*
  DataSupervisor - Load this task with Supervisor information. Supervisors have
  both an address and an MPI rank.
*/
//==============================================================================
unsigned SBase::DataSupervisor(PMsg_p *msg)
{
   // Post(715,int2str(Urank),int2str(msg->Src()),msg->Zname(0));
   string taskName = msg->Zname(0); // first static field is the task 
   if (taskName == "")              // abort if there was no task name
   {
      Post(705, int2str(Urank));
      return AddressBook::ERR_INVALID_TASK;
   }
   TaskData_t task;
   if (GetTask(taskName, task))     // also abort if the task wasn't loaded
   {
      Post(708,"records",taskName,int2str(Urank));
      return AddressBook::ERR_TASK_NOT_FOUND;
   }
   string devTypName = msg->Zname(2); // third static field is the Supervisor type
   DTypeIdx devIdx = 0;
   // Supervisors should exist in the database as a 'normal' device type
   while ((task.DeviceTypes[devIdx].Name != devTypName) &&
	  (++devIdx < task.DeviceTypes.size()));
   if (devIdx >= task.DeviceTypes.size()) // nonexistent Supervisor type
   {
      Post(718,taskName,int2str(Urank),devTypName);
      return AddressBook::ERR_INVALID_DEVTYPE;
   }
   vector<string> devNames;        // containers for all the Supervisors in
   vector<SymAddr_t> devAddrs;     // the message
   vector<unsigned long> devRanks;
   vector<AttrIdx> devAttrs;
   msg->GetX(0, devNames);         // first dynamic field is the name
   // msg->Get(1, devData);
   msg->Get(1, devAddrs);          // second is the (POETS) address
   if (devNames.size() != devAddrs.size()) // name-address counts must match
   {
      Post(716,int2str(Urank),uint2str(devNames.size()),uint2str(devAddrs.size()),"address");
      return ERR_DEVICE_DATA_MISMATCH;	   
   }
   msg->Get(2, devAttrs);          // third is the Supervisor's attributes
   if (devNames.size() != devAttrs.size()) // which also has to match counts.
   {
      Post(716,int2str(Urank),uint2str(devNames.size()),uint2str(devAttrs.size()),"attribute");
      return ERR_DEVICE_DATA_MISMATCH;	   
   }
   // and fourth is the MPI rank. FIXME: this only works with a single comm.
   // If supervisors are on multiple comms we will need to have some way of
   // identifying the comm as well.
   msg->Get(3, devRanks);         
   if (devNames.size() != devRanks.size())
   {
      Post(716,int2str(Urank),uint2str(devNames.size()),uint2str(devRanks.size()),"rank");
      return ERR_DEVICE_DATA_MISMATCH;	   
   }
   // if everything was OK, load the supervisors in
   for (unsigned d = 0; d < devNames.size(); d++)
   {
       // by building a record for each one
       Record_t device(devNames[d],
		       devAddrs[d],
		       static_cast<uint64_t>(devRanks[d]),
		       devIdx,
		       RecordType_t(Supervisor),
		       devAttrs[d]);   
       unsigned err = AddressBook::SUCCESS;
       // and adding it to the database
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

//==============================================================================
/*
  DumpAll - Output a comprehensive dump of all tasks loaded to this SBase, 
  including device info.
*/
//==============================================================================
unsigned SBase::DumpAll(PMsg_p *msg, unsigned comm)
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
   vector<string> tasks;
   unsigned err = AddressBook::SUCCESS;
   if ((err = ListTask(tasks)))                 // get all the tasks
   {
     Post(710, "Dump", int2str(Urank));         // error: no tasks were found 
      if (dumpFile != stdout) fclose(dumpFile);
      return AddressBook::ERR_NONFATAL;         // so we don't need to do anything
   }
   WALKVECTOR(string, tasks, task) Dump(dumpFile, *task); // dump it all out
   return err;
}

//==============================================================================
/*
  DumpAll - Output only task summary info to a file.
*/
//==============================================================================
unsigned SBase::DumpSummary(PMsg_p *msg, unsigned comm)
{
   string filename = msg->Zname(1);  // second static string - file to dump to
   FILE* dumpFile;
   if (filename.empty()) dumpFile = stdout;      // no name: dump to console
   else dumpFile = fopen(filename.c_str(), "a");
   Dump(dumpFile);                   // dump without a task emits a summary
   if (dumpFile != stdout) fclose(dumpFile);     
   return AddressBook::SUCCESS;
}

//==============================================================================
/*
  DumpTask - Output complete information about a given task to a file.
*/
//==============================================================================
unsigned SBase::DumpTask(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0); // first static string is the task to dump
   if (taskName.empty())            // no name is an obvious error
   {
      Post(710, "Dump", int2str(Urank));
      return AddressBook::ERR_INVALID_TASK; 
   }
   string filename = msg->Zname(1); // second static string is the file to dump to
   FILE* dumpFile;
   if (filename.empty()) dumpFile = stdout; // if file is empty dump to console
   else dumpFile = fopen(filename.c_str(), "a");
   Dump(dumpFile, taskName);                // only dump info about this task
   if (dumpFile != stdout) fclose(dumpFile);
   return AddressBook::SUCCESS;  
}

//==============================================================================
/*
  QueryAttr - Get all device name-address pairs where the device has the
  attribute specified. Currently, the attribute is asked for by name. The 
  name can be a vector, in which case the query matches devices with any of
  the matched names (OR matching). A bit-field representation of an attribute
  (rather than a string) would also allow AND matching (extension for the 
  future?).

  The Query group of commands to SBase will be an external request for device
  information. We do not usually expect such commands to come from internal
  Orchestrator processes, although it's conceivable a Supervisor device could
  ask for an internal query (which should be fulfilled locally by the SBase 
  of the Mothership to which the supervisor belongs). All Queries will generate
  a reply to the querying process. This reply will either be a device list 
  (of some form) a NOTFOUND response (no devices matched the query criteria) or
  a TASKNOTFOUND response (the task being queried doesn't exist in the database)
*/
//==============================================================================
unsigned SBase::QueryAttr(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0); // first static string is the task to query
   if (taskName.empty())            // no task is an obvious error
   {
      Post(710, "QueryByAttribute", int2str(Urank));
      return AddressBook::ERR_INVALID_TASK; 
   }
   unsigned err = SUCCESS;
   // query asks for attributes by name (should this be by attribute ID for
   // compactness?)
   vector<string> attrs; 
   msg->GetX(2,attrs);              // third dynamic field is the attribute set
   int srcProc = msg->Src();        // need the querying source to send a reply
   msg->comm = Comms[comm];
   msg->Src(msg->Tgt());            // build the reply message
   msg->L(1,Q::RPLY);
   vector<SymAddr_t> devAddrs;
   vector<string> devNames;
   vector<string> devAttrs;
   // now grab all the devices having one or more of the attributes asked for
   WALKVECTOR(string, attrs, a)     
   {
      const RecordVect_t* devRecords;
      if ((err = FindByAttribute(taskName, *a, devRecords)))
      {
	  // filter out nonexistent task/nonexistent device queries. An
	  // ERR_INVALID_MAP indicates the task has a dirty device table, and
	  // should be rebuilt before asking for devices.
	  if (err == ERR_TASK_NOT_FOUND || err == ERR_INVALID_MAP)
	  {
	     // Q::TNF => task not found; Q::NF => no matching records found
             msg->L(3, err == ERR_TASK_NOT_FOUND ? Q::TNF : Q::NF); 
             msg->Send(srcProc); // send not found response
             return SUCCESS;
	  }
	  continue; // no devices with this attribute were found
      }
      devAttrs.push_back(*a); // send the actual attributes matched in the reply
      for (vector<const Record_t*>::const_iterator device = devRecords->begin();
	   device != devRecords->end(); device++)
      {
	  devAddrs.push_back((*device)->Address);
	  devNames.push_back((*device)->Name);
      }
   }	
   if (!devAddrs.size()) msg->L(3, Q::NF); // no attributes matched
   else
   {
      msg->Put<SymAddr_t>(0,&devAddrs); // otherwise reply with the matching 
      msg->PutX(1,&devNames);           // devices' name-address pairs
      msg->PutX(2,&devAttrs);
   }
   msg->Send(srcProc);
   return SUCCESS;
}

//==============================================================================
/*
  QueryDevIAll - Gets all devices for a task. We return every device in a single
  PMsg_p. This could get VERY big, but since there are no certainties the
  receiving device knows how many devices it's expecting, chunking the reply
  into bite-sized messages makes it impossible for the receiver to know when it
  can stop receiving. An alternative would be to insert the device count in the
  message, along with possibly a sequence number, but for a first revision this
  is just adding complication for possibly little benefit. To be tested.
*/
//==============================================================================
unsigned SBase::QueryDevIAll(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0); // as usual, first static field is the task
   if (taskName.empty())            // also as usual, no task is an error.
   {
      Post(710, "QueryAllDevices", int2str(Urank));
      return AddressBook::ERR_INVALID_TASK; 
   }
   unsigned err = SUCCESS;
   int srcProc = msg->Src();        // set up the reply to the querying process
   msg->comm = Comms[comm];
   msg->Src(msg->Tgt());
   msg->L(1,Q::RPLY);
   vector<SymAddr_t> devAddrs;
   vector<string> devNames;
   const vector<Record_t>* devRecords;
   if ((err = GetDevices(taskName, devRecords))) // grab all the devices
   {
      // as long as any exist
      msg->L(3, err == ERR_TASK_NOT_FOUND ? Q::TNF : Q::NF); 
      msg->Send(srcProc);
      return SUCCESS;
   }
   // reply message gives all the device name-address pairs.
   for (vector<Record_t>::const_iterator device = devRecords->begin();
	device != devRecords->end(); device++)
   {
       devAddrs.push_back(device->Address);
       devNames.push_back(device->Name);
   }
   // no devices. Should have been caught above, but just in case...
   if (!devAddrs.size()) msg->L(3, Q::NF); 
   else
   {
      msg->Put<SymAddr_t>(0,&devAddrs); // addresses and names are both
      msg->PutX(1,&devNames);           // packed as vectors.
   }
   msg->Send(srcProc);
   return SUCCESS;
}

//==============================================================================
/*
  QueryDevIByName - This gets device addresses from a device name. We handle
  both the case of a simple query for a device address given a name, and the 
  more complex cases where we want all devices in the same group as a named 
  device, or where we want the supervisor of the named device. The latter will 
  return only the address, in the first dynamic field, the former will return a 
  vector of device address-name pairs, in the first and second dynamic fields. 
*/
//============================================================================== 
unsigned SBase::QueryDevIByName(PMsg_p *msg, unsigned comm)
{
   // check that the key type is valid - if not the query is unrecognised. 
   if (!((msg->L(3) == Q::NGRP) || (msg->L(3) == Q::NM) || (msg->L(3) == Q::NSUP)))
   {
      Post(702,uint2str(msg->Key()),int2str(Urank));
      return ERR_NONFATAL;
   }
   string taskName = msg->Zname(0); // first static field is the task name
   if (taskName.empty())            // which, as usual, must exist.
   {
      Post(710, "QueryDeviceByName", int2str(Urank));
      return AddressBook::ERR_INVALID_TASK; 
   }
   string deviceName = msg->Zname(1); // second static field is the device name 
   if (deviceName.empty())          // which also must exist or it is an error.
   {
      Post(731, int2str(Urank));
      return ERR_INVALID_DEVICE;
   }
   unsigned err = SUCCESS;
   int srcProc = msg->Src();       // query is valid - set up a reply.
   msg->comm = Comms[comm];
   msg->Src(msg->Tgt());
   msg->L(1,Q::RPLY);
   const Record_t* device;
   if ((err = FindDevice(taskName, deviceName, device))) // get the base device
   {
      // if the device or task doesn't exist, we can reply immediately
      msg->L(3, err == ERR_TASK_NOT_FOUND ? Q::TNF : Q::NF);
      msg->Send(srcProc);
      return SUCCESS;
   }
   if (msg->L(3) == Q::NGRP) // asking for all devices in the same group.
   {
      vector<SymAddr_t> devAddrs;
      vector<string> devNames;
      const RecordVect_t* devRecords;
      // same group => devices share a supervisor. Look up by supervisor of
      // named device.
      if ((err = FindBySuper(taskName, device->Supervisor, devRecords)) ||
	  (devRecords->size() == 0))
      {
	 // given that at least the lookup device must belong to its
	 // Supervisor, finding no devices attached to the Supervisor in
	 // question would be a serious error!
	 Post(732,int2str(Urank),uint2str(device->Supervisor),device->Name,
	      err == ERR_TASK_NOT_FOUND ? "task" : "devices");
	 msg->L(3, err == ERR_TASK_NOT_FOUND ? Q::TNF : Q::NF);
         msg->Send(srcProc);
	 return ERR_DEVICE_DATA_MISMATCH;
      }
      // bundle up all the device address-name pairs and send them as the reply.
      for (vector<const Record_t*>::const_iterator deviceIt = devRecords->begin();
	   deviceIt != devRecords->end(); deviceIt++)
      {
          devAddrs.push_back((*deviceIt)->Address);
          devNames.push_back((*deviceIt)->Name);
      }
      msg->Put<SymAddr_t>(0,&devAddrs);
      msg->PutX(1,&devNames);
   }
   else // expecting only to respond with an address - either a supervisor or device
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

//==============================================================================
/*
  QueryDevIByID - Gets device names (and possibly addresses from a device
  address. This mirrors (almost) the functionality of QueryDevIByName. The only
  difference is that if a Supervisor is being queried, only the Supervisor ID 
  is returned (it makes no sense to ask for a Supervisor name, since from the
  application POV, there is only one Supervisor device, so its name is
  automatically known). 
*/
//============================================================================== 
unsigned SBase::QueryDevIByID(PMsg_p *msg, unsigned comm)
{
   // just like the ByName case, validate the query type 
   if (!((msg->L(3) == Q::IGRP) || (msg->L(3) == Q::ID) || (msg->L(3) == Q::ISUP)))
   {
      Post(702,uint2str(msg->Key()),int2str(Urank));
      return ERR_NONFATAL;
   }
   string taskName = msg->Zname(0); // as always, first static field is the task
   if (taskName.empty())            // and an empty task is an error.
   {
      Post(710, "QueryDeviceByID", int2str(Urank));
      return AddressBook::ERR_INVALID_TASK; 
   }
   int devCount = 0;
   // first dynamic field is the device address to query on. This must exist,
   // so if the count of values in the first field is 0, it's an error.
   SymAddr_t* deviceAddr = msg->Get<SymAddr_t>(0,devCount);
   if (!devCount)
   {
      Post(731, int2str(Urank));
      return ERR_INVALID_DEVICE;
   }
   unsigned err = SUCCESS;
   int srcProc = msg->Src();       // query is valid, so set up the reply.
   msg->comm = Comms[comm];
   msg->Src(msg->Tgt());
   msg->L(1,Q::RPLY);
   const Record_t* device;
   // find the device being queried by ID.
   if ((err = FindDevice(taskName, *deviceAddr, device)))
   {
      // no such device, or no such task. 
      msg->L(3, err == ERR_TASK_NOT_FOUND ? Q::TNF : Q::NF);
      msg->Send(srcProc);
      return SUCCESS;
   }
   if (msg->L(3) == Q::IGRP)      // we want all devices in the same group
   {
      vector<SymAddr_t> devAddrs;
      vector<string> devNames;
      const RecordVect_t* devRecords;
      // just like the ByName case, look up by supervisor of the query device
      if ((err = FindBySuper(taskName, device->Supervisor, devRecords)) ||
	  (devRecords->size() == 0))
      {
	 // given that at least the lookup device must belong to its Supervisor,
	 // finding no devices attached to the Supervisor in question would be
	 // a serious error!
	 Post(732,int2str(Urank),uint2str(device->Supervisor),device->Name,
	      err == ERR_TASK_NOT_FOUND ? "task" : "devices");
	 msg->L(3, err == ERR_TASK_NOT_FOUND ? Q::TNF : Q::NF);
         msg->Send(srcProc);
	 return ERR_DEVICE_DATA_MISMATCH;
      }
      // and in the same way, as long as devices are found, return their
      // address-name pairs (which should include the queried device)
      for (vector<const Record_t*>::const_iterator deviceIt = devRecords->begin();
	   deviceIt != devRecords->end(); deviceIt++)
      {
          devAddrs.push_back((*deviceIt)->Address);
          devNames.push_back((*deviceIt)->Name);
      }
      msg->Put<SymAddr_t>(0,&devAddrs);
      msg->PutX(1,&devNames);
   }
   else
   {
      // for a simple device, return the name in the first static field
      msg->Zname(1,device->Name);
      if (msg->L(3) == Q::ISUP) // supervisors return their address
      {
	 SymAddr_t nonConstSuperAddr = device->Supervisor;
	 msg->Put<SymAddr_t>(1,&nonConstSuperAddr);
      }
   }
   msg->Send(srcProc);
   return SUCCESS;
}

//==============================================================================
/*
  QueryDevT - Gets device name-address pairs with a given type. There are 3
  flavours of this query: NM - matches on the type, IN - matches on all devices
  sharing an input with the same message type, OUT - matches on all devices
  sharing an output with the same message type.
*/
//============================================================================== 
unsigned SBase::QueryDevT(PMsg_p *msg, unsigned comm)
{
   // validate the query flavour 
   if (!((msg->L(3) == Q::IN) || (msg->L(3) == Q::NM) || (msg->L(3) == Q::OUT)))
   {
      Post(702,uint2str(msg->Key()),int2str(Urank));
      return ERR_NONFATAL;
   }
   string taskName = msg->Zname(0); // first static field is the task name
   if (taskName.empty())            // no task name is an error as always
   {
      Post(710, "QueryDeviceByName", int2str(Urank));
      return AddressBook::ERR_INVALID_TASK; 
   }
   string lookupType = msg->Zname(2); // third static field is the type name
   if (lookupType.empty())
   {
      // no type name is an error - different error class depending on the
      // query flavour
      if (msg->L(3) == Q::NM)
      {
         Post(706,"find",taskName,int2str(Urank));
         return ERR_INVALID_DEVTYPE;
      }
      else
      {
	 Post(733,int2str(Urank));
	 return ERR_INVALID_MESSAGE_TYPE;
      }
       
   }
   unsigned err = SUCCESS;
   int srcProc = msg->Src();  // query is valid; set up the reply
   msg->comm = Comms[comm];
   msg->Src(msg->Tgt());
   msg->L(1,Q::RPLY);
   vector<SymAddr_t> devAddrs;
   vector<string> devNames;
   const RecordVect_t* devRecords;
   // do the lookup for the 3 different query flavours: whichever is
   // specified, we will return name-address pairs.
   if (msg->L(3) == Q::NM) err = FindByType(taskName,lookupType,devRecords);
   else if (msg->L(3) == Q::IN) err = FindByInMsg(taskName,lookupType,devRecords);
   else err = FindByOuMsg(taskName,lookupType,devRecords);
   if (err)
   {
      // no such devices found or no such task found: immediate reply 
      msg->L(3, err == ERR_TASK_NOT_FOUND ? Q::TNF : Q::NF);
      msg->Send(srcProc);
      return SUCCESS;
   }
   // otherwise pack the reply vectors with addresses and names
   for (vector<const Record_t*>::const_iterator device = devRecords->begin();
	device != devRecords->end(); device++)
   {
       devAddrs.push_back((*device)->Address);
       devNames.push_back((*device)->Name);
   }	
   if (!devAddrs.size()) msg->L(3, Q::NF); // should have been caught above
   else
   {
      msg->Put<SymAddr_t>(0,&devAddrs);
      msg->PutX(1,&devNames);
   }
   msg->Send(srcProc);
   return SUCCESS;
   
}

//==============================================================================
/*
  QueryExtn - Gets all external device name-address pairs. An External is a 
  virtual device not hosted in the POETS system but that can interact with 
  other devices as if it were. Since we currently don't have any support for
  externals in the rest of the system, for the moment this query won't do 
  very much (and it can't be practicably tested), but the machinery looks 
  just like a QueryDevIAll so it can be implemented in principle.
*/
//============================================================================== 
unsigned SBase::QueryExtn(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0); // as usual, first static field is the task
   if (taskName.empty())            // also as usual, no task is an error.
   {
      Post(710, "QueryAllExternals", int2str(Urank));
      return AddressBook::ERR_INVALID_TASK; 
   }
   unsigned err = SUCCESS;
   int srcProc = msg->Src();        // set up the reply to the querying process
   msg->comm = Comms[comm];
   msg->Src(msg->Tgt());
   msg->L(1,Q::RPLY);
   vector<SymAddr_t> devAddrs;
   vector<string> devNames;
   const vector<Record_t>* devRecords;
   if ((err = GetExternals(taskName, devRecords))) // grab all the externals
   {
      // as long as any exist
      msg->L(3, err == ERR_TASK_NOT_FOUND ? Q::TNF : Q::NF);
      msg->Send(srcProc);
      return SUCCESS;
   }
   // reply message gives all the external name-address pairs.
   for (vector<Record_t>::const_iterator device = devRecords->begin();
	device != devRecords->end(); device++)
   {
       devAddrs.push_back(device->Address);
       devNames.push_back(device->Name);
   }
   // no externals. Should have been caught above, but just in case...
   if (!devAddrs.size()) msg->L(3, Q::NF);
   else
   {
      msg->Put<SymAddr_t>(0,&devAddrs); // addresses and names are both
      msg->PutX(1,&devNames);           // packed as vectors.
   }
   msg->Send(srcProc);
   return SUCCESS;
}

//==============================================================================
/*
  QueryList - Ask for a list of tasks on this SBase. Can't fail: either we have
  tasks or not so we just return a list if we have them, a not found message if
  not.
*/
//============================================================================== 
unsigned SBase::QueryList(PMsg_p *msg, unsigned comm)
{
   unsigned err = SUCCESS;
   int srcProc = msg->Src(); // query is automatically valid; set up a reply
   msg->comm = Comms[comm];
   msg->Src(msg->Tgt());
   msg->L(1,Q::RPLY);
   vector<string> taskNames;
   // if the list produces no tasks, return with task not found
   if ((err = ListTask(taskNames)) || (taskNames.size() == 0)) msg->L(3,Q::TNF);
   else msg->PutX(1,&taskNames); // otherwise return the vector of task names
   msg->Send(srcProc);
   return SUCCESS;
}

//==============================================================================
/*
  QuerySupv - Get all supervisors for a task. We return one more list in
  addition to the address and the name: the list of ranks, because the
  requesting process may wish to identify a Supervisor by MPI rank rather than
  POETS address. Otherwise this looks like the other 'all'-type queries.
*/
//============================================================================== 
unsigned SBase::QuerySupv(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0); // as usual, first static field is the task
   if (taskName.empty())            // also as usual, no task is an error.
   {
      Post(710, "QueryAllSupervisors", int2str(Urank));
      return AddressBook::ERR_INVALID_TASK; 
   }
   unsigned err = SUCCESS;
   int srcProc = msg->Src();        // set up the reply to the querying process
   msg->comm = Comms[comm];
   msg->Src(msg->Tgt());
   msg->L(1,Q::RPLY);
   vector<SymAddr_t> devAddrs;
   vector<string> devNames;
   vector<unsigned long> devRanks; 
   const vector<Record_t>* devRecords;
   if ((err = GetSupervisors(taskName, devRecords))) // grab all the supervisors
   {
      // as long as any exist
      msg->L(3, err == ERR_TASK_NOT_FOUND ? Q::TNF : Q::NF);
      msg->Send(srcProc);
      return SUCCESS;
   }
      // reply message gives name-address-rank triplets for each Supervisor
   for (vector<Record_t>::const_iterator device = devRecords->begin();
	device != devRecords->end(); device++)
   {
       devAddrs.push_back(device->Address);
       devNames.push_back(device->Name);
       devRanks.push_back(device->Rank);
   }
   // no supervisors. Should have been caught above, but just in case...
   if (!devAddrs.size()) msg->L(3, Q::NF);
   else
   {
      msg->Put<SymAddr_t>(0,&devAddrs);     // addresses,
      msg->PutX(1,&devNames);               // names, and
      msg->Put<unsigned long>(2,&devRanks); // ranks are all packed as vectors
   }
   msg->Send(srcProc);
   return SUCCESS; 
}

//==============================================================================
/*
  QueryTask - A task query might end up being some set of requests with further
  internal details in future, but for the moment this simply returns summary
  task info, like a dump, but to a remote querying process.
*/
//============================================================================== 
unsigned SBase::QueryTask(PMsg_p *msg, unsigned comm)
{
   string taskName = msg->Zname(0); // first static field is the task, as always
   if (taskName.empty())            // and no task name is an error.
   {
      Post(710, "QueryTask", int2str(Urank));
      return AddressBook::ERR_INVALID_TASK; 
   }
   unsigned err = SUCCESS;
   int srcProc = msg->Src();            // set up the reply
   msg->comm = Comms[comm];
   msg->Src(msg->Tgt());
   msg->L(1,Q::RPLY);
   TaskData_t task;
   if ((err = GetTask(taskName, task))) // get the task info
   {
      msg->L(3,Q::TNF);                 // there is no such task
      msg->Send(srcProc);
      return SUCCESS;
   }
   // turn the state into a string for easier reading at the querying process
   msg->Zname(1,State2Str(task.State));
   if (msg->Zname(1) == "Unknown")
      Post(734,taskName,int2str(Urank)); // some strange unknown task state
   /*
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
   */
   msg->Zname(2,task.Path);             // dump all other available info 
   msg->Zname(3,task.XML);              // into the reply message
   msg->PutX(0,&task.MessageTypes);
   msg->PutX(1,&task.AttributeTypes);
   vector<unsigned long> counts;        // put device counts in a vector
   counts.push_back(task.DeviceCount);  // (just so we have fewer fields)
   counts.push_back(task.ExternalCount);
   counts.push_back(task.SupervisorCount);
   msg->Put<unsigned long>(2,&counts);
   msg->Send(srcProc);
   return SUCCESS;
}

//==============================================================================
/*
  The Send group of commands. In general, these forward on pre-packed messages
  containing POETS packets to some subset of devices in a POETS system. These
  are all virtual in SBase, because what a given process actually does with a
  Send depends on what type of process it is. A Nameserver, for example, needs
  to detect which Supervisors to talk to and relay the message on to them. A
  Supervisor needs either to unpack the message and send it over HostLink, or
  identify where to forward the message, if it doesn't have the devices being
  targetted. The functions here are not pure-virtual, though, because one can
  conceive of SBase processes which simply don't implement Send functionality.
  So we provide the not-implemented stubs here.
*/
//============================================================================== 
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

//==============================================================================
/*
  State2Str - a static method to transform a state enum into a readable string
*/
//============================================================================== 
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

//==============================================================================
/*
  Str2State - a static method to transform a state string into a state enum
*/
//============================================================================== 
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

//------------------------------------------------------------------------------

#include "NameServer.h"
#include "PMsg_p.hpp"
#include "mpi.h"
#include "Pglobals.h"
// #include "Ns_el.h"
// #include "jnj.h"
#include <stdio.h>

//==============================================================================

NameServer::NameServer(int argc,char * argv[],string d):
  SBase(argc,argv,d,string(__FILE__))
{
  FnMapx.push_back(new FnMap_t);       // create a default function table
                                       // Load the message map
  (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::CFG,Q::DIST        )] = &NameServer::OnCfg;
  (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::CFG,Q::TDIR        )] = &NameServer::OnCfg;
  (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::CFG,Q::BLD         )] = &NameServer::OnCfg;
  (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::CFG,Q::RECL        )] = &NameServer::OnCfg;
  (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::CFG,Q::DEL         )] = &NameServer::OnCfg;
  (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::CFG,Q::STATE       )] = &NameServer::OnCfg;
  (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::ALL)] = &NameServer::OnDump;
  (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::NM )] = &NameServer::OnDump;
  (*FnMapx[0])[Msg_p::KEY(Q::NAME,Q::DUMP,Q::LIST       )] = &NameServer::OnDump;

  MPISpinner();                        // Spin on MPI messages; exit only on DIE

//printf("********* NameServer rank %d on the way out\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

NameServer::~NameServer()
{
//printf("********* NameServer rank %d destructor\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

void NameServer::Dump(FILE * fp, string task)
{
if (task.empty())
{
   fprintf(fp,"NameServer summary dump+++++++++++++++++++++++++++\n");
   SBase::Dump(fp);
   fprintf(fp,"NameServer summary dump---------------------------\n");
}
else
{
   fprintf(fp,"NameServer dump+++++++++++++++++++++++++++++++++++\n");
   SBase::Dump(fp,task);
   fprintf(fp,"NameServer dump-----------------------------------\n");
}
CommonBase::Dump(fp);
}

//------------------------------------------------------------------------------

unsigned NameServer::OnCfg(PMsg_p * msg, unsigned comm)
{
   switch(msg->L(2))
   {
   case Q::DIST:
   return ConfigDistribute(msg, comm);
   case Q::TDIR:
   return ConfigDir(msg, comm);
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

//------------------------------------------------------------------------------

unsigned NameServer::OnDump(PMsg_p *msg, unsigned comm)
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

//------------------------------------------------------------------------------

unsigned NameServer::ConfigDir(PMsg_p * msg, unsigned comm)
{
   return 0;
}

unsigned NameServer::ConfigDistribute(PMsg_p * msg, unsigned comm)
{
   return 0;
}

unsigned NameServer::ConfigRecall(PMsg_p * msg, unsigned comm)
{
   return 0;
}

unsigned NameServer::ConfigState(PMsg_p * msg, unsigned comm)
{
   return 0;
}

//------------------------------------------------------------------------------

unsigned NameServer::DumpAll(PMsg_p *msg)
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

//------------------------------------------------------------------------------

unsigned NameServer::DumpSummary(PMsg_p *msg)
{
   string filename = msg->Zname(1);
   FILE* dumpFile;
   if (filename.empty()) dumpFile = stdout;
   else dumpFile = fopen(filename.c_str(), "a");
   Dump(dumpFile);
   if (dumpFile != stdout) fclose(dumpFile);
   return AddressBook::SUCCESS;
}

//------------------------------------------------------------------------------

unsigned NameServer::DumpTask(PMsg_p *msg)
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


//==============================================================================


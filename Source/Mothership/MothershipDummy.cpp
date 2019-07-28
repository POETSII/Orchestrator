#include "MothershipDummy.h"
#include "CMsg_p.h"
#include "Pglobals.h"
#include <stdio.h>


MothershipDummy::MothershipDummy(int argc,char * argv[], string d) :
  SBase(argc,argv,d,string(__FILE__))
{
  FnMapx.push_back(new FnMap_t);       // create a default function table
                                       // Load the message map
  
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DIST        )] = &MothershipDummy::OnCfg;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::TDIR        )] = &MothershipDummy::OnCfg;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::BLD         )] = &MothershipDummy::OnCfg;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::RECL        )] = &MothershipDummy::OnCfg;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DEL         )] = &MothershipDummy::OnCfg;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::STATE       )] = &MothershipDummy::OnCfg;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::ALL)] = &MothershipDummy::OnDump;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::NM )] = &MothershipDummy::OnDump;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::LIST       )] = &MothershipDummy::OnDump;
  (*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::LOAD               )] = &MothershipDummy::OnCmnd;
  (*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::RUN                )] = &MothershipDummy::OnCmnd;
  (*FnMapx[0])[PMsg_p::KEY(Q::CMND,Q::STOP               )] = &MothershipDummy::OnCmnd;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DIST               )] = &MothershipDummy::OnName;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::RECL               )] = &MothershipDummy::OnName;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::TDIR               )] = &MothershipDummy::OnName;
  (*FnMapx[0])[PMsg_p::KEY(Q::SUPR                       )] = &MothershipDummy::OnSuper;
  (*FnMapx[0])[PMsg_p::KEY(Q::SYST,Q::HARD               )] = &MothershipDummy::OnSyst;
  (*FnMapx[0])[PMsg_p::KEY(Q::SYST,Q::KILL               )] = &MothershipDummy::OnSyst;
  (*FnMapx[0])[PMsg_p::KEY(Q::SYST,Q::SHOW               )] = &MothershipDummy::OnSyst;
  (*FnMapx[0])[PMsg_p::KEY(Q::SYST,Q::TOPO               )] = &MothershipDummy::OnSyst;

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
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DIST        )] = &MothershipDummy::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::TDIR        )] = &MothershipDummy::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::BLD         )] = &MothershipDummy::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::RECL        )] = &MothershipDummy::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DEL         )] = &MothershipDummy::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::STATE       )] = &MothershipDummy::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::ALL)] = &MothershipDummy::OnDump;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::NM )] = &MothershipDummy::OnDump;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::LIST       )] = &MothershipDummy::OnDump;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::CMND,Q::LOAD               )] = &MothershipDummy::OnCmnd;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::CMND,Q::RUN                )] = &MothershipDummy::OnCmnd;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::CMND,Q::STOP               )] = &MothershipDummy::OnCmnd;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DIST               )] = &MothershipDummy::OnName;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::RECL               )] = &MothershipDummy::OnName;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::TDIR               )] = &MothershipDummy::OnName;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::SUPR                       )] = &MothershipDummy::OnSuper;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::SYST,Q::HARD               )] = &MothershipDummy::OnSyst;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::SYST,Q::KILL               )] = &MothershipDummy::OnSyst;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::SYST,Q::SHOW               )] = &MothershipDummy::OnSyst;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::SYST,Q::TOPO               )] = &MothershipDummy::OnSyst;
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
   if (err = SBase::ConfigDir(msg,comm)) return err;
   if (err = SBase::ConfigDistribute(msg,comm)) return err;
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
   return err;
}

unsigned MothershipDummy::ConfigRecall(PMsg_p *msg, unsigned comm)
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
	  err = ERR_NONFATAL;
       }
   }
   return err;
}

unsigned MothershipDummy::ConfigState(PMsg_p *msg, unsigned comm)
{
   return 0;
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

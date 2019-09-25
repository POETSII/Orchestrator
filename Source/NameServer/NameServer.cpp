//------------------------------------------------------------------------------

#include "NameServer.h"
#include "PMsg_p.hpp"
#include "mpi.h"
#include "Pglobals.h"
// #include "Ns_el.h"
// #include "jnj.h"
#include "CMsg_p.h"
#include <stdio.h>

//==============================================================================

NameServer::NameServer(int argc,char * argv[],string d):
  SBase(argc,argv,d,string(__FILE__))
{
  FnMapx.push_back(new FnMap_t);       // create a default function table
                                       // Load the message map
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DIST        )] = &NameServer::OnCfg;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::TDIR        )] = &NameServer::OnCfg;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::BLD         )] = &NameServer::OnCfg;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::RECL        )] = &NameServer::OnCfg;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DEL         )] = &NameServer::OnCfg;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::STATE       )] = &NameServer::OnCfg;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::ALL)] = &NameServer::OnDump;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::NM )] = &NameServer::OnDump;
  (*FnMapx[0])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::LIST       )] = &NameServer::OnDump;

  MPISpinner();                        // Spin on MPI messages; exit only on DIE

  printf("********* NameServer rank %d on the way out\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

NameServer::~NameServer()
{
   WALKVECTOR(FnMap_t*,FnMapx,F)          // WALKVECTOR and WALKMAP are in macros.h (long include chain)
     delete *F;                           // get rid of derived class function tables
   printf("********* NameServer rank %d destructor\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

unsigned NameServer::Connect(string svc)
{
   unsigned connErr = MPI_SUCCESS;
   // set up the connection in the base class
   if ((connErr = SBase::Connect(svc)) != MPI_SUCCESS) return connErr; 
   FnMapx.push_back(new FnMap_t); // add another function table in the derived class
   int fIdx=FnMapx.size()-1;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DIST        )] = &NameServer::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::TDIR        )] = &NameServer::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::BLD         )] = &NameServer::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::RECL        )] = &NameServer::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::DEL         )] = &NameServer::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::CFG,Q::STATE       )] = &NameServer::OnCfg;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::ALL)] = &NameServer::OnDump;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::TASK,Q::NM )] = &NameServer::OnDump;
   (*FnMapx[fIdx])[PMsg_p::KEY(Q::NAME,Q::DUMP,Q::LIST       )] = &NameServer::OnDump;
   return MPI_SUCCESS;
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

unsigned NameServer::ConfigDir(PMsg_p * msg, unsigned comm)
{
   string taskName = msg->Zname(0);
   const vector<Record_t>* supervisors;
   unsigned err = GetSupervisors(taskName, supervisors);
   if (err == AddressBook::ERR_TASK_NOT_FOUND)
   {
      Post(724,taskName,int2str(Urank));
      return err;
   }
   if (err == AddressBook::ERR_INVALID_MAP)
   {
      Post(727,int2str(Urank));
      return ERR_NONFATAL;
   }
   // finding supervisors naively by rank starting with comm 0 will work for
   // the moment but this should be fixed to permit arbitrary lookup by comm.
   for (vector<Record_t>::const_iterator s = supervisors->begin(); s != supervisors->end(); s++)
   {
      for (unsigned sComm = 0; sComm < pPmap.size(); sComm++)
      {
	  WALKVECTOR(int,pPmap[sComm]->U.Mothership,m)
	  {
	     if (*m == static_cast<int>(s->Rank))
	     {
	        msg->comm = Comms[sComm]; // relay the message to all supervisors
		msg->Send(*m);           // associated with this task.
		break;
	     }
	  }
	  if (msg->Tgt() == s->Rank) break; // supervisor found
      }
      if (msg->Tgt() != s->Rank)            // supervisor not found
      {
	 Post(728,int2str(Urank),uint2str(s->Rank));
	 return ERR_INVALID_SUPERVISOR;
      }
   }
   return SBase::ConfigDir(msg,comm);
}

unsigned NameServer::ConfigDistribute(PMsg_p * msg, unsigned comm)
{
   string taskName = msg->Zname(0);
   int rcount = 0;
   // SymAddr_t superAddr = msg->Get<SymAddr_t>(0,rcount); // use once symbolic addresses are implemented
   int* superAddr = msg->Get<int>(0,rcount);
   if (!superAddr)
   {
      Post(730, "NAME::CFG::DIST",int2str(Urank),"Mothership Address");
      return ERR_NONFATAL;
   }
   const vector<Record_t>* supervisors;
   unsigned err = GetSupervisors(taskName, supervisors);
   if (err == AddressBook::ERR_TASK_NOT_FOUND)
   {
      Post(724,taskName,int2str(Urank));
      return err;
   }
   if (err == AddressBook::ERR_INVALID_MAP)
   {
      Post(727,int2str(Urank));
      return ERR_NONFATAL;
   }
   // finding supervisors naively by rank starting with comm 0 will work for
   // the moment but this should be fixed to permit arbitrary lookup by comm.
   for (vector<Record_t>::const_iterator s = supervisors->begin(); s != supervisors->end(); s++)
   {
      for (unsigned sComm = 0; sComm < pPmap.size(); sComm++)
      {
	  WALKVECTOR(int,pPmap[sComm]->U.Mothership,m)
	  {
	     // if ((s->Address == *superAddr) && (*m == static_cast<int>(s->Rank))) // SymAddr_t form
	     if ((static_cast<int>(s->Rank) == *superAddr) && (*m == static_cast<int>(s->Rank)))
	     {
	        DebugPrint("Deploying name data to Mothership at rank %d\n",*m); 
	        // send over the task NameServer data 
	        PMsg_p taskInfo(Comms[sComm]);
		taskInfo.Src(Urank);
		taskInfo.Key(Q::NAME,Q::DATA,Q::TASK);
		TaskData_t taskData;
		if (err = GetTask(taskName,taskData))
		{
                   Post(724,taskName,int2str(Urank));
                   return err;   
		}
                vector<unsigned long> devCounts;
		devCounts.push_back(taskData.DeviceCount);
		devCounts.push_back(taskData.ExternalCount);
		taskInfo.Zname(0,taskData.Name);
		taskInfo.Zname(1,taskData.Path);
		taskInfo.Zname(2,taskData.XML);
		taskInfo.PutX(0,&taskData.MessageTypes);
		taskInfo.PutX(1,&taskData.AttributeTypes);
		taskInfo.Put(2,&devCounts);
		taskInfo.Send(*m);
		taskInfo.Key(Q::NAME,Q::DATA,Q::DEVT);
                WALKVECTOR(DevTypeRecord_t,taskData.DeviceTypes,devType)
		{
		   taskInfo.Zname(2,devType->Name);
		   taskInfo.Put(0,&devType->InMsgs);
		   taskInfo.Put(1,&devType->OuMsgs);
		   taskInfo.Send(*m);
		}
		vector<string>devNames;
		vector<SymAddr_t>devAddrs;
		vector<AttrIdx>devAttrs;
		vector<unsigned long> superRanks;
		// send over the Supervisor device info
		taskInfo.Key(Q::NAME,Q::DATA,Q::SUPV);
		devNames.push_back(s->Name);
		devAddrs.push_back(s->Address);
		devAttrs.push_back(s->Attribute);
		superRanks.push_back(s->Rank);
		taskInfo.Zname(2,taskData.DeviceTypes[s->DeviceType].Name);
		taskInfo.PutX(0,&devNames);
		taskInfo.Put(1,&devAddrs);
		taskInfo.Put(2,&devAttrs);
		taskInfo.Put(3,&superRanks);
		taskInfo.Send(*m);
		devNames.clear();
		devAddrs.clear();
		devAttrs.clear();
		superRanks.clear();
		const RecordVect_t* devicesThisSuper;
		if (err = FindBySuper(taskName,s->Address,devicesThisSuper))
		{
		   switch (err)
		   {
		   case ERR_TASK_NOT_FOUND:
		   Post(724,taskName,int2str(Urank));
		   return err;
		   case ERR_INVALID_MAP:
		   Post(727,int2str(Urank));
		   return ERR_NONFATAL;
		   case ERR_DEVICE_NOT_FOUND:
		   Post(718,taskName,int2str(Urank));
		   return err;
		   default:
		   Post(136);
		   return err;
		   }
		}
		if (devicesThisSuper->size())
		{
		   RecordType_t devRType((*devicesThisSuper->begin())->RecordType);
		   DTypeIdx devTypeIndex = (*devicesThisSuper->begin())->DeviceType;
		   taskInfo.Zname(2,taskData.DeviceTypes[devTypeIndex].Name); 
		   switch(devRType)
		   {
		   case Device:
		   taskInfo.Key(Q::NAME,Q::DATA,Q::DEVI);
		   break;
		   case DeviceExt:
		   taskInfo.Key(Q::NAME,Q::DATA,Q::DEVE);
		   break;
		   case External:
		   taskInfo.Key(Q::NAME,Q::DATA,Q::EXTN);
		   break;
		   case Supervisor:
                   taskInfo.Key(Q::NAME,Q::DATA,Q::SUPV);
		   }
		   for (RecordVect_t::const_iterator dR = devicesThisSuper->begin(); dR != devicesThisSuper->end(); dR++)
		   {
		      if (((*dR)->DeviceType != devTypeIndex) || ((*dR)->RecordType != devRType))
		      {
			 if ((devRType == Device) || (devRType == DeviceExt))
			 {
			    /* awkwardly, because GetSupervisors returns a const vector<Record_t>*,
			       internal members are const-qualified. But the Put functionality expects
			       non-const objects when creating the data stream (because the stream itself
			       is non-const and it uses an initialiser form to stream the data in). In
			       turn this means we have to create a temporary copy of the Supervisor
			       address for no other reason than to be able to heave it into another
			       buffer.
			    */
			    SymAddr_t supAddr = s->Address;
			    taskInfo.Put<SymAddr_t>(3,&supAddr);
			 }
			 taskInfo.PutX(0,&devNames);
			 taskInfo.Put(1,&devAddrs);
			 taskInfo.Put(2,&devAttrs);
			 taskInfo.Send(*m);
			 devTypeIndex = (*dR)->DeviceType;
			 taskInfo.Zname(2,taskData.DeviceTypes[devTypeIndex].Name);
			 devRType = (*dR)->RecordType;
			 switch(devRType)
		         {
                         case Device:
		         taskInfo.Key(Q::NAME,Q::DATA,Q::DEVI);
		         break;
		         case DeviceExt:
		         taskInfo.Key(Q::NAME,Q::DATA,Q::DEVE);
		         break;
		         case External:
		         taskInfo.Key(Q::NAME,Q::DATA,Q::EXTN);
		         break;
		         case Supervisor:
                         taskInfo.Key(Q::NAME,Q::DATA,Q::SUPV);
		         }
			 devNames.clear();
			 devAddrs.clear();
			 devAttrs.clear();
			 superRanks.clear();
		      }
		      if ((*dR)->RecordType == Supervisor)
		      {
			 if ((*dR)->Address != s->Address)
		         {
			    Post(729,(*dR)->Name,s->Name);
		            return ERR_INVALID_SUPERVISOR;
		         }
		         superRanks.push_back((*dR)->Rank);
	                 taskInfo.Put(3,&superRanks);
		      }
                      devNames.push_back((*dR)->Name);
		      devAddrs.push_back((*dR)->Address);
		      devAttrs.push_back((*dR)->Attribute);
		   }
		   if ((devRType == Device) || (devRType == DeviceExt))
		   {
                      SymAddr_t supAddr = s->Address;
                      taskInfo.Put<SymAddr_t>(3,&supAddr);
		   }
		   taskInfo.PutX(0,&devNames);
		   taskInfo.Put(1,&devAddrs);
		   taskInfo.Put(2,&devAttrs);
		   taskInfo.Send(*m);
		}   
	        msg->comm = Comms[sComm]; // relay the message to the supervisor
		msg->Send(*m);            // being deployed to in this message
		break;
	     }
	  }
	  if (msg->Tgt() == s->Rank) break; // supervisor found
      }
      if (msg->Tgt() != s->Rank)            // supervisor not found
      {
	 Post(728,int2str(Urank),uint2str(s->Rank));
	 return ERR_INVALID_SUPERVISOR;
      }
      else break; // distribute is one supervisor at a time
   }
   // debug output -----------------------------------
   FILE* nsOutput = fopen("DistributionDump.txt","w");
   CMsg_p distMsg(*msg);
   distMsg.Dump(nsOutput);
   fclose(nsOutput);
   //-------------------------------------------------
   if (err = SBase::ConfigDir(msg,comm)) // distribution messages contain the binary directory
   {
      Post(724,taskName,int2str(Urank)); // no task of this name. 
      return err;                        // this should have been caught earlier.
   }
   return SBase::ConfigDistribute(msg, comm);
}

unsigned NameServer::ConfigRecall(PMsg_p * msg, unsigned comm)
{
   return 0;
}

/* NameServer only needs set up its own state. Motherships don't
   need a state until deployed, because it's only at this point that
   name services data is uploaded to them, and after they do have it,
   further state changes involve direct explicit commands to the Motherships
   anyway.
*/
unsigned NameServer::ConfigState(PMsg_p * msg, unsigned comm)
{
   return SBase::ConfigState(msg, comm);
}

//==============================================================================


#include "TaskInfo.h"

const unsigned char TaskInfo_t::TASK_IDLE;
const unsigned char TaskInfo_t::TASK_BOOT;
const unsigned char TaskInfo_t::TASK_RDY;
const unsigned char TaskInfo_t::TASK_BARR;
const unsigned char TaskInfo_t::TASK_RUN;
const unsigned char TaskInfo_t::TASK_STOP;
const unsigned char TaskInfo_t::TASK_END;
const unsigned char TaskInfo_t::TASK_ERR;
const map<unsigned char,string> TaskInfo_t::Task_Status({{TaskInfo_t::TASK_IDLE,"TASK_IDLE"},{TaskInfo_t::TASK_BOOT,"TASK_BOOT"},{TaskInfo_t::TASK_RDY,"TASK_RDY"},{TaskInfo_t::TASK_BARR,"TASK_BARR"},{TaskInfo_t::TASK_RUN,"TASK_RUN"},{TaskInfo_t::TASK_STOP,"TASK_STOP"},{TaskInfo_t::TASK_END,"TASK_END"},{TaskInfo_t::TASK_ERR,"TASK_ERR"}});

TaskInfo_t::TaskInfo_t()
{
    status = TASK_IDLE;
}

TaskInfo_t::~TaskInfo_t()
{
   WALKMAP(P_board*,vector<BinPair_t>*,binaries,bBins)
     delete bBins->second;
}

vector<P_core*>& TaskInfo_t::CoresForTask()
{
   if (!cores.size())
   {  
      WALKVECTOR(P_board*,VirtualBox->P_boardv,board)
        cores.insert(cores.end(),(*board)->P_corev.begin(),(*board)->P_corev.end());
   }
   return cores;
}
vector<P_thread*>& TaskInfo_t::ThreadsForTask()
{
   if (!threads.size())
   {
      WALKVECTOR(P_board*,VirtualBox->P_boardv,board)
      {
	WALKVECTOR(P_core*,(*board)->P_corev,core)
	  threads.insert(threads.end(),(*core)->P_threadv.begin(),(*core)->P_threadv.end());
      }
      return threads;
   }
}
vector<P_device*>& TaskInfo_t::DevicesForTask()
{
   if (!devices.size())
   {
      WALKVECTOR(P_board*,VirtualBox->P_boardv,board)
      {
	WALKVECTOR(P_core*,(*board)->P_corev,core)
	{
	  WALKVECTOR(P_thread*,(*core)->P_threadv,thread)
	    devices.insert(devices.end(),(*thread)->P_devicel.begin(),(*thread)->P_devicel.end());
	}
      }
      return devices;
   }  
}

vector<BinPair_t>& TaskInfo_t::BinariesForBoard(P_board* board)
{
   if (binaries.find(board) == binaries.end())
   {
      BinPair_t core_bins;
      binaries[board] = new vector<BinPair_t>;
      WALKVECTOR(P_core*,board->P_corev,core)
      {
	core_bins.instr = (*core)->pCoreBin;
	core_bins.data = (*core)->pDataBin;
	binaries[board]->push_back(core_bins);
      }
   }
   return *binaries[board];
}

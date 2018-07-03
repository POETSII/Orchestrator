#ifndef __TASKINFO_H__
#define __TASKINFO_H__

#include "P_task.h"
#include "P_box.h"
#include "P_board.h"
#include "P_core.h"
#include "P_thread.h"
#include "P_device.h"

//typedef map<unsigned, unsigned> hardMap_t;    // physical->virtual mapping tables
typedef map<P_core*,unsigned> hardMap_t;        // physical->virtual mapping tables
typedef struct BinPair
{
        Bin* instr;
        Bin* data;
} BinPair_t;

class TaskInfo_t 
{
public:

  TaskInfo_t();
  virtual ~TaskInfo_t();
  
  unsigned char status;       // task runtime state
  P_box* VirtualBox;          // which boards/cores does this task use?
  hardMap_t* CoreMap;         // which threads are on which board/core
  string BinPath;             // which directory has which task's binaries

  inline vector<P_board*>& BoardsForTask() {return VirtualBox->P_boardv;};
  vector<P_core*>& CoresForTask();
  vector<P_thread*>& ThreadsForTask();
  vector<P_device*>& DevicesForTask();
  vector<BinPair_t>& BinariesForBoard(P_board*);

  static const unsigned char TASK_IDLE = 0x0;
  static const unsigned char TASK_BOOT = 0x1;
  static const unsigned char TASK_RDY  = 0x2; 
  static const unsigned char TASK_BARR = 0x4;
  static const unsigned char TASK_RUN  = 0x8;
  static const unsigned char TASK_STOP = 0x10;
  static const unsigned char TASK_END  = 0x40;
  static const unsigned char TASK_ERR  = 0x80;
  static const map<unsigned char,string> Task_Status;

private:
  
  /* flattened object lists. It seems inefficient to store these again, when
     they're contained in the object hierarchy of VirtualBox, but alternatives
     are complex enough (involving template classes with user-defined iterators
     and function objects) that they are not being attempted for this version
     of the tools.
  */
  vector<P_core*> cores;
  vector<P_thread*> threads;
  vector<P_device*> devices;
  
  map<P_board*,vector<BinPair_t>*> binaries;
  
};

#endif

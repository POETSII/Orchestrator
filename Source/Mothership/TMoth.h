#ifndef __TMothH__H
#define __TMothH__H

#include "CommonBase.h"
#include "PMsg_p.hpp"
#include "Cli.h"
#include "HostLink.h"
#include "pthread.h"

//==============================================================================

class TMoth : public CommonBase, private HostLink
{

public:
                    TMoth(int,char **,string);
virtual ~           TMoth();

private:
void*               Accept(void*);
unsigned            Boot(string);
unsigned            CmLoad(string);
unsigned            CmRun(string);
unsigned            CmStop(string);
#include            "Decode.cpp"
virtual string      Dname(){ return typeid(*this).name(); }
void                Dump(FILE * = stdout);
void*               LoadBoard(void*);
unsigned            MPISpinLoop();
unsigned            NameDist();
unsigned            OnCmnd(PMsg_p *,unsigned);
unsigned            OnExit(PMsg_p *,unsigned);
unsigned            OnName(PMsg_p *,unsigned);
unsigned            OnSuper(PMsg_p *,unsigned);
unsigned            OnSyst(PMsg_p *,unsigned);
unsigned            OnTinsel(void *,unsigned);
unsigned            ProcCmnd(Cli *);
unsigned            SystHW(const vector<string>&);
unsigned            SystKill(unsigned);
unsigned            SystShow();
unsigned            SystTopo();

unsigned            PAddress; // address of this mothership in POETS-space  
char                MPIPort[MPI_MAX_PORT_NAME];
static const char*  MPISvc = "POETS_Box_Master";
static const int    MPISrv = 1; // Mothercores are servers
 
public:
typedef map<unsigned, unsigned> coreMap_t;    // which physical cores are which virtual cores
typedef map<unsigned, coreMap_t*> boardMap_t; // which board has which cores 
typedef unsigned    (TMoth::*pMeth)(PMsg_p *);
typedef map<unsigned,pMeth> FnMap_t;
typedef struct TaskInfo
{
  unsigned char status; // which tasks are in what what state
  boardMap_t* boards;   // which tasks use which boards/cores
} TaskInfo_t;
 
map<string, TaskInfo_t*> TaskMap;                // which tasks are mapped to the machine
map<unsigned, unsigned> ThreadMap;               // which threads are on which core
map<unsigned, coreMap_t*> BoardMap;              // which board has which cores 
map<unsigned, pthread_t*> BootMap;               // which booter is starting which board
map<string, string> BinPath;                     // which directory has which task's binaries
vector<FnMap_t*>    FnMapx;
char                Port[MPI_MAX_PORT_NAME]; // MPI port for this mothership


static const int           NumBoards = TinselMeshXLen*TinselMeshYLen;
static const unsigned char TASK_IDLE = 0x0;
static const unsigned char TASK_BOOT = 0x1;
static const unsigned char TASK_RDY  = 0x2;
static const unsigned char TASK_BARR = 0x4;
static const unsigned char TASK_RUN  = 0x8;
static const unsigned char TASK_STOP = 0x10;
static const unsigned char TASK_END  = 0x40;
static const unsigned char TASK_ERR  = 0x80;
 
};

//==============================================================================

#endif





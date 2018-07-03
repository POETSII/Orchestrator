#ifndef __TMothH__H
#define __TMothH__H

#include <deque>
#include "CommonBase.h"
#include "PMsg_p.hpp"
#include "Cli.h"
#include "HostLink.h"
#include "pthread.h"
#include "TaskInfo.h"
#include "poets_msg.h"
#include "P_addr.h"

//==============================================================================

class TMoth : public CommonBase, public HostLink
{

public:
                      TMoth(int,char **,string);
virtual ~             TMoth();
static void*          Accept(void*);
// somewhat bodgey function to return the hardware address from the composite; needed because of
// the actual hardware mappings; this is the Orchestrator-side equivalent of toAddr.
static inline unsigned GetHWAddr(P_addr& VAddr) {return (VAddr.A_box << P_BOX_OS) | (VAddr.A_board << P_BOARD_OS) | (VAddr.A_core << P_CORE_OS) | (VAddr.A_thread << P_THREAD_OS);};
static void*          LoadBoard(void*);
static void*          Twig(void*);

private:
unsigned              Boot(string);
unsigned              CmLoad(string);
unsigned              CmRun(string);
unsigned              CmStop(string);
#include              "Decode.cpp"
inline virtual string Dname(){ return typeid(*this).name(); }
void                  Dump(FILE * = stdout);
unsigned              NameDist();
unsigned              NameTdir(const string&, const string&);
unsigned              OnCmnd(PMsg_p *,unsigned);
unsigned              OnExit(PMsg_p *,unsigned);
void                  OnIdle();
unsigned              OnName(PMsg_p *,unsigned);
unsigned              OnSuper(PMsg_p *,unsigned);
unsigned              OnSyst(PMsg_p *,unsigned);
unsigned              OnTinsel(PMsg_p*, unsigned);
unsigned              OnTinselOut(P_Sup_Msg_t *);
int                   SupervisorCall(PMsg_p*, PMsg_p*); // entry point for the Supervisor
unsigned              SystHW(const vector<string>&);
unsigned              SystKill();
unsigned              SystShow();
unsigned              SystTopo();

public:
unsigned            PAddress; // address of this mothership in POETS-space  
char                MPIPort[MPI_MAX_PORT_NAME];
bool                AcceptConns;
bool                ForwardMsgs;
static const char*  MPISvc;
static const int    MPISrv = 1; // Mothercores are servers
 
typedef unsigned (TMoth::*pMeth)(PMsg_p *,unsigned);
typedef map<unsigned,pMeth> FnMap_t;
typedef map<uint16_t,char*> PinBuf_t; // type to hold buffers for messages received from devices

map<uint32_t,deque<P_Msg_t>*> TwigExtMap;     // dynamic queues for messages bound for external devices
map<uint32_t,PinBuf_t*> TwigMap;              // dynamic buffer state for each device from which the Mothership is receiving
map<string, TaskInfo_t*> TaskMap;             // which tasks are mapped to the machine
vector<pthread_t*> BootMap;                   // which booter is starting which board
vector<FnMap_t*>    FnMapx;
char                Port[MPI_MAX_PORT_NAME];  // MPI port for this mothership

static const int    NumBoards = NUM_BOARDS_PER_BOX;
 
};

//==============================================================================

#endif





#ifndef __MOTHERSHIP__H
#define __MOTHERSHIP__H

#include <deque>
#include "SBase.h"
#include "Debug.h"
#include "PMsg_p.hpp"
#include "Cli.h"
#include "HostLink.h"
#include "pthread.h"
#include "TaskInfo.h"
#include "poets_msg.h"
#include "P_addr.h"

//==============================================================================

class Mothership : public SBase, public HostLink
{

public:
                      Mothership(int,char **,string);
virtual ~             Mothership();

unsigned              Connect(string="");
 
// somewhat bodgey function to return the hardware address from the composite; needed because of
// the actual hardware mappings; this is the Orchestrator-side equivalent of toAddr.
static inline unsigned GetHWAddr(P_addr& VAddr) {return (VAddr.A_box << P_BOX_HWOS) |
                                                    (VAddr.A_board << P_BOARD_HWOS) |
                                                (VAddr.A_mailbox << P_MAILBOX_HWOS) |
                                                      (VAddr.A_core << P_CORE_HWOS) |
                                                  (VAddr.A_thread << P_THREAD_HWOS);};
// static void*          LoadBoard(void*); // threaded version of bootloader
static void*          Twig            (void*); // thread to handle Tinsel messages

private:
unsigned              Boot            (string);
unsigned              CmLoad          (string);
unsigned              CmRun           (string);
unsigned              CmStop          (string);
unsigned              ConfigDir       (PMsg_p *, unsigned);
unsigned              ConfigDistribute(PMsg_p *, unsigned);
unsigned              ConfigRecall    (PMsg_p *, unsigned);
unsigned              ConfigState     (PMsg_p *, unsigned);
#include              "SDecode.cpp"
inline virtual string Dname(){ return typeid(*this).name(); }
void                  Dump            (FILE * = stdout, string = "");
unsigned              DumpAll         (PMsg_p *, unsigned);
unsigned              DumpSummary     (PMsg_p *, unsigned);
unsigned              DumpTask        (PMsg_p *, unsigned);
long                  LoadBoard       (P_board*); // unthreaded version of bootloader
unsigned              OnCfg           (PMsg_p *,unsigned);
unsigned              OnCmnd          (PMsg_p *,unsigned);
unsigned              OnDump          (PMsg_p *,unsigned);
unsigned              OnExit          (PMsg_p *,unsigned);
void                  OnIdle();
unsigned              OnSend          (PMsg_p *, unsigned);
unsigned              OnSuper         (PMsg_p *,unsigned);
unsigned              OnSyst          (PMsg_p *,unsigned);
unsigned              OnTinsel        (PMsg_p*, unsigned);
unsigned              OnTinselOut     (P_Sup_Msg_t *);
void                  StopTwig();
// forwarded messages containing POETS packets. The Mothership 
// should unpack the packets contained and inject them using
// the HostLink. (This could be placed in a Branch thread).
unsigned              SendAttr        (PMsg_p *, unsigned);
unsigned              SendDevIAll     (PMsg_p *, unsigned);
unsigned              SendDevIByName  (PMsg_p *, unsigned);
unsigned              SendDevIByID    (PMsg_p *, unsigned);
unsigned              SendDevT        (PMsg_p *, unsigned);
unsigned              SendExtn        (PMsg_p *, unsigned);
unsigned              SendSupv        (PMsg_p *, unsigned);
unsigned              SystHW          (const vector<string>&);
unsigned              SystKill();
unsigned              SystShow();
unsigned              SystTopo();

void*                 SuperHandle; // dynamically loadable supervisor
int                   (*SupervisorCall)(PMsg_p*, PMsg_p*); // entry point for the Supervisor
 
public:
unsigned              PAddress; // address of this mothership in POETS-space
bool                  ForwardMsgs;
 
typedef unsigned (Mothership::*pMeth)(PMsg_p *,unsigned);
typedef map<unsigned,pMeth> FnMap_t;
typedef map<uint16_t,char*> PinBuf_t; // type to hold buffers for messages received from devices

map<uint32_t,deque<P_Msg_t>*> TwigExtMap;     // dynamic queues for messages bound for external devices
map<uint32_t,PinBuf_t*> TwigMap;              // dynamic buffer state for each device from which the Mothership is receiving
map<string, TaskInfo_t*> TaskMap;             // which tasks are mapped to the machine
vector<pair<pthread_t*,int*>> BootMap;        // which booter is starting which board
vector<FnMap_t*>    FnMapx;
pthread_t           Twig_thread;              // thread for processing tinsel connections
bool                twig_running;             // flag to show twig thread is active

static const int    NumBoards = NUM_BOARDS_PER_BOX;
 
};

//==============================================================================

#endif





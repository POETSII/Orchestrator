//------------------------------------------------------------------------------

#include "evpump.h"
#include <stdio.h>
#include "macros.h"
#include <time.h>
#include <algorithm>
#include <set>
#include <iostream>

//==============================================================================

//template <class T_t> unsigned long  EvPump<T_t>::_StopCond::wallti0 = 0;
unsigned long EvPump::_StopCond::wallti0 = 0;
EvPump *      EvPump::BASE_EVENT::par    = 0;

//==============================================================================

EvPump::EvPump(void * p)
// The event handling subsystem. Even though we're in single-thread-land, some
// algorithms lend themselves naturally to a message queue and pump.
{
par = p;                               // Who owns me?
Reset();                               // Initialise everything
EvPump::BASE_EVENT::par = this;
}

//------------------------------------------------------------------------------

EvPump::~EvPump(void)
{
Reset();                               // Kill everything
}

//------------------------------------------------------------------------------

void EvPump::Dump()
{
printf("EvPump Dump----------\n");
printf("sflag          = %s\n",sflag ? "TRUE" : "FALSE");
printf("oflag          = %s\n",oflag ? "TRUE" : "FALSE");
printf("Callback       = %p\n",pCB);
printf("evmax          = %u\n",evmax);
printf("evcnt          = %u\n",evcnt);
//StopCond.Dump();
printf("Q (size %u): \n",(unsigned)Q.size());
if (oflag) WALKDEQUE(BASE_EVENT *,Q,i) {
  printf("%s",i==Q.begin()?"<- ":"   ");
  (*i)->Dump1();
  printf("\n");
}
else WALKDEQUE(BASE_EVENT *,Q,i) {
  printf("%s",i!=Q.end()-1?"   ":"<- ");
  (*i)->Dump1();
  printf("\n");
}
printf("\nEND EvPump Dump----------\n\n");
}

//------------------------------------------------------------------------------

int EvPump::Go()
// Loop through the message pump until either we run out of events, or a stop
// condition is triggered.
{
if (pCB==0) return STOP_NOCB;          // No callback -> no point in popping
while(!Q.empty()) {
  if (sflag) return STOP_REQ;          // Asychronous stop flag set (somewhere)
                                       // Behead the heap if necessary
  if (oflag) pop_heap(Q.begin(),Q.end(),lt_BASE_EVENT());
  BASE_EVENT * pE = Q.back();          // Event to be handled
  Q.pop_back();                        // Extract from deque
  evcnt++;                             // Count the event
  StopCond.SetS_now(pE->Time());       // Advance simulation time
  int s = StopCond.Stop(pE);           // Any stop condition triggered?
  pCB(par,pE);                         // Handle the event
  if (s!=STOP_NO) return s;            // Stop triggered?
}
return STOP_NOQ;                       // Q empty
}

//------------------------------------------------------------------------------

void EvPump::Inject(BASE_EVENT * pBASE)
// Insert a new event into the queue.
{
if (oflag) {                           // If it's an ordered queue
  Q.push_back(pBASE);
  push_heap(Q.begin(),Q.end(),lt_BASE_EVENT());
}
else Q.push_front(pBASE);              // If it's a FIFO
unsigned s = Q.size();                 // Collect some stats
if (s>evmax) evmax = s;
}

//------------------------------------------------------------------------------

void EvPump::Order(bool b)
// (Re)sets the flag that makes the ordered (TRUE) or FIFO (FALSE)
{
oflag = b;
if (oflag) make_heap(Q.begin(),Q.end(),lt_BASE_EVENT());
}

//------------------------------------------------------------------------------

void EvPump::Purge(void(* pFn)(void *,BASE_EVENT *))
// To help clear up any events left in the queue after the pump has stopped
// for whatever reason: the callback is executed *once* on every distinct value
// in the queue
{
set<BASE_EVENT *> X;                   // Copy to a set to avoid duplicates
WALKDEQUE(BASE_EVENT *,Q,i) X.insert(*i);
                                       // And do it (whatever it is)
WALKSET2(BASE_EVENT *,X,i) (*pFn)(this,*i);
}

//------------------------------------------------------------------------------

void EvPump::Reset()
// Reset the whole event pump object. Note the event queue is emptied, but
// it is the responsibility of any external code to delete anything that needs
// deleting
{
Q.clear();                             // Kill the queue itself
StopCond.Reset();                      // Reset the stop condition object
evmax = 0;                             // Peak-hold event counter
evcnt = 0;                             // Event counter
pCB = 0;                               // Disconnect the callback
sflag = false;                         // Unset the "stop" flag
oflag = false;                         // Unset the "order" flag
}

//------------------------------------------------------------------------------

void EvPump::Set_CB(void(* pFn)(void *,BASE_EVENT *))
// Push in the callback function. Note some compilers won't allow default
// arguments, so if you want to kill this asynchronously you have to explicitly
// call Set_CB(0)
{
pCB = pFn;
}

//------------------------------------------------------------------------------

void EvPump::Stats(int & rEvcnt, int & rEvmax)
{
rEvcnt = evcnt;                        // Events popped to date
rEvmax = evmax;                        // Maximum size of the event queue so far
}

//------------------------------------------------------------------------------

void EvPump::Stop(bool f)
// Set the stop flag. Calling this from within the callback gives a way for the
// simulation to stop itself; the flag is read by the event pump each time
// round.
{
sflag = f;
}

//==============================================================================

EvPump::BASE_EVENT::BASE_EVENT(BASE_EVENT * pB)
{
time = pB->time;
tag  = pB->tag;
type = pB->type;
}

//------------------------------------------------------------------------------

void EvPump::BASE_EVENT::Dump()
{
printf("BASE_EVENT Dump\n");
printf("time = "); cout << time; printf("\n");
printf("type = %d\n",type);
printf("tag  = %p\n",tag);
printf("(Wallclock = %lu ms)\n",_StopCond::Delta());
printf("\n");
}

//------------------------------------------------------------------------------

void EvPump::BASE_EVENT::Dump1()
// One line Dump to go into the event handler Dump
{
printf("%p->%p [%d] @ t= ",this,tag,type);
cout << time;
}

//==============================================================================

EvPump::_StopCond::_StopCond()
// Event pump stopping condition subclass
{
Reset();                               // Kill everything
}

//------------------------------------------------------------------------------

unsigned long EvPump::_StopCond::Delta()
{
unsigned long now = (1000UL * (unsigned long)(clock()))/CLOCKS_PER_SEC;
return now - wallti0;
}

//------------------------------------------------------------------------------

void EvPump::_StopCond::Dump()
{
printf("Stop condition dump----------\n\n");
printf("Interrupt count stop condition set : %s\n",iB? "YES" : "NO");
printf("Interrupt count stop limit         : %u\n",ints);
printf("Current interrupt count            : %u\n\n",icnt);

printf("Packet count stop condition set    : %s\n",pB? "YES" : "NO");
printf("Packet count stop limit            : %u\n",pckt);
printf("Current packet count               : %u\n\n",pcnt);

printf("Simulation time stop condition set : %s\n",sB? "YES" : "NO");
printf("Simulation time stop limit         : %u\n",simt);
printf("Current simulation time            : %u\n\n",sima);

printf("Wallclock time stop condition set  : %s\n",wB? "YES" : "NO");
printf("Wallclock time stop limit          : %lu ms\n",wallti);
printf("Delta_t from start to now          : %lu ms\n",Delta());
printf("(Absolute wallclock time at start  : %lu ms)\n\n",wallti0);

printf("END Stop condition dump----------\n\n");
}

//------------------------------------------------------------------------------

void EvPump::_StopCond::Reset()
// Reset all the stopping conditions
{
iB      = false;                       // Care about interrupt count?
ints    = 0;                           // Current interrupt count
icnt    = 0;                           // Interrupt count limit

pB      = false;                       // Care about packet count?
pckt    = 0;                           // Current packet count
pcnt    = 0;                           // Packet count limit

sB      = false;                       // Care about simulation time?
simt    = 0;                           // Current simulation time
sima    = 0;                           // Simulation time limit

wB      = false;                       // Care about wallclock time?
wallti  = 0UL;                         // Wallclock stop limit
                                       // Time now (i.e. at reset)
wallti0 = (1000UL * (unsigned long)(clock()))/CLOCKS_PER_SEC;
}

//------------------------------------------------------------------------------

void EvPump::_StopCond::SetI(unsigned ilim)
// Set the interrupt count stop condition
{
ints = ilim;                           // Numeric limit
iB   = true;                           // Yes we care
}

//------------------------------------------------------------------------------

void EvPump::_StopCond::SetP(unsigned plim)
// Set the packet count stop condition
{
pckt = plim;                           // Numeric limit
pB   = true;                           // Yes we care
}

//------------------------------------------------------------------------------

void EvPump::_StopCond::SetS(unsigned tlim)
// Set the simulated time stop condition
{
simt = tlim;                           // Numeric limit
sB   = true;                           // Yes we care
}

//------------------------------------------------------------------------------

void EvPump::_StopCond::SetS_now(unsigned t_now)
// Set the current simulation time
{
sima = t_now;
}

//------------------------------------------------------------------------------

void EvPump::_StopCond::SetW(unsigned long wlim)
// Set the elapsed wallclock time stop condition
{
wallti = wlim;                         // Numeric limit
wB     = true;                         // Yes we care
}

//------------------------------------------------------------------------------

int EvPump::_StopCond::Stop(BASE_EVENT * pE)
// Stop test; called every time round the message loop.
// ANY return value apart from STOP_NO will stop the event pump.
{
                                       // If we care AND wallclock too much...
if (wB&&(Delta() > wallti)) return EvPump::STOP_WALL;
/*                                       // The rest are type-specific
switch (pE->Type()) {
  case BASE_EVENT::E_MP3 : if (pB&&(++pcnt>pckt))     return EvPump::STOP_PCKT;
                           if (sB&&(pE->Time()>simt)) return EvPump::STOP_SIMT;
                           break;
  case BASE_EVENT::E_INT : if (iB&&(++icnt>ints))     return EvPump::STOP_INTS;
                           if (sB&&(pE->Time()>simt)) return EvPump::STOP_SIMT;
                           break;
  case BASE_EVENT::E_CFG : return EvPump::STOP_NO;
  default                : return EvPump::STOP_UNK;
} */
return EvPump::STOP_NO;
}

//------------------------------------------------------------------------------

//==============================================================================



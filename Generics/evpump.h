#ifndef __EvPumpH__H
#define __EvPumpH__H

#include <stdio.h>
#include <deque>
#include <vector>
#include <string>
using namespace std;

//==============================================================================

typedef           double        EvTime_t;

//==============================================================================

class EvPump
{
public:
class             BASE_EVENT;
                  EvPump(void * = 0);
virtual ~         EvPump(void);
void              Dump();
int               Go();
void              Inject(BASE_EVENT *);
void              Order(bool=true);
void              Purge(void(*)(void *,BASE_EVENT *));
void              Reset();
void              Set_CB(void(*)(void *,BASE_EVENT *));
unsigned          Size() { return Q.size(); }
void              Stats(int &,int &);
void              Stop(bool=true);

static const int  STOP_NO   = 0;       // Don't stop
static const int  STOP_NOCB = 1;       // No callback supplied
static const int  STOP_NOQ  = 2;       // Q empty
static const int  STOP_REQ  = 3;       // Asynch stop request issued
static const int  STOP_UNK  = 4;       // Q popped unknown/BASE event type
static const int  STOP_PCKT = 5;       // Packet count limit reached
static const int  STOP_SIMT = 6;       // Simulation time limit reached
static const int  STOP_INTS = 7;       // Interrupt limit reached
static const int  STOP_WALL = 8;       // Wallclock time limit reached

private:
deque<BASE_EVENT *> Q;                 // The event queue
bool                sflag;             // Asynchronous stop flag
bool                oflag;             // Heap/FIFO flag
unsigned            evmax;             // Largest size of Q since last reset
unsigned            evcnt;             // Popped event count
void(* pCB)(void *,BASE_EVENT *);      // Pump event callback pointer
void *              par;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class BASE_EVENT
// Event base class. In principle, there's no reason - that I can see - why this
// can't be abstract, but on the other hand, who cares?
// In the sense that Dump() is virtual, it's polymorphic; the design intention
// is that you derive from it for more complicated events.
{
friend struct lt_BASE_EVENT;
friend class EvPump;
public:
                  BASE_EVENT()     :time(0),tag(0),type(0){}
                  BASE_EVENT(int t):time(0),tag(0),type(t){}
                  BASE_EVENT(BASE_EVENT *);
virtual ~         BASE_EVENT(void){}
virtual void      Dump();
virtual void      Dump1();
void              Tag(void * p)           { tag = p;               }
void *            Tag()                   { return tag;            }
void              Time(EvTime_t t)        { time = t;              }
EvTime_t          Time()                  { return time;           }
void              Type(int t)             { type = t;              }
int               Type()                  { return type;           }
BASE_EVENT &      operator+=(EvTime_t t)  { time+=t; return *this; }

protected:
EvTime_t          time;                // Simulation timestamp
void *            tag;                 // This is just *sooooo* useful....
int               type;                // Event type
static EvPump *   par;                 // The pump itself
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class _StopCond
// Handles the set(s) of conditions that dictate when the simulation should stop
{
public:
                  _StopCond();
virtual ~         _StopCond(void){}
static
unsigned long     Delta();             // Time in ms since the last Reset()
void              Dump();
void              Reset();             // Clear all the stop conditions
void              SetI(unsigned);      // Set interrupt count stop condition
void              SetP(unsigned);      // Set packet count stop condition
void              SetS(unsigned);      // Set simulation time stop condition
void              SetS_now(unsigned);  // Inform current simulation time
void              SetW(unsigned long); // Set wallclock time stop condition
int               Stop(BASE_EVENT *);  // So does we stop or not?

private:
bool              iB;                  // Interrupt count stop condition set
unsigned          ints;                // Interrupt count stop limit
unsigned          icnt;                // Actual interrupt count

bool              pB;                  // Packet count stop condition set
unsigned          pckt;                // Packet count stop limit
unsigned          pcnt;                // Actual packet count

bool              sB;                  // Simulation time stop condition set
unsigned          simt;                // Simulation time stop limit
unsigned          sima;                // Actual simulation time

bool              wB;                  // Wallclock time stop condition set
unsigned long     wallti;              // Relative wallclock time stop limit
static
unsigned long     wallti0;             // Absolute wallclock time on start

} StopCond;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

};

//==============================================================================
// For some reason unknown to reason, the Borland debugger doesn't seem to be
// able to see this. You don't get the blobs in the LH gutter showing that the
// linker has found it, and you can't set breakpoints in it. BUT it has been
// found and linked, because if you put a printf statement in it you get what
// you expect.

// The invocation syntax is bloody weird. You have to call the heap functions
// like "push_heap(Q.begin(),Q.end(),lt_BASE_EVENT());" - note the () on the 3rd
// argument, which is supposed to be a bloody operator address. Surely this
// syntax should just execute the bloody thing?
// In the Borland STL help, there's all sorts of stuff about deriving
// lt_BASE_EVENT from some library base class, but it seems to work OK if you
// don't bother.

struct lt_BASE_EVENT
{
bool operator()(EvPump::BASE_EVENT * pa,EvPump::BASE_EVENT * pb) const
{
return pa->time > pb->time;
}
};


//==============================================================================

#endif

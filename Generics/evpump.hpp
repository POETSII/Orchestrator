#ifndef __EvPumpH__H
#define __EvPumpH__H

#include <stdio.h>
#include <deque>
#include <iostream>
using namespace std;

//==============================================================================

template <class T> class EvPump
{
public:
class               BASE_EVENT;
                    EvPump(void * = 0);
virtual ~           EvPump(void);
void                Dump();
int                 Go();
void                Inject(BASE_EVENT *);
void                Order(bool=true);
void * &            Par()              { return par;              }
void                Purge(void(*)(void *,BASE_EVENT *));
void                Reset();
void                Set_CB(void(*)(void *,BASE_EVENT *));
unsigned            Size()             { return Q.size();         }
void                Stats(unsigned &,unsigned &);
void                Stop(bool=true);

static const int    STOP_NO   = 0;     // Don't stop
static const int    STOP_NOCB = 1;     // No callback supplied
static const int    STOP_NOQ  = 2;     // Q empty
static const int    STOP_REQ  = 3;     // Asynch stop request issued
static const int    STOP_XXX  = 4;
static const char * STOP_str[STOP_XXX+1]; // STOP_str[] would do under Borland..

private:
deque<BASE_EVENT *> Q;                 // The event queue
bool                sflag;             // Asynchronous stop flag
bool                oflag;             // Heap/FIFO flag (Default is FIFO)
unsigned            evmax;             // Largest size of Q since last reset
unsigned            evcnt;             // Popped event count
void(* pCB)(void *,BASE_EVENT *);      // Pump event callback pointer
void *              par;               // Whatever holds the pump

public:
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class BASE_EVENT
// Event base class. In principle, there's no reason - that I can see - why this
// can't be abstract, but on the other hand, who cares?
// In the sense that Dump() is virtual, it's polymorphic; the design intention
// is that you derive from it for more complicated events.
{
template <class T> friend struct lt_BASE_EVENT;
friend class EvPump<T>;

public:
                  BASE_EVENT()     :time(0),tag(0),type(0){}
                  BASE_EVENT(int t):time(0),tag(0),type(t){}
                  BASE_EVENT(BASE_EVENT *);
virtual ~         BASE_EVENT(void){}
virtual void      Dump();
virtual void      Dump1();
static void       Par(EvPump * p)         { par = p;               }
static EvPump *   Par()                   { return par;            }
void              Tag(void * p)           { tag = p;               }
void *            Tag()                   { return tag;            }
void              Time(T t)               { time = t;              }
T                 Time()                  { return time;           }
void              Type(int t)             { type = t;              }
int               Type()                  { return type;           }
BASE_EVENT &      operator+=(T t)         { time+=t; return *this; }

protected:
static EvPump *   par;                 // The pump itself
T                 time;                // Simulation timestamp
void *            tag;                 // This is just *sooooo* useful....
int               type;                // Event type
};  // ...end of subclass BASE_EVENT declare

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

};  // ...end of template class EvPump

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

template <class T> struct lt_BASE_EVENT
{
bool operator()(typename EvPump<T>::BASE_EVENT * pa,
                typename EvPump<T>::BASE_EVENT * pb) const
{
return pa->time > pb->time;
}
};

//==============================================================================

#include "evpump.tpp"
#endif

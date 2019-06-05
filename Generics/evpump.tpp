//------------------------------------------------------------------------------

#include <stdio.h>
#include <typeinfo>
#include "macros.h"
#include <time.h>
#include <algorithm>
#include <set>

//==============================================================================

template <class T> EvPump<T>  * EvPump<T>::BASE_EVENT::par = 0;
template <class T> const char * EvPump<T>::STOP_str[] = {
"No stop (invalid stop code)",
"No callback supplied",
"Queue empty",
"Asynchronous stop request issued",
"Queue popped unknown event type"};

//==============================================================================

template <class T> EvPump<T>::EvPump(void * p)
// The event handling subsystem. Even though we're in single-thread-land, some
// algorithms lend themselves naturally to a message queue and pump.
{
par = p;                               // Who owns me?
Reset();                               // Initialise everything
EvPump<T>::BASE_EVENT::Par(this);      // Load the pump address into every event
}

//------------------------------------------------------------------------------

template <class T> EvPump<T>::~EvPump(void)
{
Reset();                               // Kill everything
}

//------------------------------------------------------------------------------

template <class T> void EvPump<T>::Dump()
{
printf("EvPump Dump----------\n");
printf("Time type              = %s\n",typeid(T).name());
printf("sflag (async stop req) = %s\n",sflag ? "TRUE" : "FALSE");
printf("oflag (Q/FIFO)         = %s\n",oflag ? "TRUE(Q)" : "FALSE(FIFO)");
printf("Callback               = %p\n",pCB);
printf("evmax                  = %u\n",evmax);
printf("evcnt                  = %u\n",evcnt);
cout << lpt << '\n';
printf("Q (size %u): \n",(unsigned)Q.size());
if (Q.size()==0) printf("..empty\n");
WALKDEQUE(BASE_EVENT *,Q,i) {
  if (oflag) printf("%s",i==Q.begin()?"<- ":"   ");
  else       printf("%s",i!=Q.end()-1?"   ":"<- ");
  (*i)->Dump1();
  printf("\n");
}
printf("\nEND EvPump Dump----------\n\n");
}

//------------------------------------------------------------------------------

template <class T> int EvPump<T>::Go()
// Loop through the message pump until either we run out of events, or a stop
// condition is triggered.
{
if (pCB==0) return STOP_NOCB;          // No callback -> no point in popping
while(!Q.empty()) {
  if (sflag) return STOP_REQ;          // Asychronous stop flag set (somewhere)
                                       // Behead the heap if necessary
  if (oflag) pop_heap(Q.begin(),Q.end(),lt_BASE_EVENT<T>());
  BASE_EVENT * pE = Q.back();          // Event to be handled
  Q.pop_back();                        // Extract from deque (ho ho ho)
  evcnt++;                             // Count the event
  lpt = pE->Time();                    // Store last popped time
  pCB(par,pE);                         // Handle the event
}
if (sflag) return STOP_REQ;            // In case STOP_REQ was the last one 
return STOP_NOQ;                       // Q empty
}

//------------------------------------------------------------------------------

template <class T> void EvPump<T>::Inject(typename EvPump<T>::BASE_EVENT * pBASE)
// Insert a new event into the queue.
{
if (oflag) {                           // If it's an ordered queue
  Q.push_back(pBASE);
  push_heap(Q.begin(),Q.end(),lt_BASE_EVENT<T>());
}
else Q.push_front(pBASE);              // If it's a FIFO
unsigned s = Q.size();                 // Collect some stats
if (s>evmax) evmax = s;
}

//------------------------------------------------------------------------------

template <class T> void EvPump<T>::Order(bool b)
// (Re)sets the flag that makes the ordered (TRUE) or FIFO (FALSE)
{
oflag = b;
if (oflag) make_heap(Q.begin(),Q.end(),lt_BASE_EVENT<T>());
}

//------------------------------------------------------------------------------

template <class T> void EvPump<T>::Purge(void(* pFn)(void *,typename EvPump<T>::BASE_EVENT *))
// To help clear up any events left in the queue after the pump has stopped
// for whatever reason: the callback is executed *once* on every distinct value
// in the queue
{
set<BASE_EVENT *> X;                   // Copy to a set to avoid duplicates
WALKDEQUE(BASE_EVENT *,Q,i) X.insert(*i);
                                       // And do it (whatever it is)
WALKSET(BASE_EVENT *,X,i) (*pFn)(this,*i);
}

//------------------------------------------------------------------------------

template <class T> void EvPump<T>::Reset()
// Reset the whole event pump object. Note the event queue is emptied, but
// it is the responsibility of any external code to delete anything that needs
// deleting
{
Q.clear();                             // Kill the queue itself
evmax = 0;                             // Peak-hold event counter
evcnt = 0;                             // Event counter
pCB = 0;                               // Disconnect the callback
sflag = false;                         // Unset the "stop" flag (to "unordered")
oflag = false;                         // "order" == FIFO
lpt = T(0);                            // Last popped time
}

//------------------------------------------------------------------------------

template <class T> void EvPump<T>::Set_CB(void(* pFn)(void *,typename EvPump<T>::BASE_EVENT *))
// Push in the callback function. Note some compilers won't allow default
// arguments, so if you want to kill this asynchronously you have to explicitly
// call Set_CB(0)
{
pCB = pFn;
}

//------------------------------------------------------------------------------

template <class T> void EvPump<T>::Stats(unsigned & rEvcnt,unsigned & rEvmax)
{
rEvcnt = evcnt;                        // Events popped to date
rEvmax = evmax;                        // Maximum size of the event queue so far
}

//------------------------------------------------------------------------------

template <class T> void EvPump<T>::Stop(bool f)
// Set the stop flag. Calling this from within the callback gives a way for the
// simulation to stop itself; the flag is read by the event pump each time
// round.
{
sflag = f;
}

//==============================================================================

template <class T> EvPump<T>::BASE_EVENT::BASE_EVENT(BASE_EVENT * pB)
{
time = pB->time;
tag  = pB->tag;
type = pB->type;
}

//------------------------------------------------------------------------------

template <class T> void EvPump<T>::BASE_EVENT::Dump()
{
printf("----------------------------------------------------\n");
printf("BASE_EVENT Dump\n");
printf("Time type    = %s\n",typeid(T).name());
printf("(static) par = %p\n",par);
printf("time         = "); cout << time; printf("\n");
printf("type         = %d\n",type);
printf("tag          = %p\n",tag);
printf("----------------------------------------------------\n\n");
}

//------------------------------------------------------------------------------

template <class T> void EvPump<T>::BASE_EVENT::Dump1()
// One line Dump to go into the event handler Dump
{
printf("%p:%p [%d] @ t= ",this,tag,type);
cout << time;
}

//==============================================================================



//------------------------------------------------------------------------------

#include <stdio.h>
#include "macros.h"
#include "timer.h"

//==============================================================================
// Keep the linker happy
double (*Timer_t::pClock)(void) = 0;
map<int,double> Timer_t::Tdat;

//==============================================================================

Timer_t::Timer_t()
{
}

//------------------------------------------------------------------------------

Timer_t::~Timer_t()
{
}

//------------------------------------------------------------------------------

void Timer_t::Clock(double(* pC)(void))
{
pClock = pC;                           // Attach the clock
}

//------------------------------------------------------------------------------

void Timer_t::Dump()
{
printf("\n==============================\nTimer_t dump\n");
printf("Clock : %p\n",pClock);
if (pClock!=0) printf("  (Time now is %15.8e)\n",(*pClock)());
printf("Timer_t map :\n id : started\n");
WALKMAP(int,double,Tdat,i) printf("%3d : %15.8e\n",(*i).first,(*i).second);
printf("End of Timer_t dump===========\n");
}

//------------------------------------------------------------------------------

double Timer_t::Read(int t)
// Read the timer without disturbing it
{
if (pClock==0) return 0.0;             // No clock?
if (Tdat.find(t)==Tdat.end()) return 0.0;  // Timer not there?
return (*pClock)() - Tdat[t];          // And the elapsed time is....
}

//------------------------------------------------------------------------------

double Timer_t::Reset(int t)
// Reset a timer
{
if (pClock==0) return 0.0;             // No clock?
if (Tdat.find(t)==Tdat.end()) return 0.0; // Timer NOT there? Ignore request
double ans = (*pClock)() - Tdat[t];    // What was the time?
Tdat[t] = (*pClock)();                 // Reset it
return ans;
}

//------------------------------------------------------------------------------

void Timer_t::Start(int t)
// Start a new timer
{
if (pClock==0) return;                 // No clock?
if (Tdat.find(t)!=Tdat.end()) return;  // Timer already there? Ignore request
Tdat[t] = (*pClock)();                 // Then start/reset it......
}

//------------------------------------------------------------------------------

double Timer_t::Stop(int t)
// Stop the timer and output the delay
{
if (pClock==0) return 0.0;             // No clock?
double ans = Read(t);                  // Read it
Tdat.erase(t);                         // erase() is tolerant of dud argument
return ans;                            // And the elapsed time was.....
}

//------------------------------------------------------------------------------



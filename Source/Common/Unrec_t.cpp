//------------------------------------------------------------------------------

#include "Unrec_t.h"
#include <stdio.h>

//==============================================================================

Unrec_t::Unrec_t(unsigned _i,string _d,string _r)
{
i = _i;                                // Error number
d = _d;                                // Derived parent class/process
r = _r;                                // Routine wot throws
}

//------------------------------------------------------------------------------

Unrec_t::~Unrec_t()
{
}

//------------------------------------------------------------------------------

void Unrec_t::Post()
{
printf("%s: UNRECOVERABLE INCIDENT %u from method %s:\n",d.c_str(),i,r.c_str());
switch(i) {
  case 0  : printf("\nNull exception...???   \n");            break;
  case 1  : printf("\nMessage source process has no rank\n"); break;
  case 2  : printf("\nLogServer process has no rank\n");      break;
  default : printf("\nUnknown exception...???   \n");         break;
}
printf("\nProcess %s will probably close cleanly;\n"
       "(unless it's multi-threaded). What is left of the rest of any \n"
       "Universe and/or thread set is orphaned - \n"
       "can't see it, can't close it\n\n"
       "SUBSEQUENT EXECUTION AND OUTPUT IS UNRELIABLE\n\n",d.c_str());
fflush(stdout);
}

//==============================================================================


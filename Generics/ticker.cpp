//------------------------------------------------------------------------------

#include "ticker.h"

//==============================================================================

bool Ticker::silent = false;

//==============================================================================

Ticker::Ticker()
{
ping_count = 0;
}

//------------------------------------------------------------------------------

Ticker::~Ticker()
{
}

//------------------------------------------------------------------------------

void Ticker::Start()
{
time_start = clock();
ping_count = 0;                        // Integral of pings received
}

//------------------------------------------------------------------------------

void Ticker::Tweet(string s)
{
if (silent) return;                    // Nothing to the console
if (time_start == clock_t(-1)) return; // Gotta call Start() first
ping_count++;                          // Update integral

clock_t t_delta = clock()-time_start;  // Guess

if (t_delta==0) {                      // Too small to measure?
  printf("--:--:--.--: %s %ld ~ 0 time      \r",s.c_str(),ping_count);
  return;
}
                                       // Turn it into a 'per seconds' rate
long p_per_t = (CLOCKS_PER_SEC*ping_count)/t_delta;
printf("--:--:--.--: %s %ld @ %ld /s      \r",s.c_str(),ping_count,p_per_t);
}

//------------------------------------------------------------------------------

void Ticker::VTab()
// Linux won't let me inline this
{
printf("\n\n");
}

//==============================================================================



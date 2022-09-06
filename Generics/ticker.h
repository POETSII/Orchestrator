//------------------------------------------------------------------------------

#ifndef __Ticker__H
#define __Ticker__H

#include <stdio.h>
#include <string>
#include <time.h>
using namespace std;

//==============================================================================

class Ticker
{
public:
              Ticker();
virtual ~     Ticker();
void          Start();
static void   Silent(bool s = false) { silent = s; }
void          Tweet(string);
void          VTab();

private:
clock_t       time_start;
long          ping_count;
static bool   silent;

};

//==============================================================================

#endif

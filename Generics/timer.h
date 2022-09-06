#ifndef __timer__H
#define __timer__H

#include <map>
using namespace std;

//==============================================================================

class Timer_t
{
public:
                   Timer_t();
virtual ~          Timer_t(void);
void               Clock(double(*)(void));
void               Dump();
double             Read(int);
double             Reset(int);
void               Start(int);
double             Stop(int);

private:
static map<int,double>        Tdat;
static double(*pClock)(void);
};

//==============================================================================

#endif

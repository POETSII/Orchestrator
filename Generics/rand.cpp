//------------------------------------------------------------------------------

#include "rand.h"
#include <math.h>
#include <stdio.h>

//==============================================================================

RandInt::RandInt(long s)
// Uniform distribution assuming 32-bit long
{
randx = s;
}

//------------------------------------------------------------------------------

void RandInt::seed(long s)
{
randx = s;
}

//------------------------------------------------------------------------------

long RandInt::abs(long x)
{
return x & 0x7fffffff;
}

//------------------------------------------------------------------------------

double RandInt::max()
{
return 2147483648.0;
}

//------------------------------------------------------------------------------

long RandInt::draw()
{
return randx = randx * 1103515245 + 12345;
}

//------------------------------------------------------------------------------

double RandInt::fdraw()
// Return value in the interval [0,1]
{
return abs(draw())/max();
}

//------------------------------------------------------------------------------

long RandInt::operator()()
// Return value in the interval [0,2^31]
{
return abs(draw());
}

//==============================================================================

Urand::Urand()
{
n=0;
}

//------------------------------------------------------------------------------

Urand::Urand(long nn)
// Uniform distribution in the interval [0,n]
// Except it isn't. Measurements show 0..n-1.
// You HAVE to use this constructor OR Urand::Urand() followed by box(....)
// or it returns -1 all the time.
{
n = nn;
}

//------------------------------------------------------------------------------

void Urand::box(long nn)
{
n = nn;
}

//------------------------------------------------------------------------------

long Urand::operator()()
{
long r = long(double(n) * fdraw());
//printf("Urand returns %lu in [0,%lu]\n",  (r==n) ? n-1 : r, n);
return (r==n) ? n-1 : r;
}

//==============================================================================

Erand::Erand(long m)
// Exponential distribution
{
mean = m;
}

//------------------------------------------------------------------------------

long Erand::operator()()
{
return -mean * long(log((max()-draw())/max() + 0.5));
}

//==============================================================================

bool P(double p)
// Return TRUE with a probability of p
{
static Urand RNG;
return RNG.fdraw() < p;
}

//------------------------------------------------------------------------------

void test()
// Test function for P(p) above. It all seems tickety-spong.
{
unsigned goes=10000;
unsigned bin[2] = {0,0};
for (double p=0.0;p<1.0;p+=0.1) {
  for(unsigned i=0;i<goes;i++) {
    if (P(p)) bin[0]++; else bin[1]++;;
    printf("bin[0]=%3u bin[1]=%3u\n",bin[0],bin[1]);
  }
  printf("P(%10.2e) %%T = %10.2e\n",p,double(bin[0])/(double(bin[1])+double(bin[0])));
  bin[0] = bin[1] = 0;
}
}

//==============================================================================


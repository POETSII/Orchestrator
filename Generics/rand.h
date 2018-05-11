#ifndef __RandInt__H
#define __RandInt__H

// Copied verbatim from Stroustrup White Book Edition 1 p686

// long == signed long int, but experiments show it only ever returns +ve values
//==============================================================================

class RandInt {
public:
                   RandInt(long=0);
virtual ~          RandInt(void){}
void               seed(long);
long               abs(long);
static double      max();
long               draw();
double             fdraw();
long               operator()();

private:
unsigned long      randx;
};

//==============================================================================

class Urand : public RandInt {
public:
                   Urand();
                   Urand(long);
void               box(long);
long               operator()();

private:
long               n;
};

//==============================================================================

class Erand : public RandInt {
public:
                   Erand(long);
long               operator()();

private:
long               mean;
};

//==============================================================================

bool               P(double);

//==============================================================================






























#endif

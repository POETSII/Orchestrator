#ifndef __Histogram__H
#define __Histogram__H

#include <stdio.h>
#include <map>
using namespace std;

//==============================================================================

class Histogram {
public:
                   Histogram(unsigned=1);
virtual ~          Histogram(void){}

void               Add(unsigned,int=1);
unsigned           Bin();
unsigned           Bin(unsigned);
void               Dump(FILE * = stdout);
int                Get(unsigned);
unsigned           InBin(unsigned,unsigned &,unsigned &);
unsigned           Key(unsigned);
void               MaxBin(unsigned &,unsigned &);
double             Mean();
void               MinBin(unsigned &,unsigned &);
void               Pad();
void               ReBin(unsigned);
void               Reset();
void               Unpad();
double             Variance();

private:
map<unsigned,int>  D;
unsigned           binsiz;

};

//==============================================================================

#endif

#ifndef __TraceH__H
#define __TraceH__H

#include <stdio.h>
//#include "NameBase.h"
using namespace std;
#include <string>

//==============================================================================

class Trace
{
public:
                   Trace();
virtual ~          Trace();

void               Close();
void               Dump(unsigned = 0,FILE * = stdout);
void               Init();
void               Push(string);
void               Push(string,string);
void               Push(string,unsigned);
void               PushS(string);

private:
bool               alive;
FILE *             ft;

};

//==============================================================================

#endif





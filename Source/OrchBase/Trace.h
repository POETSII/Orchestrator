#ifndef __TraceH__H
#define __TraceH__H

#include <stdio.h>
#include "NameBase.h"
//#include "P_board.h"
//#include "Bin.h"
//#include "P_addr.h"
//class P_graph;
//class Config_t;
//#include "pdigraph.hpp"
//#include <vector>
using namespace std;
#include <string>

//==============================================================================

class Trace
{
public:
                   Trace();
virtual ~          Trace();

void               Close();
void               Dump(FILE * = stdout);
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





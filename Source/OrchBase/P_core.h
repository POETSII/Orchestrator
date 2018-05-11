#ifndef __P_coreH__H
#define __P_coreH__H

#include <stdio.h>
#include "P_thread.h"
#include "P_board.h"
#include "Bin.h"
#include <vector>
using namespace std;

//==============================================================================

class P_core : public NameBase
{
public:
                   P_core(P_board *,string=string());
virtual ~          P_core();

void               Dump(FILE * = stdout);

P_addr             addr;
vector<P_thread *> P_threadv;
Bin *              pCoreBin;
P_board *          par;

};

//==============================================================================

#endif





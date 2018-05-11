#ifndef __P_boardH__H
#define __P_boardH__H

#include <stdio.h>
#include "NameBase.h"
#include "P_addr.h"
#include <vector>
using namespace std;
class P_core;
class P_box;
class P_device;

//==============================================================================

class P_board : public NameBase
{
public:
                   P_board(P_box *,string=string());
virtual ~          P_board();

void               Dump(FILE * = stdout);

P_addr             addr;
vector<P_core *>   P_corev;
P_box *            par;
P_device *         pSup;
unsigned           uMem;

};

//==============================================================================

#endif





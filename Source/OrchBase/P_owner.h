#ifndef __P_ownerH__H
#define __P_ownerH__H

#include <stdio.h>
#include "NameBase.h"
#include "P_task.h"
//#include "macros.h"
#include <vector>
using namespace std;

//==============================================================================

class P_owner : public NameBase
{
public:
                    P_owner();
                    P_owner(string);
virtual ~           P_owner();

void                Disown();
void                Disown(P_task *);
void                Dump(FILE * = stdout);
void                Own(P_task *);

vector<P_task *>    P_taskv;

};

//==============================================================================

#endif





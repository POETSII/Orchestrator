#ifndef __P_threadH__H
#define __P_threadH__H

#include <stdio.h>
#include "NameBase.h"
#include "Bin.h"
#include "P_device.h"
#include <list>
using namespace std;
class P_core;

//==============================================================================

class P_thread : public NameBase
{
public:
                    P_thread(P_core *,string=string());
virtual ~           P_thread();

void                Dump(FILE * = stdout);

list<P_device *>    P_devicel;
P_core *            par;
P_addr              addr;

};

//==============================================================================

#endif





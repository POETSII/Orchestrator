#ifndef __P_typdclH__H
#define __P_typdclH__H

#include <stdio.h>
#include "NameBase.h"
#include "P_message.h"
#include "PoetsBoard.h"
#include "Bin.h"
class D_graph;
class P_devtyp;
class P_task;
class OrchBase;

#include <vector>
#include <list>
using namespace std;

//==============================================================================

class P_typdcl : public NameBase
{
public:
                    P_typdcl(OrchBase *,string);
virtual ~           P_typdcl();

void                Dump(FILE * = stdout);

OrchBase *          par;
vector<P_devtyp *>  P_devtypv;
vector<P_message *> P_messagev;
list<P_task *>      P_taskl;
vector<CFrag *>     General;
CFrag *             pPropsI;
CFrag *             pPropsD;

};

//==============================================================================

#endif





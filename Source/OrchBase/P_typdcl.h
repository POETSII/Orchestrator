#ifndef __P_typdclH__H
#define __P_typdclH__H

#include <stdio.h>
#include "NameBase.h"
#include "P_message.h"
#include "P_board.h"
#include "Bin.h"
class D_graph;
class P_devtyp;
class P_task;
class OrchBase;
// class P_datatype;

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
map<string, P_devtyp*> P_devtypm;     // device and message type vectors replaced
map<string, P_message*> P_messagem;   // with maps for easy lookup during parse
// map<string, P_datatype*> P_typdefm;
// vector<P_devtyp *>  P_devtypv;
// vector<P_message *> P_messagev;
list<P_task *>      P_taskl;
vector<CFrag *>     General;
// P_datatype*         pProps;        // Global properties (V3)
CFrag *             pPropsI;          // Global properties initialiser (V4)
CFrag *             pPropsD;          // Global properties declaration (V4)

};

//==============================================================================

#endif





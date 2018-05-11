#ifndef __P_taskH__H
#define __P_taskH__H

#include <stdio.h>
#include "NameBase.h"
#include "OrchBase.h"
class P_owner;
//#include "P_owner.h"

//==============================================================================

class P_task : public NameBase
{
public:
                    P_task(OrchBase *,string);
virtual ~           P_task();

void                Clear();
void                Dump(FILE * = stdout);
bool                IsPoL();
void                LinkFlag(bool f = true){ linked = f; }

OrchBase *          par;               // Parent
P_super *           pSup;              // Supervisor device for this task
D_graph *           pD;                // Device graph
P_typdcl *          pP_typdcl;         // Declare block
string              filename;          // XML source from HbD-land
P_owner *           pOwn;              // Task owner
bool                linked;

struct PoL_t {
public:
PoL_t();
void Dump(FILE * = stdout);
bool IsPoL;
string type;
vector<string> params;
};

PoL_t PoL;

};

//==============================================================================

#endif





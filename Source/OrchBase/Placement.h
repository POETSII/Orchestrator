#ifndef __PlacementH__H
#define __PlacementH__H

#include <algorithm>
#include <stdio.h>
#include "Constraints.h"
#include "HardwareAddress.h"
#include "HardwareIterator.h"
#include "OrchBase.h"
#include "pdigraph.hpp"
#include "P_core.h"
#include "P_super.h"
#include "P_devtyp.h"
#include "build_defs.h"

class Constraints;
class D_graph;
class HardwareIterator;
class OrchBase;
class P_device;
class P_task;

//==============================================================================

class Placement
{
public:
                   Placement(OrchBase *);
virtual ~          Placement();

void               DoLink();
void               Dump(FILE * = stdout);
void               Init();
bool               Place(P_task *);
void               Xlink(P_device *,P_thread *);

HardwareIterator*  iterator;
OrchBase *         par;
Constraints *      pCon;
D_graph *          pD_graph;

};

//==============================================================================

#endif

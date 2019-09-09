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
#include "DevT_t.h"
#include "build_defs.h"

class Constraints;
class GraphI_t;
class HardwareIterator;
class OrchBase;
class DevI_t;
class Apps_t;

//==============================================================================

class Placement
{
public:
                   Placement(OrchBase *);
virtual ~          Placement();

void               DoLink();
void               Dump(FILE * = stdout);
void               Init();
bool               Place(Apps_t *);
void               Xlink(DevI_t *,P_thread *);

HardwareIterator*  iterator;
OrchBase *         par;
Constraints *      pCon;
GraphI_t *         pD_graph;

};

//==============================================================================

#endif

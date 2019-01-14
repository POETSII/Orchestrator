#ifndef __PlacementH__H
#define __PlacementH__H

#include <stdio.h>
#include "HardwareAddress.h"
#include "pdigraph.hpp"
class PoetsBoard;
class PoetsMailbox;
class PoetsCore;
class PoetsThread;
class P_task;
class D_graph;
class Constraints;
class OrchBase;
class P_link;
class P_port;
class P_device;

//==============================================================================

class Placement
{
public:

enum P_stride {thread, core, mailbox, board};

                   Placement(OrchBase *);
virtual ~          Placement();

void               DoLink();
void               Dump(FILE * = stdout);
bool               GetNext(PoetsThread *&, P_stride = thread);
void               Init();
bool               Place(P_task *);
void               Xlink(P_device *,PoetsThread *);

OrchBase *         par;
Constraints *      pCon;
D_graph *          pD_graph;

pdigraph<AddressComponent, PoetsBoard*,
         unsigned int, float,
         unsigned int, unsigned int>::TPn_it boardIterator;
pdigraph<AddressComponent, PoetsMailbox*,
         unsigned int, float,
         unsigned int, unsigned int>::TPn_it mailboxIterator;
map<AddressComponent, PoetsCore*>::iterator coreIterator;
map<AddressComponent, PoetsThread*>::iterator threadIterator;

};

//==============================================================================

#endif

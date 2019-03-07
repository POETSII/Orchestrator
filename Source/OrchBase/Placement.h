#ifndef __PlacementH__H
#define __PlacementH__H

#include <stdio.h>
#include "HardwareAddress.h"
#include "pdigraph.hpp"
class Constraints;
class D_graph;
class OrchBase;
class P_board;
class P_core;
class P_device;
class P_engine;
class P_link;
class P_mailbox;
class P_port;
class P_task;
class P_thread;

//==============================================================================

class Placement
{
public:

enum P_stride {thread, core, mailbox, board};

                   Placement(OrchBase *);
virtual ~          Placement();

void               DoLink();
void               Dump(FILE * = stdout);
bool               GetNext(P_thread *&, P_stride = thread);
void               Init();
bool               Place(P_task *);
void               Xlink(P_device *,P_thread *);

OrchBase *         par;
Constraints *      pCon;
D_graph *          pD_graph;

pdigraph<AddressComponent, P_board*,
         unsigned, P_link*,
         unsigned, P_port*>::TPn_it boardIterator;
pdigraph<AddressComponent, P_mailbox*,
         unsigned, P_link*,
         unsigned, P_port*>::TPn_it mailboxIterator;
map<AddressComponent, P_core*>::iterator coreIterator;
map<AddressComponent, P_thread*>::iterator threadIterator;
};

//==============================================================================

#endif

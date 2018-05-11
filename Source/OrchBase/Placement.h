#ifndef __PlacementH__H
#define __PlacementH__H

#include <stdio.h>
#include "pdigraph.hpp"
class P_graph;
class P_task;
class D_graph;
class Constraints;
class OrchBase;
class P_thread;
class P_device;
class P_core;
class P_board;
class P_box;
class P_link;
class P_port;

//==============================================================================

class Placement
{
public:

enum P_stride {thread, core, board, box};

                   Placement(OrchBase *);
virtual ~          Placement();

void               DoLink();
void               Dump(FILE * = stdout);
bool               GetNext(P_thread *&, P_stride = thread);
void               Init();
bool               Place(P_task *);
void               Xlink(P_device *,P_thread *);

P_graph *          pP_graph;
D_graph *          pD_graph;
Constraints *      pCon;
OrchBase *         par;

pdigraph<unsigned,P_box *,unsigned,P_link *,unsigned,P_port *>::TPn_it Nbo;
P_box *                                                                pPbo;
vector<P_board*>::iterator                                             Nbd;
vector<P_core*>::iterator                                              Nco;
vector<P_thread*>::iterator                                            Nth;

};

//==============================================================================

#endif





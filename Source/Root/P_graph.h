#ifndef __P_graphH__H
#define __P_graphH__H

#include <stdio.h>
#include "pdigraph.hpp"
#include "NameBase.h"
#include "P_Box.h"
#include "P_Link.h"
#include "P_Port.h"
class Config_t;
class OrchBase;
class Cli;
class D_graph;

//==============================================================================

class P_graph : protected NameBase
{
public:
                   P_graph();
virtual ~          P_graph();
void               Cm(Cli *);
void               Dump(FILE * = stdout);

                                       // The hardware graph
pdigraph<unsigned,P_box *,unsigned,P_link *,unsigned,P_port *> G;
OrchBase *         par;                // Object parent
Config_t *         pConfig;            // HW configuration object
D_graph *          pD;                 // Shortcut to the device graph
};

//==============================================================================

#endif





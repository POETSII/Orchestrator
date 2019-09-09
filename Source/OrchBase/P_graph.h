#ifndef __P_graphH__H
#define __P_graphH__H

#include <stdio.h>
#include "pdigraph.hpp"
#include "NameBase.h"
#include "P_box.h"
#include "P_link.h"
#include "P_port.h"
class Config_t;
class OrchBase;
class Cli;
class D_graph;
#include <list>
using namespace std;

//==============================================================================

class P_graph : public NameBase
{
public:
                   P_graph(OrchBase *,string);
virtual ~          P_graph();
void               Clear();
void               Cm(Cli *);
void               Dump(FILE * = stdout);
bool               IsEmpty();
void               Set1();
void               Set2();
                                       // The hardware graph
pdigraph<unsigned,P_box *,unsigned,P_link *,unsigned,P_port *> G;
OrchBase *         par;                // Object parent
list<Config_t *>   pConfigl;           // HW configuration objects

};

//==============================================================================

#endif





#ifndef __P_deviceH__H
#define __P_deviceH__H

#include <stdio.h>
#include "NameBase.h"
#include "pdigraph.hpp"
#include "P_addr.h"
class D_graph;
class P_thread;
class P_devtyp;
class CFrag;

//==============================================================================

class P_device : public NameBase, protected DumpChan
{
public:
                   P_device(D_graph * =0, string = string());
virtual ~          P_device();
void               Dump(FILE * = stdout);
static void        DevDat_cb(P_device * const &);
static void        DevKey_cb(unsigned const &);
vector<string>     NSGetinpn();
vector<string>     NSGetinpt();
vector<string>     NSGetoupn();
vector<string>     NSGetoupt();
void               Par(D_graph * _p);
void               Unlink();

P_addr             addr;
D_graph *          par;
P_thread *         pP_thread;
P_devtyp *         pP_devtyp;
CFrag *            pPropsI;            // Device properties initialiser code
CFrag *            pStateI;            // Device state initialiser code
unsigned int       idx;                // Index in the graph
unsigned           attr;               // The eponymous attribute

};

//==============================================================================

#endif





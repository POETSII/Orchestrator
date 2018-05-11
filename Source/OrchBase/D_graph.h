#ifndef __D_graphH__H
#define __D_graphH__H

#include <stdio.h>
#include "pdigraph.hpp"
#include "NameBase.h"
#include "P_message.h"
#include "P_typdcl.h"
class P_device;
class P_graph;
class P_task;
class P_pin;
class P_devtyp;

//==============================================================================

class D_graph : public NameBase
{
public:
                    D_graph(P_task *,string);
virtual ~           D_graph();

void                Dump(FILE * = stdout);
vector<P_device*>   DevicesOfType(const P_devtyp* d_type);

                                       // The device graph
pdigraph<unsigned,P_device *,unsigned,P_message *,unsigned,P_pin *> G;
P_task *           par;                // Object parent
P_graph *          pP;                 // Shortcut to the hardware graph
P_typdcl *         pD;                 // Shortcut to declare object
CFrag *            pPropsI;            // Graph properties initialiser code

};

//==============================================================================

#endif





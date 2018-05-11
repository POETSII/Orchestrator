#ifndef __P_pinH__H
#define __P_pinH__H

#include <stdio.h>
#include "NameBase.h"
#include "P_pintyp.h"
#include "D_graph.h"
#include "pdigraph.hpp"           

//==============================================================================

class P_pin : public NameBase, protected DumpChan
{
public:
                   P_pin(D_graph *,string);
virtual ~          P_pin();
void               Dump(FILE * = stdout);
static void        PinDat_cb(P_pin * const &);
static void        PinKey_cb(unsigned const &);

D_graph *          par;
P_pintyp *         pP_pintyp;
CFrag *            pPropsI;
CFrag *            pStateI;
unsigned int       idx; // Index of the pin in the instance graph.

};

//==============================================================================

#endif





#ifndef __P_boxH__H
#define __P_boxH__H

#include <stdio.h>
#include "NameBase.h"
#include "P_board.h"
#include "Bin.h"
#include "P_addr.h"
class P_graph;
class Config_t;
#include "pdigraph.hpp"
#include <vector>
using namespace std;

//==============================================================================

class P_box : public NameBase, protected DumpChan
{
public:
                   P_box(P_graph *,string=string());
virtual ~          P_box();

static void        BoxDat_cb(P_box * const &);
static void        BoxKey_cb(unsigned const &);
void               Build(Config_t *);
void               Dump(FILE * = stdout);

P_addr             addr;
vector<P_board *>  P_boardv;
Bin *              pMothBin;
P_graph *          par;
Config_t *         pConfig;         // HW configuration object
bool               vBox;            // not a detected hardware box.

};

//==============================================================================

#endif





#ifndef __EdgeI_tH__H
#define __EdgeI_tH__H

#include <stdio.h>
#include "NameBase.h"
#include "P_addr.h"
#include "DefRef.h"
#include "DumpChan.h"
class Meta_t;
class GraphI_t;
class P_thread;
class CFrag;

//==============================================================================

class EdgeI_t : public NameBase, protected DumpChan, public DefRef
{
public:
                   EdgeI_t(GraphI_t * = 0, string = string());
virtual ~          EdgeI_t();
void               Dump(unsigned = 0,FILE * = stdout);
static void        EdgDat_cb(EdgeI_t * const &);
static void        EdgKey_cb(unsigned const &);
void               Par(GraphI_t * _p) { par = _p; }

P_addr             addr;
GraphI_t *         par;
unsigned           Key;                // Graph key
size_t             Idx;                // Index of this edge in PinI_t->key_v
vector<Meta_t *>   Meta_v;             // MetaData vector

CFrag *            pPropsI;            // Properties
CFrag *            pStateI;            // State
};

//==============================================================================

#endif

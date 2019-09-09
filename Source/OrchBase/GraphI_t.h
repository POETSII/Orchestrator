#ifndef __GraphI_tH__H
#define __GraphI_tH__H

#include <stdio.h>
#include "pdigraph.hpp"
#include "NameBase.h"
#include "DefRef.h"
#include "GraphT_t.h"
class    DevI_t;
class    P_graph;
class    Apps_t;
class    PinI_t;
class    DevT_t;
class    Meta_t;
class    P_link;

//==============================================================================

class GraphI_t : public NameBase, public DefRef
{
public:
                     GraphI_t(Apps_t *,string);
virtual ~            GraphI_t();
void                 Dump(FILE * = stdout);
vector<DevI_t *>     DevicesOfType(const DevT_t * d_type);

                                       // The device graph itself
pdigraph<unsigned,DevI_t *,unsigned,P_link *,unsigned,PinI_t *> G;
Apps_t *             par;              // Object parent
string               tyId;             // Name of type in type tree
string               tyId2;            // Type tree we're going to link to
GraphT_t *           pT;               // Shortcut to type declare object
CFrag *              pPropsI;          // Graph properties initialiser code
map<string,unsigned> Dmap;             // Device name->key map
vector<Meta_t *>     Meta_v;           // MetaData vector
};

//==============================================================================

#endif





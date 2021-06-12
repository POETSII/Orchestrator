#ifndef __GraphI_tH__H
#define __GraphI_tH__H

#include <stdio.h>
#include "pdigraph.hpp"
#include "NameBase.h"
#include "DefRef.h"
class    GraphT_t;
class    DevI_t;
class    P_graph;
class    Apps_t;
class    PinI_t;
class    DevT_t;
class    Meta_t;
class    EdgeI_t;
class    CFrag;
class    P_super;

//==============================================================================

class GraphI_t : public NameBase, public DefRef
{
public:
                     GraphI_t(Apps_t *,string);
virtual ~            GraphI_t();
void                 DevicesOfType(DevT_t *,vector<DevI_t *>&);
unsigned             DevicesByType(map<DevT_t *,unsigned>&);
void                 Dump(unsigned =0,FILE * = stdout);
string               GetCompoundName(bool path = false);
DevI_t *             GetDevice(string &);
bool                 TLink() { return pT!=0; }
unsigned             TypeLink();
void                 UndefDevs();
void                 UnTLink();

Apps_t *             par;              // Object parent
bool                 deployed;         // Have the binaries been deployed?
                                       // The device graph itself
pdigraph<unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *> G;
string               tyId;             // Name of type in type tree
string               tyId2;            // Type tree we're going to link to
GraphT_t *           pT;               // Type tree pointer
CFrag *              pPropsI;          // Graph properties initialiser code
map<string,unsigned> Dmap;             // Device name->key map
vector<Meta_t *>     Meta_v;           // MetaData vector
P_super*             pSupI;            // Supervisor information

bool                 built;            // Flag to indicate compile state
};

//==============================================================================

#endif

#ifndef __DevI_tH__H
#define __DevI_tH__H

#include <stdio.h>
#include "NameBase.h"
#include "pdigraph.hpp"
#include "P_addr.h"
#include "DefRef.h"
class Meta_t;
class GraphI_t;
class P_thread;
class DevT_t;
class CFrag;

//==============================================================================

class DevI_t : public NameBase, protected DumpChan, public DefRef
{
public:
                   DevI_t(GraphI_t * =0, string = string());
virtual ~          DevI_t();
void               Dump(FILE * = stdout);
static void        DevDat_cb(DevI_t * const &);
static void        DevKey_cb(unsigned const &);
//vector<Entity::pin_t>  NSGetinpn();
//vector<Entity::pin_t>  NSGetoupn();
void               Par(GraphI_t * _p);
void               Unlink();

P_addr             addr;
GraphI_t *         par;
P_thread *         pP_thread;
string             tyId;               // Name of type in type tree
DevT_t *           pT;
CFrag *            pPropsI;            // Device properties initialiser code
CFrag *            pStateI;            // Device state initialiser code
unsigned           Kdev;               // Graph key
unsigned int       idx;                // Index in the graph
unsigned           attr;               // The eponymous attribute
vector<Meta_t *>   Meta_v;             // MetaData vector

};

//==============================================================================

#endif





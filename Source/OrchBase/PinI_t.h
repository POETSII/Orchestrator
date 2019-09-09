#ifndef __PinI_tH__H
#define __PinI_tH__H

#include <stdio.h>
#include "NameBase.h"
#include "PinT_t.h"
#include "GraphI_t.h"
#include "pdigraph.hpp"
#include <set>
using namespace std;

//==============================================================================

class PinI_t : public NameBase, protected DumpChan, public DefRef
{
public:
                     PinI_t(GraphI_t *,string);
virtual ~            PinI_t();
void                 AddKey(unsigned);
static void          Delete();
void                 Dump(FILE * = stdout);
static void          PinDat_cb(PinI_t * const &);
static void          PinKey_cb(unsigned const &);

GraphI_t *           par;              // Parent graph instance
string               tyId;             // Type name string
PinT_t *             pT;               // Type cross-link
CFrag *              pPropsI;          // Properties
CFrag *              pStateI;          // State
unsigned int         idx;              // Index of pin in the instance graph
vector<unsigned>     Keyv;             // Vector of graph pin keys for multi-
                                       // edge pins....  which makes deletion
static set<PinI_t *> DelSet;           // fiddly, hence the 'deletion set'

};

//==============================================================================

#endif





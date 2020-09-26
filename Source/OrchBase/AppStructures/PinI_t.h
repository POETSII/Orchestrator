#ifndef __PinI_tH__H
#define __PinI_tH__H

#include <stdio.h>
#include "NameBase.h"
#include "DumpChan.h"
#include "DefRef.h"
class PinT_t;
class GraphI_t;
class CFrag;

//==============================================================================

class PinI_t : public NameBase, protected DumpChan, public DefRef
{
public:
                     PinI_t(GraphI_t *,string);
virtual ~            PinI_t();
void                 AddKey(unsigned);
void                 Dump(unsigned = 0,FILE * = stdout);
static void          PinDat_cb(PinI_t * const &);
static void          PinKey_cb(unsigned const &);

GraphI_t *           par;              // Parent graph instance
PinT_t *             pT;               // Type cross-link
CFrag *              pPropsI;          // Properties
CFrag *              pStateI;          // State
vector<unsigned>     Key_v;            // Vector of graph pin keys for multi-
                                       // edge pins
};

//==============================================================================

#endif

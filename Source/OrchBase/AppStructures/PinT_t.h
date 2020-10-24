#ifndef __P_pintypH__H
#define __P_pintypH__H

#include <stdio.h>
#include "NameBase.h"
#include "DefRef.h"
class DevT_t;
class CFrag;
class MsgT_t;

//==============================================================================

class PinT_t : public NameBase, public DefRef
{
public:
                   PinT_t(DevT_t *,string);
virtual ~          PinT_t();

void               Dump(unsigned = 0,FILE * = stdout);

DevT_t *           par;                // Parent device
CFrag *            pPropsD;            // Pin properties source
CFrag *            pStateD;            // Pin state source
CFrag *            pHandl;             // Pin event handler source
string             tyId;               // Message type name
MsgT_t *           pMsg;               // Pin message type
size_t             Idx;                // Index of edge in DevT_t->PinT{I/O}_v
bool               dir;                // Pin direction. True for Input.
};

//==============================================================================

#endif

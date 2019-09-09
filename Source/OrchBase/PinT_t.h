#ifndef __P_pintypH__H
#define __P_pintypH__H

#include <stdio.h>
#include "NameBase.h"
#include "DevT_t.h"
#include "CFrag.h"
class P_message;

//==============================================================================

class PinT_t : public NameBase, public DefRef
{
public:
                   PinT_t(DevT_t *,string);
virtual ~          PinT_t();

void               Dump(FILE * = stdout);

CFrag *            pPropsD;
//CFrag *            pPropsI;
CFrag *            pStateD;
//CFrag *            pStateI;
CFrag *            pHandl;
string             tyIdM;
MsgT_t *           pMsg;
DevT_t *           par;
//unsigned           PinPropsSize;
//unsigned           PinStateSize;
//unsigned           idx; // index in the parent's P_pintypXv.

};

//==============================================================================

#endif





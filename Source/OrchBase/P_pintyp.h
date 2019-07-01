#ifndef __P_pintypH__H
#define __P_pintypH__H

#include <stdio.h>
#include "NameBase.h"
#include "P_devtyp.h"
#include "CFrag.h"
class P_message;
// class P_datatype;

//==============================================================================

class P_pintyp : public NameBase
{
public:
                   P_pintyp(P_devtyp *,string);
virtual ~          P_pintyp();

void               Dump(FILE * = stdout);


// P_datatype*        pProps; // V3 version
// P_datatype*        pState; // V3 version
CFrag *            pPropsD; // V4 version
CFrag *            pPropsI; // V4 version
CFrag *            pStateD; // V4 version
CFrag *            pStateI; // V4 version
CFrag *            pHandl;
P_message*         pMsg;
P_devtyp *         par;
size_t             PinPropsSize;
size_t             PinStateSize;
unsigned           idx; // index in the parent's P_pintypXv.

};

//==============================================================================

#endif





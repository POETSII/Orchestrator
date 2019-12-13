#ifndef __P_pintypH__H
#define __P_pintypH__H

#include <stdio.h>
#include "NameBase.h"
#include "P_devtyp.h"
#include "CFrag.h"
class P_message;

//==============================================================================

class P_pintyp : public NameBase
{
public:
                   P_pintyp(P_devtyp *,string);
virtual ~          P_pintyp();

void               Dump(FILE * = stdout);

CFrag *            pPropsD;
CFrag *            pPropsI;
CFrag *            pStateD;
CFrag *            pStateI;
CFrag *            pHandl;
P_message*         pMsg;
P_devtyp *         par;
unsigned           idx; // index in the parent's P_pintypXv.

};

//==============================================================================

#endif





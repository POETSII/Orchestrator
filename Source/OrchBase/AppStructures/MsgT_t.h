#ifndef __P_Msg_tH__H
#define __P_Msg_tH__H

#include <stdio.h>
#include "NameBase.h"
#include "DefRef.h"
#include "DumpChan.h"
class GraphT_t;
class CFrag;

//==============================================================================

class MsgT_t : public NameBase, protected DumpChan, public DefRef
{
public:
                    MsgT_t(GraphT_t *,string);
virtual ~           MsgT_t();
static void         MsgDat_cb(MsgT_t * const &);
static void         MsgKey_cb(unsigned const &);
void                Dump(unsigned = 0,FILE * = stdout);

GraphT_t *          par;
CFrag *             pPropsD;
};

//==============================================================================

#endif





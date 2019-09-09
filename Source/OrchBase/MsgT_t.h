#ifndef __P_Msg_tH__H
#define __P_Msg_tH__H

#include <stdio.h>
#include "NameBase.h"
#include "CFrag.h"
#include "DefRef.h"
class GraphT_t;
#include "pdigraph.hpp"

//==============================================================================

class MsgT_t : public NameBase, protected DumpChan, public DefRef
{
public:
                    MsgT_t(GraphT_t *,string);
virtual ~           MsgT_t();
static void         MsgDat_cb(MsgT_t * const &);
static void         MsgKey_cb(unsigned const &);
void                Dump(FILE * = stdout);

GraphT_t *          par;
CFrag *             pPropsD;
//size_t              MsgSize;
//unsigned            MsgType; // this is the same as the index in P_messagev.

};

//==============================================================================

#endif





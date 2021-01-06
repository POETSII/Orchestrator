#ifndef __P_messageH__H
#define __P_messageH__H

#include <stdio.h>
#include "NameBase.h"
#include "CFrag.h"
class P_typdcl;
#include "pdigraph.hpp"

//==============================================================================

class P_message : public NameBase, protected DumpChan
{
public:
                    P_message(P_typdcl *,string);
virtual ~           P_message();
static void         MsgDat_cb(P_message * const &);
static void         MsgKey_cb(unsigned const &);
void                Dump(FILE * = stdout);

P_typdcl *          par;
CFrag *             pPropsD;
size_t              MsgSize;
unsigned            MsgType; // this is the same as the index in P_messagev.

};

//==============================================================================

#endif

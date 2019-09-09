#ifndef __P_GraphT_tH__H
#define __P_GraphT_tH__H

#include <stdio.h>
#include "NameBase.h"
#include "MsgT_t.h"
#include "P_board.h"
#include "Bin.h"
class GraphI_t;
class DevT_t;
class SupT_t;
class Apps_t;
class PinT_t;
class Meta_t;

#include <vector>
#include <list>
using namespace std;

//==============================================================================

class GraphT_t : public NameBase, public DefRef
{
public:
                    GraphT_t(Apps_t *,string);
virtual ~           GraphT_t();

void                Dump(FILE * = stdout);
DevT_t *            FindDev(string);
MsgT_t *            FindMsg(string);
PinT_t *            FindPin(DevT_t *,string);

Apps_t *            par;
vector<GraphI_t *>  GraphI_v;
vector<DevT_t *>    DevT_v;
vector<MsgT_t *>    MsgT_v;
vector<CFrag *>     ShCd_v;
CFrag *             pPropsD;
bool                TyFlag;
SupT_t *            pSup;
vector<Meta_t *>    Meta_v;           // MetaData vector

};

//==============================================================================

#endif





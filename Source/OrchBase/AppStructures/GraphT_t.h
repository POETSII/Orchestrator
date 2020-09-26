#ifndef __P_GraphT_tH__H
#define __P_GraphT_tH__H

#include <stdio.h>
#include "NameBase.h"
#include "DefRef.h"
class MsgT_t;
class GraphI_t;
class DevT_t;
class SupT_t;
class Apps_t;
class PinT_t;
class Meta_t;
class CFrag;
#include <vector>
using namespace std;

//==============================================================================

class GraphT_t : public NameBase, public DefRef
{
public:
                    GraphT_t(Apps_t *,string);
virtual ~           GraphT_t();


void                Dump(unsigned = 0, FILE * = stdout);
DevT_t *            FindDev(string);
MsgT_t *            FindMsg(string);
PinT_t *            FindPin(DevT_t *,string);
void                MsgDefault();
void                MsgLink();
void                MsgLinkW(string,PinT_t *);

Apps_t *            par;               // Parent application
vector<GraphI_t *>  GraphI_v;          // Typelinked to....
vector<DevT_t *>    DevT_v;            // Contained device types
vector<MsgT_t *>    MsgT_v;            // Contained message types
CFrag *             pShCd;             // Shared code source
CFrag *             pPropsD;           // Properties source
SupT_t *            pSup;              // Supervisor device (type)
vector<Meta_t *>    Meta_v;            // MetaData vector

static const string dMsgName;          // Default message name

};

//==============================================================================

#endif

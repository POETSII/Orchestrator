#ifndef __P_devtypH
#define __P_devtypH

#include <stdio.h>
#include "NameBase.h"
#include "DefRef.h"
class GraphI_t;
class DevT_t;
class GraphT_t;
class PinT_t;
class CFrag;
class Meta_t;
#include <vector>
using namespace std;

//==============================================================================

class DevT_t : public NameBase, public DefRef
{
public:
                     DevT_t(GraphT_t *,string);
                     DevT_t(){}
virtual ~            DevT_t();

void                 Dump(unsigned = 0,FILE * = stdout);
PinT_t *             Loc_pintyp(string,char);

GraphT_t *           par;              // Owning graph
CFrag *              pShCd;            // Shared code
CFrag *              pOnDeId;          // On device idle handler
CFrag *              pOnHWId;          // On hardware idle handler
CFrag *              pOnRTS;           // On RTS handler
CFrag *              pOnInit;          // On init handler
CFrag *              pPropsD;          // Properties code
CFrag *              pStateD;          // State code
char                 devTyp;           // D(evice)/X(ternal)/S(uper)/U(ndef)
PinT_t *             pPinTSI;          // (Single) input pin from supervisor
PinT_t *             pPinTSO;          // (Single) output pin to supervisor
vector<PinT_t *>     PinTI_v;          // Input pin types
vector<PinT_t *>     PinTO_v;          // Output pin types

vector<Meta_t *>     Meta_v;           // MetaData vector
};

//==============================================================================

#endif

#ifndef __P_devtypH
#define __P_devtypH

#include <stdio.h>
#include "NameBase.h"
#include "P_board.h"
#include "Bin.h"
#include "DefRef.h"
class GraphI_t;
class DevT_t;
class GraphT_t;
class PinT_t;
#include "CFrag.h"
#include <vector>
using namespace std;

//==============================================================================

class DevT_t : public NameBase, public DefRef
{
public:
                     DevT_t(GraphT_t *,string);
virtual ~            DevT_t();

void                 Dump(FILE * = stdout);
PinT_t *             Loc_pintyp(string,char);

/* a crude approximation to the executable image size; we only want a rough idea
   of how big a core with such a device would be. If a core completely packed
   with such devices won't fit, we are dead in the water because the smallest
   possible executable will result from a monolithic device type on a given core.
   So we can attempt to build such a core speculatively, and if the resultant
   image is larger than the instruction memory the build fails.
*/
//inline unsigned int NumHandlers() {return 2+pHandlv.size()+P_pintypIv.size()+P_pintypOv.size();};
unsigned             MemPerDevice();

vector<PinT_t *>     PinTI_v;           // Input pin types
vector<PinT_t *>     PinTO_v;           // Output pin types
vector<CFrag *>      ShCd_v;            // Shared code
CFrag *              pOnIdle;           // OnIdle handler
CFrag *              pOnHW;             // On hardware idle handler
CFrag *              pOnRTS;            // On RTS handler
CFrag *              pOnCtl;            // OnControl handler
CFrag *              pPropsD;           // Properties code
CFrag *              pStateD;           // State code
char                 devTyp;            // D(evice)/X(ternal)/S(uper)/U(ndef)
GraphT_t *           par;               // Owning graph
//unsigned            idx; // this device type's index in its parent's P_devtypv
vector<Meta_t *>     Meta_v;           // MetaData vector
};

//==============================================================================

#endif





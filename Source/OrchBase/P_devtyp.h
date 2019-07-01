#ifndef __P_devtypH
#define __P_devtypH

#include <stdio.h>
#include "NameBase.h"
#include "P_board.h"
#include "Bin.h"
class D_graph;
class P_devtyp;
class P_typdcl;
class P_pintyp;
// class P_datatype;
#include "CFrag.h"
#include <vector>
using namespace std;

//==============================================================================

class P_devtyp : public NameBase
{
public:
                    P_devtyp(P_typdcl *,string);
virtual ~           P_devtyp();

void                Dump(FILE * = stdout);
/* a crude approximation to the executable image size; we only want a rough idea
   of how big a core with such a device would be. If a core completely packed
   with such devices won't fit, we are dead in the water because the smallest
   possible executable will result from a monolithic device type on a given core.
   So we can attempt to build such a core speculatively, and if the resultant 
   image is larger than the instruction memory the build fails.
*/ 
inline unsigned int NumHandlers() {return 2+pHandlv.size()+P_pintypIm.size()+P_pintypOm.size();};
unsigned int        MemPerDevice();


map<string, P_pintyp*> P_pintypIm;  // replaced pin type vectors with maps  
 map<string, P_pintyp*> P_pintypOm; // for straightforward lookup during parse 
// vector<P_pintyp *>  P_pintypIv; 
// vector<P_pintyp *>  P_pintypOv;
vector<CFrag *>     pHandlv;
CFrag *             pOnIdle;
CFrag *             pOnRTS;
CFrag *             pOnCtl;
// P_datatype *        pProps;      // Edge properties (V3)
// P_datatype *        pState;      // Edge state (V3)
CFrag *             pPropsI;        // Properties initialiser (V3)
CFrag *             pStateI;        // State initialiser (V4)
CFrag *             pPropsD;        // Edge properties (V4)
CFrag *             pStateD;        // Edge state (V4)
P_typdcl *          par;
unsigned            idx; // this device type's index in its parent's P_devtypv
bool                isExt;
bool                isSuper;


};

//==============================================================================

#endif





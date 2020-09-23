#ifndef __P_suptypH
#define __P_suptypH

#include <stdio.h>
#include "DevT_t.h"
class GraphT_t;
class CFrag;
#include <string>
using namespace std;

//==============================================================================

class SupT_t : public DevT_t
{
public:
                    SupT_t(GraphT_t *,string);
                    SupT_t(DevT_t *);
virtual ~           SupT_t();

void                Dump(unsigned = 0,FILE * = stdout);

CFrag *             pOnPkt;            // On packet from POETS net
CFrag *             pOnRTCL;           // On RTCL message (MPI)
CFrag *             pOnStop;           // On stop (MPI)
CFrag *             pOnCTL;            // On control from console
};

//==============================================================================

#endif





#ifndef __P_suptypH
#define __P_suptypH

#include <stdio.h>
#include "DevT_t.h";
class GraphT_t;
#include "CFrag.h"
#include <string>
using namespace std;

//==============================================================================

class SupT_t : public DevT_t
{
public:
                    SupT_t(GraphT_t *,string);
virtual ~           SupT_t();

void                Dump(FILE * = stdout);
CFrag *             pOnPkt;            // On packet from POETS net
CFrag *             pOnRTCL;           // On RTCL message (MPI)
CFrag *             pOnStop;           // On stop (MPI)
};

//==============================================================================

#endif





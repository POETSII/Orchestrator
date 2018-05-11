#ifndef __RTCHL__H
#define __RTCHL__H

#include "CommonBase.h"
#include "PMsg_p.hpp"

//==============================================================================

class RTCL : public CommonBase
{

public:
                    RTCL(int,char **,string);
virtual ~           RTCL();

typedef unsigned    (RTCL::*pMeth)(PMsg_p *,unsigned); 
typedef map<unsigned,pMeth> FnMap_t;
 
private:

#include            "Decode.cpp"
void                Dump(FILE * = stdout);
unsigned            OnExit(PMsg_p *,unsigned);
unsigned            OnRTCL(PMsg_p *,unsigned);

vector<FnMap_t*>       FnMapx;

public:
struct comms_t {
  RTCL * pthis;
  double tick;
  bool   l_stop;
  double t_stop;
  bool   l_kill;
  double d_stop;
  double t_start;
} comms;

};

//==============================================================================

#endif





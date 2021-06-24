#ifndef __RTCHL__H
#define __RTCHL__H

#include "CommonBase.h"
#include "PMsg_p.hpp"
#include "OSFixes.hpp"

//==============================================================================

class RTCL : public CommonBase
{

public:
                    RTCL(int,char **,string);
virtual ~           RTCL();

typedef unsigned    (RTCL::*pMeth)(PMsg_p *);
map<unsigned,pMeth> FnMap;

private:
#include            "Decode.cpp"
void                Dump(unsigned = 0,FILE * = stdout);
unsigned            OnExit(PMsg_p *);
unsigned            OnRTCL(PMsg_p *);

bool                HaveIdleWork();

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





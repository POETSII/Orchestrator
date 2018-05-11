#ifndef __MonitorH__H
#define __MonitorH__H

#include "CommonBase.h"

//==============================================================================

class Monitor : public CommonBase
{

public:
                    Monitor(int,char **,string);
virtual ~           Monitor();

private:
#include            "Decode.cpp"
void                Dump(FILE * = stdout);
unsigned            Onxxxx(PMsg_p *);

typedef unsigned    (Monitor::*pMeth)(PMsg_p *);
map<unsigned,pMeth> FnMapx;

};

//==============================================================================

#endif


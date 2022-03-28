#ifndef __MonitorH__H
#define __MonitorH__H

#include "CommonBase.h"

//==============================================================================

class Monitor : public CommonBase
{
public:
                    Monitor(int,char **,string);
virtual ~           Monitor();

typedef unsigned    (Monitor::*pMeth)(PMsg_p *);
map<unsigned,pMeth> FnMap;

private:
#include            "Decode.cpp"
void                Dump(unsigned = 0,FILE * = stdout);
unsigned            Onxxxx(PMsg_p *);

};

//==============================================================================

#endif


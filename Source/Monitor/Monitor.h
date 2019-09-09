#ifndef __MonitorH__H
#define __MonitorH__H

#include "CommonBase.h"

//==============================================================================

class Monitor : public CommonBase
{

public:
                    Monitor(int,char **,string);
virtual ~           Monitor();

typedef unsigned    (Monitor::*pMeth)(PMsg_p *,unsigned);
typedef map<unsigned,pMeth> FnMap_t;

private:
#include            "Decode.cpp"
void                Dump(FILE * = stdout);
unsigned            Onxxxx(PMsg_p *,unsigned);

//typedef unsigned    (Monitor::*pMeth)(PMsg_p *);
//map<unsigned,pMeth> FnMapx;

vector<FnMap_t *>   FnMapx;

};

//==============================================================================

#endif


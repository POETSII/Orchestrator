#ifndef __InjectorH__H
#define __InjectorH__H

#include "CommonBase.h"

//==============================================================================

class Injector : public CommonBase
{

public:
                    Injector(int,char **,string);
virtual ~           Injector();

typedef unsigned    (Injector::*pMeth)(PMsg_p *,unsigned);
typedef map<unsigned,Injector::pMeth> FnMap_t;

private:
unsigned            Connect(string);
#include            "Decode.cpp"
void                Dump(FILE * = stdout);
unsigned            OnInjectAck (PMsg_p *,unsigned);
unsigned            OnInjectFlag(PMsg_p *,unsigned);

vector<FnMap_t*>    FnMapx;

public:
static struct injData_t {
  unsigned          flag;
} injData;

};

//==============================================================================

#endif


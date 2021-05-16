#ifndef __InjectorH__H
#define __InjectorH__H

#include "CommonBase.h"

//==============================================================================

class Injector : public CommonBase
{

public:
                    Injector(int,char **,string);

typedef unsigned    (Injector::*pMeth)(PMsg_p*);
map<unsigned,Injector::pMeth> FnMap;

private:
#include            "Decode.cpp"
void                Dump(FILE * = stdout);
unsigned            OnInjectAck (PMsg_p*);
unsigned            OnInjectFlag(PMsg_p*);

public:
static struct injData_t {
  unsigned          flag;
} injData;

};

//==============================================================================

#endif


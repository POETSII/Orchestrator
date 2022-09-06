#ifndef __DummyH__H
#define __DummyH__H

#include "CommonBase.h"
#include "OSFixes.hpp"

//==============================================================================

class Dummy : public CommonBase
{

public:
                    Dummy(int,char **,string);

typedef unsigned    (Dummy::*pMeth)(PMsg_p *);
map<unsigned,pMeth> FnMap;

private:
#include            "Decode.cpp"
void                Dump(unsigned = 0,FILE * = stdout);
void                Init(int, char**);
unsigned            OnMoniDeviReq(PMsg_p *);
};

//==============================================================================
   
#endif

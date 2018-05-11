#ifndef __DummyH__H
#define __DummyH__H

#include "CommonBase.h"

//==============================================================================

class Dummy : public CommonBase
{

public:
                    Dummy(int,char **,string);
virtual ~           Dummy();

typedef unsigned    (Dummy::*pMeth)(Msg_p *,unsigned);
typedef map<unsigned,pMeth> FnMap_t;

private:
#include            "Decode.cpp"
void                Dump(FILE * = stdout);
void                Init(int, char**);

vector<FnMap_t*>    FnMapx;


};

//==============================================================================
   
#endif

#ifndef __MonServerH__H
#define __MonServerH__H

#include "CommonBase.h"
#include "Pserver_t.h"
#include "OSFixes.hpp"

//==============================================================================

class MonServer : public CommonBase
{

public:
                    MonServer(int,char **,string);
virtual ~           MonServer();

typedef unsigned    (MonServer::*pMeth)(PMsg_p *);
map<unsigned,pMeth> FnMap;

Pserver_t           Server;
private:
#include            "Decode.cpp"
void                Dump(unsigned = 0,FILE * = stdout);
unsigned            OnMoniDeviReq (PMsg_p *);
unsigned            OnMoniDeviAck (PMsg_p *);
unsigned            OnMoniInjeReq (PMsg_p *);
unsigned            OnMoniInjeAck (PMsg_p *);
unsigned            OnMoniMothData(PMsg_p *);
unsigned            OnMoniSoftData(PMsg_p *);

};

//==============================================================================

#endif

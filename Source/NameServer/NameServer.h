#ifndef __NameServerH__H
#define __NameServerH__H

#include "CommonBase.h"
#include "Ns_el.h"

//==============================================================================

class NameServer : public CommonBase
{

public:
                    NameServer(int,char **,string);
virtual ~           NameServer();

private:
#include            "Decode.cpp"
void                Dump(unsigned = 0,FILE * = stdout);
void                Load1(Ns_0 *);
unsigned            OnClr  (PMsg_p *);
unsigned            OnDump (PMsg_p *);
unsigned            OnFwd  (PMsg_p *);
unsigned            OnLoad (PMsg_p *);
unsigned            OnLog  (PMsg_p *);
unsigned            OnMoni (PMsg_p *);
unsigned            OnQuery(PMsg_p *);

typedef unsigned    (NameServer::*pMeth)(PMsg_p *);
map<unsigned,pMeth> FnMap;

};

//==============================================================================

#endif


#ifndef __NameServerH__H
#define __NameServerH__H

#include "SBase.h"
// #include "Ns_el.h"

//==============================================================================

class NameServer : public SBase
{

public:
                    NameServer(int,char **,string);
virtual ~           NameServer();

        unsigned    OnCfg            (PMsg_p *, unsigned);
	unsigned    OnDump           (PMsg_p *, unsigned);

private:
#include            "SDecode.cpp"
        void        Dump(FILE * = stdout, string s = "");

        unsigned    ConfigDir        (PMsg_p *, unsigned);
        unsigned    ConfigDistribute (PMsg_p *, unsigned);
        unsigned    ConfigRecall     (PMsg_p *, unsigned);
	unsigned    ConfigState      (PMsg_p *, unsigned);
	unsigned    DumpAll          (PMsg_p *);
        unsigned    DumpSummary      (PMsg_p *);
        unsigned    DumpTask         (PMsg_p *);

typedef unsigned    (NameServer::*pMeth)(PMsg_p *, unsigned);
typedef map<unsigned,pMeth> FnMap_t;
vector<FnMap_t*> FnMapx;

};

//==============================================================================

#endif


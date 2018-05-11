#ifndef __RootH__H
#define __RootH__H

#include "CommonBase.h"
#include "OrchBase.h"
#include "PMsg_p.hpp"
#include "Cli.h"

//==============================================================================

class Root : public OrchBase
{

public:
                    Root(int,char **,string);
virtual ~           Root();

private:
unsigned            CmDrop(Cli *);
unsigned            CmExit(Cli *);
unsigned            CmRTCL(Cli *);
unsigned            CmSyst(Cli *);
bool                CmSystPing(Cli::Cl_t *);
bool                CmSystShow(Cli::Cl_t *);
bool                CmSystTime(Cli::Cl_t *);
unsigned            CmTest(Cli *);
unsigned            CmTopo(Cli *);
#include            "Decode.cpp"
virtual string      Dname(){ return typeid(*this).name(); }
void                Dump(FILE * = stdout);
unsigned            OnKeyb(PMsg_p *);
unsigned            OnLogP(PMsg_p *);
unsigned            OnTest(PMsg_p *);
unsigned            OnTestFloo(PMsg_p *);
unsigned            ProcCmnd(Cli *);
public:
static void         Prompt(FILE * = stdout);

typedef unsigned    (Root::*pMeth)(PMsg_p *);
map<unsigned,pMeth> FnMapx;

public:
static const char * prompt;

};

//==============================================================================

#endif





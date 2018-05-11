#ifndef __RootH__H
#define __RootH__H

#include "CommonBase.h"
#include "OrchBase.h"
#include "PMsg_p.hpp"
#include "Cli.h"
#include "Injector.h"
#include <string>
using namespace std;

//==============================================================================

class Root : public OrchBase
{

public:
                    Root(int,char **,string);
virtual ~           Root();

typedef unsigned      (Root::*pMeth)(PMsg_p *, unsigned);
typedef map<unsigned,pMeth> FnMap_t;

void                  CallEcho(Cli::Cl_t);
void                  CallFile(Cli::Cl_t);
void                  CallShow(Cli::Cl_t);
unsigned              CmCall(Cli *);
unsigned              CmDrop(Cli *);
unsigned              CmExit(Cli *);
unsigned              CmInje(Cli *);
unsigned              CmRTCL(Cli *);
unsigned              CmSyst(Cli *);
unsigned              CmTest(Cli *);
unsigned              Connect(bool,string);
#include              "Decode.cpp"
virtual string        Dname(){ return typeid(*this).name(); }
void                  Dump(FILE * = stdout);
void                  OnIdle();
unsigned              OnInje(PMsg_p *,unsigned);
unsigned              OnKeyb(PMsg_p *,unsigned);
unsigned              OnLogP(PMsg_p *,unsigned);
unsigned              OnTest(PMsg_p *,unsigned);
unsigned              ProcCmnd(Cli *);
static void           Prompt(FILE * = stdout);
void                  SystConn(Cli::Cl_t);
void                  SystPath(Cli::Cl_t);
void                  SystPing(Cli::Cl_t);
void                  SystRun(Cli::Cl_t);
void                  SystShow(Cli::Cl_t);
void                  SystTime(Cli::Cl_t);

public:
vector<FnMap_t*>      FnMapx;
bool                  echo;
vector <string>       stack;
list<Cli>             Equeue;
static const char *   prompt;
struct injData_t {
  unsigned            flag;
} injData;

};

//==============================================================================

#endif





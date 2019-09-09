#ifndef __RootH__H
#define __RootH__H

//==============================================================================
/* Way in from the console. There is a spinning thread, (kb_func), reading from
the console, that wraps each command into a message and sends it to its own
process, where it is picked up by the MPIspinner loop, and handed onto the Onxxx
routines (this is so the keyboard and the other processes get a fair go).
OnKeyb (the method that handles monkey input) passes the command to a splitter
method (ProcCmnd). This in turn passes the command to the relevant clause
splitter. For the simple ones, they are handled by this source file (Cm****
methods). The more complex ones go to dedicated classes (Cm****_t)
*/
//==============================================================================

#include "OrchBase.h"
#include "PMsg_p.hpp"
#include "Cli.h"
#include "Injector.h"
#include "CmCall_t.h"
#include "CmGrph_t.h"
#include "CmRTCL_t.h"
#include "CmTest_t.h"
#include "CmSyst_t.h"
#include <string>
using namespace std;

//==============================================================================

class Root : public OrchBase
{

public:
                            Root(int,char **,string);
virtual ~                   Root();

typedef unsigned            (Root::*pMeth)(PMsg_p *, unsigned);
typedef map<unsigned,pMeth> FnMap_t;

unsigned                    CmDrop(Cli *);
unsigned                    CmExit(Cli *);
unsigned                    CmInje(Cli *);
unsigned                    Connect(string);
#include                    "Decode.cpp"
void                        Dump(FILE * = stdout);
void                        OnIdle();
unsigned                    OnInje(PMsg_p *,unsigned);
unsigned                    OnKeyb(PMsg_p *,unsigned);
unsigned                    OnLogP(PMsg_p *,unsigned);
unsigned                    OnTest(PMsg_p *,unsigned);
unsigned                    ProcCmnd(Cli *);
static void                 Prompt(FILE * = stdout);

public:
vector<FnMap_t*>            FnMapx;
static const char *         prompt;
struct injData_t {
  unsigned                  flag;
} injData;

CmCall_t *                  CmCall;
CmRTCL_t *                  CmRTCL;
CmSyst_t *                  CmSyst;
CmTest_t *                  CmTest;

};

//==============================================================================

#endif





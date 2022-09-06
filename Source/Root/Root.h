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
#include "OrchConfig.h"
#include "PMsg_p.hpp"
#include "RootArgs.h"
#include "Cli.h"
#include "Injector.h"
#include <string>
#include "OSFixes.hpp"
#include "pthread.h"
using namespace std;

//==============================================================================

class Root : public OrchBase
{

public:
                            Root(int,char **,string);
virtual ~                   Root();

typedef unsigned            (Root::*pMeth)(PMsg_p *);
map<unsigned,pMeth>         FnMap;

unsigned                    CmDrop(Cli *);
unsigned                    CmExit(Cli *);
unsigned                    CmInje(Cli *);
unsigned                    CmRetu(Cli *);
bool                        Config();
#include                    "Decode.cpp"
void                        Dump(unsigned = 0,FILE * = stdout);
void                        OnIdle();
unsigned                    OnInje(PMsg_p *);
unsigned                    OnKeyb(PMsg_p *);
unsigned                    OnLogP(PMsg_p *);
unsigned                    OnMoniInjeReq(PMsg_p *);
unsigned                    OnMoniDeviReq(PMsg_p *);
unsigned                    OnMshipAck(PMsg_p *);
unsigned                    OnMshipReq(PMsg_p *);
unsigned                    OnPmap(PMsg_p *);
unsigned                    OnTest(PMsg_p *);
unsigned                    ProcCmnd(Cli *);
static void                 Prompt(FILE * = stdout);
void                        WriteUheader(Cli *);

public:
static const char *            prompt;
static bool                    promptOn;
struct injData_t {
  unsigned flag;
}                              injData;

OrchConfig *                   pOC;

pthread_t                      kb_thread;
bool                           exitOnEmpty;   // exit /at = end
bool                           exitOnStop;    // exit /at = stop
bool                           appJustStopped;
};

//==============================================================================

#endif

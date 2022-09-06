#ifndef __CommonBaseH__H
#define __CommonBaseH__H

#include <stdio.h>
#include <string>
#include <cstring>
using namespace std;
#include "PMsg_p.hpp"
#include "ProcMap.h"
#include "flat.h"
#include "mpi.h"
#include "OSFixes.hpp"

#ifndef SNDBUFSIZ
#define SNDBUFSIZ 1000000000  // MPI immediate message send buffer size
#endif

//==============================================================================

class CommonBase
{
public:                CommonBase(){}
                       CommonBase(int,char **,string,string);
virtual ~              CommonBase();

typedef unsigned       (CommonBase::*pMeth)(PMsg_p *);
map<unsigned,pMeth>    FnMap;

protected:
virtual unsigned       Decode(PMsg_p *) = 0;
void                   Dump(unsigned = 0,FILE * = stdout);
unsigned               OnExit(PMsg_p *);
virtual void           OnIdle();
unsigned               OnPmap(PMsg_p *);
unsigned               OnSystPingAck(PMsg_p *);
unsigned               OnSystPingReq(PMsg_p *);
unsigned               OnSystRun(PMsg_p *);

public:
unsigned               OnTestFloo(PMsg_p *);
bool                   Post(int,string=S00,string=S00,string=S00,string=S00,
                                string=S00,string=S00,string=S00);
bool                   Post(int,vector<string> &);

protected:
void                   Prologue(int,char **);
void                   SendPMap(PMsg_p *);
void                   MPISpinner();

public:
string                 Sderived;
static const string    S00;
int                    Urank;
int                    Usize;
string                 Sproc;
string                 Suser;
unsigned               UBPW;
string                 Scompiler;
string                 SOS;
string                 Ssource;
string                 Sbinary;
string                 STIME;
string                 SDATE;
ProcMap *              pPmap;
int                    MPI_provided;

};

//==============================================================================

#endif

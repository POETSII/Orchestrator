#ifndef __CommonBaseH__H
#define __CommonBaseH__H

#include <stdio.h>
#include <string>
#include <cstring>
using namespace std;
#include "PMsg_p.hpp"
#include "ProcMap.h"
#include "pthread.h"
#include "flat.h"
#include "mpi.h"

//==============================================================================

class CommonBase
{
public:                CommonBase(){}
                       CommonBase(int,char **,string,string);
virtual ~              CommonBase();

typedef unsigned       (CommonBase::*pMeth)(PMsg_p *,unsigned);
typedef map<unsigned,pMeth> FnMap_t;
static void*           Accept(void*);

protected:
virtual unsigned       Connect(string="");
virtual unsigned       Decode(PMsg_p *,unsigned) = 0;
void                   Dump(FILE * = stdout);
int                    LogSCIdx();
int                    NameSCIdx();
unsigned               OnExit(PMsg_p *,unsigned);
virtual void           OnIdle();
unsigned               OnPmap(PMsg_p *,unsigned);
unsigned               OnSystAcpt(PMsg_p *,unsigned);
unsigned               OnSystConn(PMsg_p *,unsigned);
unsigned               OnSystPingAck(PMsg_p *,unsigned);
unsigned               OnSystPingReq(PMsg_p *,unsigned);
unsigned               OnSystRun(PMsg_p *, unsigned);
unsigned               OnTestFloo(PMsg_p *,unsigned);

public:
bool                   Post(int,string=S00,string=S00,string=S00,string=S00,
                                string=S00,string=S00,string=S00);
bool                   Post(int,vector<string> &);

protected:
void                   Prologue(int,char **);
//virtual inline void    RegFn(unsigned comIdx,unsigned fnIdx,pMeth fn){(*FnMapx[comIdx])[fnIdx]=fn;};
void                   SendPMap(MPI_Comm,PMsg_p*);
int                    RootCIdx();
void                   MPISpinner();

const int              MPICli = 0;

public:
vector<FnMap_t*>       FnMapx;
vector<MPI_Comm>       Comms;    // this could be a map indexed by service name
string                 Sderived;
static const string    S00;
int                    Urank;
vector<int>            Usize;
int                    Ulen;
string                 Sproc;
string                 Suser;
unsigned               UBPW;
string                 Scompiler;
string                 SOS;
string                 Ssource;
string                 Sbinary;
string                 STIME;
string                 SDATE;
vector<ProcMap *>      pPmap;   // if Comms is a map, this must be too
int                    MPI_provided;

pthread_t              MPI_accept;
static const char*     MPISvc;
bool                   AcceptConns; // allow external universes to connect.
int                    Lrank;       // rank of the local leader for accept/connect.
char                   MPIPort[MPI_MAX_PORT_NAME];     // port name
MPI_Comm               Tcomm;       // New comm set up by an MPI_Comm_accept

private:
char *                 MPI_Buf;
int                    Msgbufsz;
const int              LOG_MSGBUF_BLK_SZ = 14;
const int              MSGBUF_BLK_MASK = (1<<LOG_MSGBUF_BLK_SZ)-1;

};

//==============================================================================

#endif

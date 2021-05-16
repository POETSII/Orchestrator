//------------------------------------------------------------------------------

#include "NameServer.h"
#include "CommonBase.h"
#include "PMsg_p.hpp"
#include "mpi.h"
#include "Pglobals.h"
#include "Ns_el.h"
#include "jnj.h"
#include <stdio.h>

//==============================================================================

NameServer::NameServer(int argc,char * argv[],string d):
  CommonBase(argc,argv,d,string(__FILE__))
{
                                       // Load the message map
FnMap[PMsg_p::KEY(Q::CANDC,Q::CLEAR)] = &NameServer::OnClr;
FnMap[PMsg_p::KEY(Q::CANDC,Q::DUMP )] = &NameServer::OnDump;
FnMap[PMsg_p::KEY(Q::APP,  Q::FWD  )] = &NameServer::OnFwd;
FnMap[PMsg_p::KEY(Q::CANDC,Q::LOAD )] = &NameServer::OnLoad;
FnMap[PMsg_p::KEY(Q::CANDC,Q::LOG  )] = &NameServer::OnLog;
FnMap[PMsg_p::KEY(Q::CANDC,Q::MONI )] = &NameServer::OnMoni;
FnMap[PMsg_p::KEY(Q::QUERY,Q::N000 )] = &NameServer::OnQuery;

MPISpinner();                          // Spin on MPI messages; exit only on DIE

//printf("********* NameServer rank %d on the way out\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

NameServer::~NameServer()
{
//printf("********* NameServer rank %d destructor\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

void NameServer::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sNameServer dump ++++++++++++++++++++++++++++++++++++++++++\n",os);
CommonBase::Dump(off+2,fp);
fprintf(fp,"%sNameServer dump ------------------------------------------\n",os);
}

//------------------------------------------------------------------------------

void NameServer::Load1(Ns_0 * p0)
{
static unsigned cnt=0;
printf("\n********** %u\n\n",cnt++);
p0->Dump();
}

//------------------------------------------------------------------------------

unsigned NameServer::OnClr(PMsg_p *)
{
return 0;
}

//------------------------------------------------------------------------------

unsigned NameServer::OnDump(PMsg_p *)
{
return 0;
}

//------------------------------------------------------------------------------

unsigned NameServer::OnFwd(PMsg_p *)
{
return 0;
}

//------------------------------------------------------------------------------

unsigned NameServer::OnLoad(PMsg_p * pZ)
// Load some fraction of the NameServer datastructure with the entities supplied
// in the message.
{
Ns_el * pN = new Ns_el(pZ);            // Unwrap the message
int cnt=0;
unsigned * ploc = pN->Get<unsigned>(0,cnt);// Get hold of the key vector
if (ploc==0) Post(909);
vector<Ns_0 *> * pV = pN->Construct(); // Expose the entities
WALKVECTOR(Ns_0 *,(*pV),i) Load1(*i);  // Add to database
delete pN;
return 0;
}

//------------------------------------------------------------------------------

unsigned NameServer::OnLog(PMsg_p *)
{
return 0;
}

//------------------------------------------------------------------------------

unsigned NameServer::OnMoni(PMsg_p *)
{
return 0;
}

//------------------------------------------------------------------------------

unsigned NameServer::OnQuery(PMsg_p *)
{
return 0;
}

//==============================================================================


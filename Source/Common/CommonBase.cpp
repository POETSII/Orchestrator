//------------------------------------------------------------------------------

#include "CommonBase.h"
#include "Environment.h"
#include "PMsg_p.hpp"
#include "Pglobals.h"
#include <stdio.h>
#include "Unrec_t.h"
#include <algorithm>
#include <string>
#include <stdint.h>

#include "OSFixes.hpp"

#include <string>
using namespace std;

//==============================================================================

typedef unsigned char byte;
const string CommonBase::S00 = string();

//==============================================================================

CommonBase::CommonBase(int argc,char ** argv,string d,string src)
{
MPI_Init_thread(&argc,&argv,MPI_THREAD_MULTIPLE,&MPI_provided);
char * SNDBUF = (char *)malloc(SNDBUFSIZ);             // Pull it off the heap
if (SNDBUF!=0) MPI_Buffer_attach(SNDBUF,SNDBUFSIZ);    // Attach it to MPI
pPmap = new ProcMap(this);

Sderived = d;                          // Derived class name
Ssource = src;                         // Derived class source file name
// Load event handler map
FnMap[Msg_p::KEY(Q::EXIT                )] = &CommonBase::OnExit;
FnMap[Msg_p::KEY(Q::PMAP                )] = &CommonBase::OnPmap;
FnMap[Msg_p::KEY(Q::SYST,Q::PING,Q::ACK )] = &CommonBase::OnSystPingAck;
FnMap[Msg_p::KEY(Q::SYST,Q::PING,Q::REQ )] = &CommonBase::OnSystPingReq;
FnMap[Msg_p::KEY(Q::SYST,Q::RUN         )] = &CommonBase::OnSystRun;
FnMap[Msg_p::KEY(Q::TEST,Q::FLOO        )] = &CommonBase::OnTestFloo;
Prologue(argc,argv);                   // Start MPI, build process map.....

}

//------------------------------------------------------------------------------

CommonBase::~CommonBase()
{
//printf("********* CommonBase rank %d destructor\n",Urank); fflush(stdout);
delete pPmap;                          // Lose the process map
int idummy;                            // Clean up MPI immediate message buffer
void * SNDBUF;                         // POETS send buffer
MPI_Buffer_detach(&SNDBUF,&idummy);    // (Blocks until exhausted)
free(SNDBUF);                          // Now we can kill it
MPI_Finalize();
}

//------------------------------------------------------------------------------

void CommonBase::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sCommonBase +++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sSderived (this derived process) : %s\n",os,Sderived.c_str());
fprintf(fp,"%sEvent handler table:\n",os);
fprintf(fp,"%sKey        Method\n",os);
WALKMAP(unsigned,pMeth,FnMap,i)
{
  fprintf(fp,"%s%#010x %" PTR_FMT "\n",os,(*i).first,
                        OSFixes::getAddrAsUint((*i).second));
}

fprintf(fp,"%sS00 (const static)              : %s\n",os,S00.c_str());
fprintf(fp,"%sUrank                           : %d\n",os,Urank);
fprintf(fp,"%sUsize                           : %u\n",os,Urank);
fprintf(fp,"%sSproc                           : %s\n",os,Sproc.c_str());
fprintf(fp,"%sSuser                           : %s\n",os,Suser.c_str());
fprintf(fp,"%sUBPW                            : %u\n",os,UBPW);
fprintf(fp,"%sScompiler                       : %s\n",os,Scompiler.c_str());
fprintf(fp,"%sSOS                             : %s\n",os,SOS.c_str());
fprintf(fp,"%sSsource                         : %s\n",os,Ssource.c_str());
fprintf(fp,"%sSbinary                         : %s\n",os,Sbinary.c_str());
fprintf(fp,"%sSTIME                           : %s\n",os,STIME.c_str());
fprintf(fp,"%sSDATE                           : %s\n",os,SDATE.c_str());
if (pPmap==0) fprintf(fp,"%sNo process map\n",os);
else pPmap->Show(fp);
fprintf(fp,"%sCommonBase ---------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void CommonBase::MPISpinner()
// Spinner to handle ALL incoming MPI packets. There is only one exit from this:
// Decode() returns 1 when an "exit" command is detected.
{

int MSGBUFSIZ = 1024;                  // Individual incoming message buffer
char * MSGBUF = new char[MSGBUFSIZ];   // Pull it off the heap
for (;;) {
// See if there are any MPI packets coming down the pipe
  MPI_Status status;                   // Note the multi-threaded MPI probe
  int flag;
  MPI_Iprobe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&flag,&status);
  if (flag==0) {                       // Nothing there....
    OnIdle();                          // Guess
    continue;                          // And try again
  }
  int count;
  MPI_Get_count(&status,MPI_CHAR,&count);
  if (count > MSGBUFSIZ) {             // Ensure we have the space for it
    delete [] MSGBUF;                  // Throw the too-small one away
    MSGBUF = new char[MSGBUFSIZ=count+1]; // Pull a bigger one off the heap
  }
  MPI_Request request;
  MPI_Irecv(&MSGBUF[0],count,MPI_CHAR,MPI_ANY_SOURCE,MPI_ANY_TAG,
            MPI_COMM_WORLD,&request);
  do {; MPI_Test(&request,&flag,&status); } while (flag==0);
  PMsg_p Pkt((byte *)&MSGBUF[0],count);// Turn it into a packet
  Pkt.Ztime(1,MPI_Wtime());            // Timestamp arrival
  if (Decode(&Pkt)!=0) break;          // Do it, possibly leaving afterwards
}

if (MSGBUF!=0) delete [] MSGBUF;       // Kill the individual message buffer
}

//------------------------------------------------------------------------------

unsigned CommonBase::OnExit(PMsg_p * Z)
// Do not post anything further here - the LogServer may have already gone
{
return 1;                              // Return != 0 means close spinner
}

//------------------------------------------------------------------------------

void CommonBase::OnIdle()
{

}

//------------------------------------------------------------------------------

unsigned CommonBase::OnPmap(PMsg_p * Z)
{
pPmap->Register(Z);
return 0;
}

//------------------------------------------------------------------------------

unsigned CommonBase::OnSystPingAck(PMsg_p * Z)
// Ping acknowledge?
{
int cnt;
double ZT0 = 0.0;                      // MPI times
double * pZT0 = Z->Get<double>(0,cnt); // Unload the REQ launch time
if (pZT0!=0) ZT0 = *pZT0;
double trip = Z->Ztime(1)-ZT0;         // Round trip time
int * pTgtR = Z->Get<int>(0,cnt);      // Unload target rank
int TgtR;                              // Target rank
if (pTgtR!=0) TgtR = * pTgtR;
string TgtS = pPmap->M[TgtR];          // Target name
const unsigned len = 20;               // Process name length
TgtS.resize(len,' ');                  // So they line up on the console
string Fr(Sderived);
Fr.resize(len,' ');
unsigned * pu = Z->Get<unsigned>(4,cnt);// Ping attempt
Post(21,uint2str(*pu),Fr,int2str(Urank),TgtS,int2str(Z->Src()),dbl2str(trip));
return 0;
}

//------------------------------------------------------------------------------

unsigned CommonBase::OnSystPingReq(PMsg_p * Z)
{
unsigned * pu;
int cnt;
double ZT0,ZT1;                        // MPI times
PMsg_p Pkt(*Z);                        // Build response on top of incomer
Pkt.L(2,Q::ACK);                       // Sending it back....
ZT0 = Z->Ztime(0);                     // Copy MPI wallclock times over
ZT1 = Z->Ztime(1);
Pkt.Put<double>(0,&ZT0);
Pkt.Put<double>(1,&ZT1);
pu = Z->Get<unsigned>(4,cnt);          // Ping attempt
if (pu!=0) Pkt.Put<unsigned>(4,pu);
Pkt.Tgt(Z->Src());                     // Back to sender
string tD(GetDate());
string tT(GetTime());
Pkt.Put(4,&tD);                        // Payload
Pkt.Put(5,&tT);
Pkt.Put<int>(0,&Urank);                // My (sending) rank. Why, Lord?
Pkt.Src(Urank);                        // My sending rank.
Pkt.Send();
return 0;
}

//------------------------------------------------------------------------------

unsigned CommonBase::OnSystRun(PMsg_p * Z)
{
return 0;
}

//------------------------------------------------------------------------------

unsigned CommonBase::OnTestFloo(PMsg_p * Z)
// Got a test flood event.
{
int c;
unsigned lev = *(Z->Get<unsigned>(2,c));// Extract remaining level
if (lev==0) {                           // If we're done...
  Post(54,Sproc,uint2str(Urank));
  return 0;
}
--lev;

Z->Put<unsigned>(2,&lev,1);              // No, decrement the level

unsigned * pw = Z->Get<unsigned>(1,c); // Extract burst width
for (unsigned i=0;i<*pw;i++) Z->Bcast(); // Send it on its way
return 0;
}

//------------------------------------------------------------------------------

bool CommonBase::Post(int i,
  string s00,string s01,string s02,string s03,string s04,string s05,string s06)
// Ideally we want a variable length argument list of std::strings, but the
// draft C++ standard section 5.2.2 Function call paragraph 7 clause 9:
// ...if the argument has a non-POD class type, the behavior is undefined...
// Which means we can have a list of strings but we can't tell when the list is
// empty.
// So I thought I'd use char * as the dummy arguments, but the type promotion in
// the compiler happily downconverts string() actuals to char *, but va_arg()
// chokes on it, so no joy there.
// So we do it the clunky way.
// When we did this for BIMPA, Post *belonged* to the Log class, but here it
// just *talks* to it.
{
vector<string> varg;
if (!s00.empty())varg.push_back(s00);
if (!s01.empty())varg.push_back(s01);
if (!s02.empty())varg.push_back(s02);
if (!s03.empty())varg.push_back(s03);
if (!s04.empty())varg.push_back(s04);
if (!s05.empty())varg.push_back(s05);
if (!s06.empty())varg.push_back(s06);
return Post(i,varg);
}

//------------------------------------------------------------------------------

bool CommonBase::Post(int i,vector<string> & varg)
// The other way into Post() - see above
{
if (Urank==Q::NAP)              throw (Unrec_t(1,Sderived,"Post"));
PMsg_p Pkt;
Pkt.PutX(1,&varg);
Pkt.Put<int>(1,&i);
Pkt.Key(Q::LOG,Q::POST);
Pkt.Src(Urank);
Pkt.Send(pPmap->U.LogServer);
// We always return true so we can write things like ....if(Post(...))return;...
return true;
}

//------------------------------------------------------------------------------

void CommonBase::Prologue(int argc,char ** argv)
// Startup code for EVERY orchestrator process.
{
MPI_Comm_size(MPI_COMM_WORLD,&Usize);  // Universe size
MPI_Comm_rank(MPI_COMM_WORLD,&Urank);  // My place within it
char Uname[MPI_MAX_PROCESSOR_NAME];
int Ulen;
MPI_Get_processor_name(Uname,&Ulen);   // Machine name
Sproc = string(Uname);                 // Translated from the original FORTRAN
Suser = GetUser();                     // User name as seen by the process
UBPW = BPW();                          // ... bits per word
Scompiler = GetCompiler();             // ... compiler
SOS = GetOS();                         // ... Operating system
Sbinary = string(argv[0]);             // ... binary file
STIME = string(__TIME__);              // ... compilation time
SDATE = string(__DATE__);              // ... compilation date

PMsg_p Pkt;
SendPMap(&Pkt);                        // Send our data to everybody else
pPmap->Register(&Pkt);                 // Need to put ourselves in seperately
MPI_Barrier(MPI_COMM_WORLD);           // Wait until everyone has told everyone
}

//------------------------------------------------------------------------------

void CommonBase::SendPMap(PMsg_p * Pkt)
{
// Shove the data all into a packet:
Pkt->Put<int>(1,&Urank);               // Load my local rank...
Pkt->Put(2,&Sproc);                    // ... processor name
Pkt->Put(3,&Suser);                    // ... user name
Pkt->Put(4,&Sderived);                 // ... C++ class
Pkt->Put<unsigned>(5,&UBPW);           // ... bits per word
Pkt->Put(6,&Scompiler);                // ... compiler
Pkt->Put(7,&SOS);                      // ... operating system
Pkt->Put(8,&Ssource);                  // ... source file
Pkt->Put(9,&Sbinary);                  // ... binary file
Pkt->Put(10,&STIME);                   // ... compilation time
Pkt->Put(11,&SDATE);                   // ... compilation date
Pkt->Put<int>(12,&MPI_provided);       // ... MPI thread class
Pkt->Key(Q::PMAP);                     // Message key
Pkt->Src(Urank);
                                       // Build process map
Pkt->Bcast();                          // Tell everyone else via a broadcast
}

//==============================================================================

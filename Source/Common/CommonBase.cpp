//------------------------------------------------------------------------------

#include "CommonBase.h"
#include "Environment.h"
//#include "mpi.h"
#include "PMsg_p.hpp"
#include "Pglobals.h"
#include <stdio.h>
#include "Unrec_t.h"
#include <algorithm>

#include <string>
using namespace std;
typedef unsigned char byte;
const string CommonBase::S00 = string();
const char* CommonBase::MPISvc = "POETS_MPI_Master";

//==============================================================================

CommonBase::CommonBase(int argc,char ** argv,string d,string src)
{
//MPI_Init(&argc,&argv);               // Start up MPI
MPI_Init_thread(&argc,&argv,MPI_THREAD_MULTIPLE,&MPI_provided);

AcceptConns = false;
const int SNDBUFSIZ = 1000000000;      // MPI immediate message send buffer size
char * SNDBUF = (char *)malloc(SNDBUFSIZ);             // Pull it off the heap
if (SNDBUF!=0) MPI_Buffer_attach(SNDBUF,SNDBUFSIZ);    // Attach it to MPI
Msgbufsz = 2 << LOG_MSGBUF_BLK_SZ;
MPI_Buf = new char[Msgbufsz];            // and set up a receive buffer
MPI_Comm POETS_COMM_WORLD;
MPI_Comm_dup(MPI_COMM_WORLD, &POETS_COMM_WORLD);
Comms.push_back(POETS_COMM_WORLD);       // Set up the default comm
// Comms.push_back(MPI_COMM_WORLD);         // Set up the default comm
pPmap.push_back(new ProcMap(this));      // Create default universe process map
int lUsize;
MPI_Comm_size(Comms[0], &lUsize);
Usize.push_back(lUsize);                // record the size of the local universe

Sderived = d;                          // Derived class name
Ssource = src;                         // Derived class source file name

FnMapx.push_back(new FnMap_t); // create a default function table (for the *base* class!)
// Load event handler map
// the 'SystConn' and 'SystRun' events, instructing processes to participate
// in setup of a connection to another group of processes, only applies to
// the local communicator. One cannot do this over an intercommunicator, so
// these functions are only registered for the local function table.
(*FnMapx[0])[Msg_p::KEY(Q::EXIT                )] = &CommonBase::OnExit;
(*FnMapx[0])[Msg_p::KEY(Q::PMAP                )] = &CommonBase::OnPmap;
(*FnMapx[0])[Msg_p::KEY(Q::SYST,Q::CONN        )] = &CommonBase::OnSystConn;
(*FnMapx[0])[Msg_p::KEY(Q::SYST,Q::ACPT        )] = &CommonBase::OnSystAcpt;
(*FnMapx[0])[Msg_p::KEY(Q::SYST,Q::PING,Q::ACK )] = &CommonBase::OnSystPingAck;
(*FnMapx[0])[Msg_p::KEY(Q::SYST,Q::PING,Q::REQ )] = &CommonBase::OnSystPingReq;
(*FnMapx[0])[Msg_p::KEY(Q::SYST,Q::RUN         )] = &CommonBase::OnSystRun;
(*FnMapx[0])[Msg_p::KEY(Q::TEST,Q::FLOO        )] = &CommonBase::OnTestFloo;

Prologue(argc,argv);                   // Start MPI, build process map.....

}

//------------------------------------------------------------------------------

CommonBase::~CommonBase()
{
//printf("********* CommonBase rank %d destructor\n",Urank); fflush(stdout);
WALKVECTOR(ProcMap*,pPmap,P)
  delete *P;
int idummy;                            // Clean up MPI immediate message buffer
void *SNDBUF;
WALKVECTOR(FnMap_t*,FnMapx,F)          // WALKVECTOR and WALKMAP are in macros.h (long include chain)
  delete *F;                           // get rid of function tables
// the seemingly daft argument to MPI_Comm_disconnect: &(*C) is correct here because
// MPI_Comm_disconnect can't work with an iterator.
for (vector<MPI_Comm>::iterator C = ++Comms.begin(); C != Comms.end(); C++) MPI_Comm_disconnect(&(*C)); // and comms other than MPI_COMM_WORLD
MPI_Comm_free(&(Comms[0]));            // free the shadow MPI_COMM_WORLD connector
MPI_Buffer_detach(&SNDBUF,&idummy);    // (Blocks until exhausted)
free(SNDBUF);                          // Now we can kill it
delete [] MPI_Buf;                     // as well as the receive buffer
MPI_Finalize();
}

//------------------------------------------------------------------------------

void* CommonBase::Accept(void* par)
/* Blocking routine to connect to another MPI universe by publishing a port.
   This operates in a separate thread to avoid blocking the whole process.
*/
{
CommonBase* parent=static_cast<CommonBase*>(par);
// The leader of the intercomm will be the lowest-ranking Mothership, if there is one,
// or the zero-rank process otherwise.
/*
bool leader = (parent->Urank == parent->Lrank);
if (leader) // The leader needs to set up the comms port.
{
MPI_Open_port(MPI_INFO_NULL,parent->MPIPort); // Announce to MPI that we are open for business
MPI_Publish_name(parent->MPISvc,MPI_INFO_NULL,parent->MPIPort); // and make us publicly available
}
*/
MPI_Group MPI_Group_world, MPI_Group_remote;
MPI_Comm_group(parent->Comms[0],&MPI_Group_world);
// MPI_Comm_group(MPI_COMM_WORLD,&MPI_Group_world);
while (parent->AcceptConns)
{
// run the blocking accept itself.
if (MPI_Comm_accept(parent->MPIPort,MPI_INFO_NULL,parent->Lrank,MPI_COMM_WORLD,&(parent->Tcomm)))
{
   // Might try Post() but failure could indicate an unreliable MPI universe
   // so all bets are off about whether any MPI communication would actually succeed.
   printf("Error: attempt to connect to another MPI universe failed\n");
   parent->AcceptConns = false;
   break;
}
int RemoteSize, RemoteRank; // detect if this is an exit by looking for a self-connection
int RootRank = 0;
MPI_Comm_remote_size(parent->Tcomm,&RemoteSize);
MPI_Comm_remote_group(parent->Tcomm,&MPI_Group_remote);
MPI_Group_translate_ranks(MPI_Group_world,1,&RootRank,MPI_Group_remote,&RemoteRank);
// printf("Connected to remote group of size %d, leader rank %d\n",RemoteSize,RemoteRank==MPI_UNDEFINED? -1 : RemoteRank);
// fflush(stdout);
if ((RemoteSize>1) || (RemoteRank == MPI_UNDEFINED)) // as long as we aren't shutting down,
{
   // trigger the Connect process in the main thread to complete the setup
   PMsg_p Creq;
   string N("");              // zero-length string indicates a server-side connection
   Creq.Put(0,&N);
   Creq.Key(Q::SYST,Q::CONN);
   Creq.Src(parent->Urank);
   Creq.comm = parent->Comms[0];
   // Creq.comm = MPI_COMM_WORLD;
   Creq.Send(parent->Urank);
   // printf("Accept waiting to complete connection\n");
   // fflush(stdout);
   while (parent->Tcomm != MPI_COMM_NULL); // block until connect has succeeded
   // printf("Accept completed connection\n");
   // fflush(stdout);
}
else
{
   printf("Process rank %d received a shutdown connect message\n", parent->Urank);
   fflush(stdout);
   MPI_Comm_disconnect(&(parent->Tcomm)); // for completeness, though we are exiting.
   break;
}
}
/*
if (leader) // close down name/port if we are the local intercomm leader
{
MPI_Unpublish_name(parent->MPISvc,MPI_INFO_NULL,parent->MPIPort);
MPI_Close_port(parent->MPIPort);
}
*/
pthread_exit(par);
return par;
}

//------------------------------------------------------------------------------

void CommonBase::Dump(FILE * fp)
{
fprintf(fp,"CommonBase+++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"Sderived (this derived process) : %s\n",Sderived.c_str());
fprintf(fp,"Event handler table:\n");
unsigned cIdx = 0;
WALKVECTOR(FnMap_t*,FnMapx,F)
{
fprintf(fp,"Function table for comm %d:\n", cIdx++);
fprintf(fp,"Key        Method\n");
WALKMAP(unsigned,pMeth,(**F),i)
  fprintf(fp,"%#010x 0x%#010p\n",(*i).first,(*i).second);
}
fprintf(fp,"S00 (const static)              : %s\n",S00.c_str());
fprintf(fp,"Urank                           : %d\n",Urank);
cIdx = 0;
WALKVECTOR(int,Usize,s)
  fprintf(fp,"Usize (comm %d)                 : %d\n",cIdx++,s);
fprintf(fp,"Ulen                            : %d\n",Ulen);
fprintf(fp,"Sproc                           : %s\n",Sproc.c_str());
fprintf(fp,"Suser                           : %s\n",Suser.c_str());
fprintf(fp,"UBPW                            : %u\n",UBPW);
fprintf(fp,"Scompiler                       : %s\n",Scompiler.c_str());
fprintf(fp,"SOS                             : %s\n",SOS.c_str());
fprintf(fp,"Ssource                         : %s\n",Ssource.c_str());
fprintf(fp,"Sbinary                         : %s\n",Sbinary.c_str());
fprintf(fp,"STIME                           : %s\n",STIME.c_str());
fprintf(fp,"SDATE                           : %s\n",SDATE.c_str());
if (pPmap.size()==0) fprintf(fp,"No process map\n");
else
{
WALKVECTOR(ProcMap*,pPmap,P)
  (*P)->Dump(fp);
}
fprintf(fp,"CommonBase-----------------------------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CommonBase::Connect(string svc)
// connects this process' MPI universe to a remote universe that has published
// a name to access it by.
{
int error = MPI_SUCCESS;
// a server has its port already so can just open a comm
if (svc=="") Comms.push_back(Tcomm);
else // clients need to look up the service name
{
   MPI_Comm newcomm;
   char port[MPI_MAX_PORT_NAME];
   // Get the published port for the service name asked for.
   // Exit if we don't get a port, probably because the remote universe isn't
   // initialised yet (we can always retry).
   if (error = MPI_Lookup_name(svc.c_str(),MPI_INFO_NULL,port)) return error;
   // now try to establish the connection itself. Again, we can always retry.
   // if (error = MPI_Comm_connect(port,MPI_INFO_NULL,0,MPI_COMM_WORLD,&newcomm)) return error;
   if (error = MPI_Comm_connect(port,MPI_INFO_NULL,0,Comms[0],&newcomm)) return error;
   Comms.push_back(newcomm); // as long as we succeeded, add to the list of comms
}
int rUsize;
MPI_Comm_remote_size(Comms.back(), &rUsize);
Usize.push_back(rUsize);       // record the size of the remote universe
FnMapx.push_back(new FnMap_t); // give the new comm some function tables to use
pPmap.push_back(new ProcMap(this));  // and a new processor map for the remote group
PMsg_p prMsg;
SendPMap(Comms.back(), &prMsg);        // Send our process data to the remote group
int fIdx=FnMapx.size()-1;
// populate the new function table with the global functions
(*FnMapx[fIdx])[Msg_p::KEY(Q::EXIT                )] = &CommonBase::OnExit;
(*FnMapx[fIdx])[Msg_p::KEY(Q::PMAP                )] = &CommonBase::OnPmap;
(*FnMapx[fIdx])[Msg_p::KEY(Q::SYST,Q::PING,Q::ACK )] = &CommonBase::OnSystPingAck;
(*FnMapx[fIdx])[Msg_p::KEY(Q::SYST,Q::PING,Q::REQ )] = &CommonBase::OnSystPingReq;
(*FnMapx[fIdx])[Msg_p::KEY(Q::TEST,Q::FLOO        )] = &CommonBase::OnTestFloo;
if (svc=="") Tcomm = MPI_COMM_NULL;  // release any Accept comm.
return error;
}

//------------------------------------------------------------------------------

int CommonBase::LogSCIdx()
{
int cIdx=0;
for (; cIdx < pPmap.size(); cIdx++)
  if (pPmap[cIdx]->U.LogServer!=Q::NAP) return cIdx;
return -1;
}

//------------------------------------------------------------------------------

int CommonBase::NameSCIdx()
{
int cIdx=0;
for (; cIdx < pPmap.size(); cIdx++)
  if (pPmap[cIdx]->U.NameServer!=Q::NAP) return cIdx;
return -1;
}

//------------------------------------------------------------------------------

void CommonBase::MPISpinner()
// Spinner to handle ALL incoming MPI packets. There is only one exit from this:
// Decode() returns 1 when an "exit" command is detected.
{
//int MSGBUFSIZ = 1 << LOG_MSGBUF_BLK_SZ;   // Individual incoming message buffer (was 1024)
//char * MSGBUF = new char[MSGBUFSIZ];   // Pull it off the heap
for (;;) {
// See if there are any MPI packets coming down the pipe
  MPI_Status status;                   // Note the multi-threaded MPI probe
  int flag;
//MPI_Message message;
//MPI_Improbe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&flag,&message,&status);
  for (unsigned cIdx = 0; cIdx < Comms.size(); cIdx++) // for all active comms
  {
    MPI_Iprobe(MPI_ANY_SOURCE,MPI_ANY_TAG,Comms[cIdx],&flag,&status);
    if (flag==0) {                       // Nothing there....
      OnIdle();                          // Guess
      continue;                          // And try again
    }
    int count;
    MPI_Get_count(&status,MPI_CHAR,&count);
//  printf("MPISpinner: %d bytes coming\n",count);  fflush(stdout);
//  vector<char> buf(count+1);
    if (count > Msgbufsz) {             // Ensure we have the space for it
      delete [] MPI_Buf;                  // Throw the too-small one away
      Msgbufsz = ((count>>LOG_MSGBUF_BLK_SZ)+1)<<LOG_MSGBUF_BLK_SZ;
      MPI_Buf = new char[Msgbufsz]; // Pull a new (bigger) one off the heap
      // MPI_Buf = new char[MSGBUFSIZ=count+1]; // Pull a new (bigger) one off the heap
    }
//  printf("MPISpinner: buf has %u elements\n",buf.size());  fflush(stdout);
    MPI_Request request;
//  MPI_Imrecv(&buf[0],count,MPI_CHAR,&message,&request);
    MPI_Irecv(&MPI_Buf[0],count,MPI_CHAR,MPI_ANY_SOURCE,MPI_ANY_TAG,Comms[cIdx],&request);
//  printf("MPISpinner: waiting...\n");  fflush(stdout);
    int i=0;
    do { i++; MPI_Test(&request,&flag,&status); } while (flag==0);
    // printf("MPISpinner on process %d: after %d, %s has landed\n",Urank,i,&MPI_Buf[0]);  fflush(stdout);
    PMsg_p Pkt((byte *)&MPI_Buf[0],count);// Turn it into a packet
    Pkt.Ztime(1,MPI_Wtime());            // Timestamp arrival
    if (Decode(&Pkt,cIdx)!=0) return;    // Do it, possibly leaving afterwards
  }
}
//printf("********* CommonBase MPIspinner rank %d on the way out\n",Urank); fflush(stdout);
// Here iff Decode() found an "exit" command

//if (MPI_Buf!=0) delete [] MPI_Buf;       // Kill the individual message buffer
}

//------------------------------------------------------------------------------

unsigned CommonBase::OnExit(PMsg_p * Z,unsigned cIdx)
// Do not post anything further here - the LogServer may have already gone
{
WALKVECTOR(MPI_Comm,Comms,C) MPI_Barrier(*C);
// printf("(%s)::CommonBase::OnExit \n",Sderived.c_str());    fflush(stdout);
if (AcceptConns) // Accept thread running?
{
   // printf("(%s)::CommonBase::OnExit closing down Accept MPI request on rank %d\n",Sderived.c_str(),Urank);
   // fflush(stdout);
   // REALLY silly: have to close the Accept thread via a matching MPI_Comm_connect
   // because the MPI interface has made no provision for a nonblocking accept, thus
   // otherwise the Accept thread will block forever waiting for a message that will
   // never come because we are shutting down. See Gropp, et al. "Using Advanced MPI"
   if (!Urank)
   {
   MPI_Comm dcomm;
   // printf("Accept port name: %s\n", MPIPort);
   // fflush(stdout);
   MPI_Comm_connect(MPIPort,MPI_INFO_NULL,0,MPI_COMM_SELF,&dcomm);
   }
   // printf("(%s)::CommonBase::OnExit waiting for Accept thread to exit\n",Sderived.c_str());
   // fflush(stdout);
   pthread_join(MPI_accept,NULL);
   AcceptConns = false; // stop accepting connections
   // printf("(%s)::CommonBase::OnExit shutting down\n",Sderived.c_str());
   // fflush(stdout);
}
if (Urank == Lrank)
{
   MPI_Unpublish_name(MPISvc,MPI_INFO_NULL,MPIPort);
   MPI_Close_port(MPIPort);
}
return 1;
}

//------------------------------------------------------------------------------

void CommonBase::OnIdle()
{

}

//------------------------------------------------------------------------------

unsigned CommonBase::OnPmap(PMsg_p * Z,unsigned cIdx)
{
pPmap[cIdx]->Register(Z);                    // Load one element into the process map
if ((cIdx == 0) && (pPmap[0]->vPmap.size() == Usize[0])) // once we have everybody local
{
   // we can decide who the leader will be for any intercomms.
   if (pPmap[0]->U.Mothership.size()) Lrank = *min_element(pPmap[0]->U.Mothership.begin(),pPmap[0]->U.Mothership.end());
   else Lrank = 0; // intercomm leader (lowest-rank mothership or 0)
   // printf("Accept intercomm leader rank: %d, for process at rank %d\n",Lrank,Urank);
   // fflush(stdout);
   if (Urank == Lrank) // and if we are the leader,
   {
       MPI_Open_port(MPI_INFO_NULL,MPIPort); // Announce to MPI that we are open for business
       MPI_Publish_name(MPISvc,MPI_INFO_NULL,MPIPort); // and make us publicly available
       string port_name(MPIPort); // all local processes should get the port name

       // start up the Accept thread everywhere.
       PMsg_p Areq;
       Areq.Key(Q::SYST,Q::ACPT);
       Areq.Src(Urank);
       Areq.comm = Comms[0];
       Areq.Put(0,&port_name);
       // Disable Accept thread until MPI issue with collectives on MPI_THREAD_MULTIPLE can be resolved
       // Areq.Bcast();
       // return OnSystAcpt(&Areq,0); // including ourselves.
   }
}
return 0;
}

//------------------------------------------------------------------------------

unsigned CommonBase::OnSystAcpt(PMsg_p * Z,unsigned cIdx)
{
if (cIdx) // Accept is only valid on the process' native comm.
{
Post(61, uint2str(cIdx));
return 1;
}
string port_name;
Z->Get(0,port_name);
strcpy(MPIPort,port_name.c_str());
// printf("Accept port name for process rank %d: %s\n", Urank, MPIPort);
// fflush(stdout);
AcceptConns = true; // set the flag enabling accepts
void* args = this;                   // spin off a thread to accept MPI connections
if (pthread_create(&MPI_accept,NULL,Accept,args))  // from other universes
{
AcceptConns = false;
Post(63, uint2str(Urank));                         // failure is fatal
return 1;
}
return 0;
}

//------------------------------------------------------------------------------

unsigned CommonBase::OnSystConn(PMsg_p * Z,unsigned cIdx)
{
if (cIdx)
{
Post(61, uint2str(cIdx));
return 1;
}
string svc;
Z->Get(0,svc);              // What is our service name?
return Connect(svc);        // connect as requested.
}

//------------------------------------------------------------------------------

unsigned CommonBase::OnSystPingAck(PMsg_p * Z,unsigned cIdx)
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
string TgtS = pPmap[cIdx]->M[TgtR];    // Target name
const unsigned len = 20;               // Process name length
TgtS.resize(len,' ');                  // So they line up on the console
string Fr(Sderived);
Fr.resize(len,' ');
unsigned * pu = Z->Get<unsigned>(4,cnt);// Ping attempt
Post(21,uint2str(*pu),Fr,int2str(Urank),TgtS,int2str(Z->Src()),dbl2str(trip));
return 0;
}

//------------------------------------------------------------------------------

unsigned CommonBase::OnSystPingReq(PMsg_p * Z,unsigned cIdx)
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
/* Qt whinges about the following 2 lines - doesn't like taking the
   address of a temporary. Presumably it is being more strict about
   compliance with standards. In any case, this means annoyingly creating
   2 silly extra string objects just to get their addresses.
   Pkt.Put(4,&string(GetDate()));       // Payload
   Pkt.Put(5,&string(GetTime()));
*/
string tD(GetDate());
string tT(GetTime());
Pkt.Put(4,&tD);         // Payload
Pkt.Put(5,&tT);
Pkt.Put<int>(0,&Urank);                // My (sending) rank. Why, Lord?
Pkt.Src(Urank);                        // My sending rank.
Pkt.comm = Comms[cIdx];                // Return path communicator
Pkt.Send();
return 0;
}

//------------------------------------------------------------------------------

unsigned CommonBase::OnSystRun(PMsg_p * Z, unsigned cIdx)
{
    return 0;
}

//------------------------------------------------------------------------------

unsigned CommonBase::OnTestFloo(PMsg_p * Z,unsigned cIdx)
// Got a test flood event.
{
static unsigned stop = 0;              // Some statistices
static unsigned sent = 0;
int c;
unsigned lev = *(Z->Get<unsigned>(2,c));// Extract remaining level
//printf("CommonBase::OnTest rank %u (coming) level %u\n",Urank,lev);  fflush(stdout);
if (lev==0) {                           // If we're done...
  Post(54,Sproc,uint2str(Urank));
  return 0;
}
--lev;

//printf("CommonBase::OnTest rank %u (middling) level %u\n",Urank,lev);  fflush(stdout);

Z->Put<unsigned>(2,&lev,1);              // No, decrement the level

//lev = *(Z->Get<unsigned>(2,c));
//printf("CommonBase::OnTest rank %u (going) level %u\n",Urank,lev);  fflush(stdout);

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
if (pPmap.size()==0)            throw (Unrec_t(2,Sderived,"Post"));
int cIdx=LogSCIdx();
if (cIdx < 0)
{
   // No LogServer. Best we can do is hope we are the Root process and can output a message.
   // Later this might become more sophisticated and attempt to send a message to Root.
   if (Urank == pPmap[0]->U.Root) printf("Error: attempted to Post a message %d without a LogServer\n");
   return false;
}
PMsg_p Pkt;
Pkt.PutX(1,&varg);
Pkt.Put<int>(1,&i);
Pkt.Key(Q::LOG,Q::POST);
Pkt.Src(Urank);
Pkt.comm = Comms[cIdx];
Pkt.Send(pPmap[cIdx]->U.LogServer);
// We always return true so we can write things like ....if(Post(...))return;...
return true;
}

//------------------------------------------------------------------------------

void CommonBase::Prologue(int argc,char ** argv)
// Startup code for EVERY orchestrator process.
{
int tUsize;
MPI_Comm_size(Comms[0],&tUsize); // Universe size
Usize.push_back(tUsize);
MPI_Comm_rank(Comms[0],&Urank);  // My place within it
char Uname[MPI_MAX_PROCESSOR_NAME];
MPI_Get_processor_name(Uname,&Ulen);   // Machine name
Sproc = string(Uname);                 // Translated from the original FORTRAN
Suser = getenv("USER");                // User name as seen by the process
strcpy(MPIPort,"PORT_NULL");           // Default MPI port

// ---------- THIS DOES NOT APPEAR TO SET ANYTHING PERSISTENT ------------------

int * io_chan;
int   io_flag;                         // Establish who has IO
MPI_Comm_get_attr(Comms[0],MPI_IO,&io_chan,&io_flag); // replaces deprecated MPI_Attr_get

//------------------------------------------------------------------------------

Tcomm = MPI_COMM_NULL;                 // initialise the Accept intercommunicator

UBPW = BPW();                          // ... bits per word
Scompiler = GetCompiler();             // ... compiler
SOS = GetOS();                         // ... Operating system
Sbinary = string(argv[0]);             // ... binary file
STIME = string(__TIME__);              // ... compilation time
SDATE = string(__DATE__);              // ... compilation date

PMsg_p Pkt;
SendPMap(Comms[0], &Pkt);        // Send our data to everybody else
pPmap[0]->Register(&Pkt);              // Need to put ourselves in seperately
//printf("%s:CommonBase at barrier\n",Sderived.c_str()); fflush(stdout);
MPI_Barrier(Comms[0]);           // Wait until everyone has told everyone
//printf("%s:CommonBase through barrier\n",Sderived.c_str()); fflush(stdout);
}

//------------------------------------------------------------------------------

int CommonBase::RootCIdx()
{
int cIdx=0;
for (; cIdx < pPmap.size(); cIdx++)
  if (pPmap[cIdx]->U.Root!=Q::NAP) return cIdx;
return -1;
}

//------------------------------------------------------------------------------

void CommonBase::SendPMap(MPI_Comm comm, PMsg_p* Pkt)
{
// Shove the data all into a packet:
Pkt->Put<int>(1,&Urank);                // Load my local rank...
Pkt->Put(2,&Sproc);                     // ... processor name
Pkt->Put(3,&Suser);                     // ... user name
Pkt->Put(4,&Sderived);                  // ... C++ class
Pkt->Put<unsigned>(5,&UBPW);            // ... bits per word
Pkt->Put(6,&Scompiler);                 // ... compiler
Pkt->Put(7,&SOS);                       // ... operating system
Pkt->Put(8,&Ssource);                   // ... source file
Pkt->Put(9,&Sbinary);                   // ... binary file
Pkt->Put(10,&STIME);                     // ... compilation time
Pkt->Put(11,&SDATE);                    // ... compilation date
Pkt->Put<int>(12,&MPI_provided);        // ... MPI thread class
Pkt->Key(Q::PMAP);                      // Message key
Pkt->Src(Urank);
Pkt->comm = comm;                       // communicator over which this is being sent
                                       // Build process map
//printf("%s:CommonBase about to send ProcMap records\n",Sderived.c_str());
//fflush(stdout);                      // Tell everyone else
Pkt->Bcast();                           // via a broadcast
}

//==============================================================================

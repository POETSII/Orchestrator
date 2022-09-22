//------------------------------------------------------------------------------

#include "MonServer.h"
#include "CommonBase.h"
#include "PMsg_p.hpp"
#include "mpi.h"
#include "Pglobals.h"
#include <stdio.h>

//==============================================================================

void OMsg(void * pSkyHook,string msg)
// Posts messages if in debug mode.
{
#if ORCHESTRATOR_DEBUG
((MonServer *)pSkyHook)->Post(433,msg);
#endif
}

//==============================================================================

void ORecv(int socket,void * pSkyHook,vector<byte> buf,int count)
// (Flat) callback for messages in from remote client monitors. Does 2 things:
// (1) Mindlessly forwards the message to the root process
// (2) Ckanges the key and sends it back to the remote monitor
{
if (count<=0) return;                  // Life is too short
                                       // Unpack the way back in
MonServer * pMonServer = (MonServer *)pSkyHook;
PMsg_p Z(&buf[0],count);               // OK, it's (probably) a PMsg_p
double T = MPI_Wtime();                // Timestamp
Z.Put<double>(-2,&T);
string K0 = hex2str(unsigned(Z.L(0)));  // Diagnostics - bigger than the
string K1 = hex2str(unsigned(Z.L(1)));  // actual code
string K2 = hex2str(unsigned(Z.L(2)));
string K3 = hex2str(unsigned(Z.L(3)));
string Ks = string("| 0x") + K0 + string("| 0x") + K1 + string("| 0x") +
            K2 + string("| 0x") + K3 + string("|");
Z.Put<int>(99,&socket);                // Put the socket into the message
Z.Put<void *>(99,&pSkyHook);           // Put the MonServer address in
                                       // Do we recognize it?
if (pMonServer->FnMap.find(Z.Key())!=pMonServer->FnMap.end()) // Yerp
  (pMonServer->*(pMonServer->FnMap)[Z.Key()])(&Z);            // So do it
else pMonServer->Post(405,Ks);                                // Nope
}

//==============================================================================

MonServer::MonServer(int argc,char * argv[],string d):
  CommonBase(argc,argv,d,string(__FILE__))
{
                                       // Fire up the server
Server.SetSkyHook(this);               // Tell receiver function about parent
Server.SetMCB(OMsg);                   // This first!
Server.SetPCB(ORecv);                  // Attach it
// TODO Put the listening port into the Orchestrator config system
Server.Recv(28755);                    // ...and start listening
                                       // Load the message map
                                       // One map holds keys from both the MPI
                                       // Universe and the remote socket
nextRequestId = 0;
FnMap[PMsg_p::KEY(Q::MONI,Q::DEVI,Q::REQ )] = &MonServer::OnMoniDeviReq;
FnMap[PMsg_p::KEY(Q::MONI,Q::DEVI,Q::ACK )] = &MonServer::OnMoniDeviAck;
FnMap[PMsg_p::KEY(Q::MONI,Q::INJE,Q::REQ )] = &MonServer::OnMoniInjeReq;
FnMap[PMsg_p::KEY(Q::MONI,Q::INJE,Q::ACK )] = &MonServer::OnMoniInjeAck;
FnMap[PMsg_p::KEY(Q::MONI,Q::MOTH,Q::DATA)] = &MonServer::OnMoniMothData;
FnMap[PMsg_p::KEY(Q::MONI,Q::SOFT,Q::DATA)] = &MonServer::OnMoniSoftData;
FnMap[PMsg_p::KEY(Q::MONI,Q::SPY         )] = &MonServer::OnMoniSpy;
FnMap[PMsg_p::KEY(Q::MONI,Q::TRAC        )] = &MonServer::OnMoniTrac;
MPISpinner();                          // Spin on MPI messages; exit only on DIE
//printf("********* MonServer rank %d on the way out\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

MonServer::~MonServer()
{
}

//------------------------------------------------------------------------------

void MonServer::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sMonServer dump +++++++++++++++++++++++++++++++++++++++++++\n",os);
printf("%sMessage handler function map:\n",os);
printf("%sKey        Method\n",os);
WALKMAP(unsigned,pMeth,FnMap,i) {
  fprintf(fp,"%s%#010x %" PTR_FMT "\n",os,(*i).first,
           OSFixes::getAddrAsUint((*i).second));
}
CommonBase::Dump(off+2,fp);
fprintf(fp,"%sMonServer dump -------------------------------------------\n",os);
}

//------------------------------------------------------------------------------

unsigned MonServer::OnMoniDeviAck(PMsg_p * pZ)
// Forward to the remote monitor; ack of "find me a device"
{
//pZ->FDump("MonServer_OnMoniDeviAck");
int count;
int skt = -1;
int * pskt = pZ->Get<int>(99,count);
if (pskt!=0) skt = *pskt;
else {
  Post(404);
  return 0;
}

Send(skt,pZ);
return 0;
}

//------------------------------------------------------------------------------

unsigned MonServer::OnMoniDeviReq(PMsg_p * pZ)
// "Start data exfiltration" command in from the Remote Monitor
{
pZ->Mode(2);                           // ONWARD PATH: Monserver->Root
pZ->Put<int>(-2,&nextRequestId);
nextRequestId++;
//pZ->FDump("MonServer_OnMoniDeviReq");
pZ->Send(pPmap->U.Root);               // I.e. unconditional forward to rank 0

                                       // ACKNOWLEDGMENT RETURN PATH
pZ->Key(Q::MONI,Q::DEVI,Q::ACK);       // Turn it into an acknowledgment message
pZ->Mode(1);               // I'm getting less sure I need "mode"....
int count;                             // Byte count
int skt;                               // Return socket
int * pskt = pZ->Get<int>(99,count);   // Extract socket from message
if (pskt!=0) skt = *pskt;              // Good?
else {                                 // No
  Post(404);                           // Bleat
  return 0;                            // Absquatulate, pointlessly
}
void ** ppV = pZ->Get<void *>(99,count);
if (ppV==0)
{
  Post(406);
  return 0;
}

bool error = false;
pZ->Put<bool>(6,&error);
double T = MPI_Wtime();
pZ->Put<double>(-20,&T);                // Timestamp: Leaving MonServer
Send(skt,pZ);
return 0;
}

//------------------------------------------------------------------------------

unsigned MonServer::OnMoniInjeAck(PMsg_p * pZ)
// Forward to the remote monitor; ack of "inject commend"
{
//pZ->FDump("MonServer_OnMoniInjeAck");
int count;
int skt = -1;
int * pskt = pZ->Get<int>(99,count);
if (pskt!=0) skt = *pskt;
else {
  Post(404);
  return 0;
}
pZ->Mode(2);
Send(skt,pZ);
return 0;
}

//------------------------------------------------------------------------------

unsigned MonServer::OnMoniInjeReq(PMsg_p * pZ)
// From : Remote monitor
// To   : Root
// POETS command injected direct from the remote monitor
{
pZ->Mode(2);                           // ONWARD PATH: Monserver->Root
pZ->Put<int>(-2,&nextRequestId);
nextRequestId++;
//pZ->FDump("MonServer_OnMoniInjeReq");
pZ->Send(pPmap->U.Root);               // I.e. unconditional forward to rank 0
                                       // ACKNOWLEDGMENT RETURN PATH
pZ->Key(Q::MONI,Q::INJE,Q::ACK);       // Turn it into an acknowledgment message
pZ->Mode(1);
//pZ->FDump("MonServer_OnMoniInjeReq");
int count;                             // Byte count
int skt;                               // Return socket
int * pskt = pZ->Get<int>(99,count);   // Extract socket from message
if (pskt!=0) skt = *pskt;              // Good?
else {                                 // No
  Post(404);                           // Bleat
  return 0;                            // Absquatulate
}
void ** ppV = pZ->Get<void *>(99,count);
if (ppV==0)
{
  Post(406);
  return 0;
}

double T = MPI_Wtime();
pZ->Put<double>(-20,&T);                // Timestamp on leaving MonServer
Send(skt,pZ);                           // Send ACK back to remote monitor
return 0;
}

//------------------------------------------------------------------------------

unsigned MonServer::OnMoniMothData(PMsg_p * pZ)
// Forward to the remote monitor, and track if enabled
{
//pZ->FDump("MonServer_OnMoniMothData");
double T = MPI_Wtime();
pZ->Put<double>(-2,&T);                // Timestamp on entering MonServer
if (Tracker(pZ)) Post(438, Tracker.GetError());
int count;                             // Pull out the target socket
int skt = -1;
int * pskt = pZ->Get<int>(99,count);
if (pskt!=0) skt = *pskt;
else {
  Post(404);
  return 0;
}
pZ->Mode(4);
T = MPI_Wtime();
pZ->Put<double>(-20,&T);                // Timestamp on leaving MonServer
//pZ->FDump("MonServer_OnMoniMothData");
Send(skt,pZ);
return 0;
}

//------------------------------------------------------------------------------

unsigned MonServer::OnMoniSoftData(PMsg_p * pZ)
// Forward to the remote monitor
{
//pZ->FDump("MonServer_OnMoniSoftData");
//printf("MONI|SOFT|DATA\n"); fflush(stdout);
double T = MPI_Wtime();
pZ->Put<double>(-2,&T);                // Timestamp on entering MonServer
if (Tracker(pZ)) Post(438, Tracker.GetError());
int count;
int skt = -1;
int * pskt = pZ->Get<int>(99,count);
if (pskt!=0) skt = *pskt;
else {
  Post(404);
  return 0;
}
pZ->Mode(4);
T = MPI_Wtime();
pZ->Put<double>(-20,&T);                // Timestamp on leaving MonServer
//pZ->FDump("MonServer_OnMoniSoftData");
Send(skt,pZ);
return 0;
}

//------------------------------------------------------------------------------

unsigned MonServer::OnMoniSpy(PMsg_p * pZ)
// Enable or disable the spy.
{
Spy.Toggle();
string spyDirToSet = "";
pZ->Get(0, spyDirToSet);
Post(420, Spy.IsEnabled() ? "enabled" : "disabled");  // Spy state change.
if (Spy.SetSpyDir(spyDirToSet)) Post(421, spyDirToSet);  // Path change, if any.
return 0;
}

//------------------------------------------------------------------------------

unsigned MonServer::OnMoniTrac(PMsg_p * pZ)
// Enable or disable the data tracker.
{
Tracker.Toggle();
string trackerDirToSet = "";
pZ->Get(0, trackerDirToSet);
// Tracker state change.
Post(436, Tracker.IsEnabled() ? "enabled" : "disabled");
// Path change, if any.
if (Tracker.SetTrackerDir(trackerDirToSet)) Post(437, trackerDirToSet);
return 0;
}

//------------------------------------------------------------------------------

int MonServer::Send(int socket, PMsg_p * pZ)
// Streams a PMsg_p over a socket, and writes it out with the Spy if set up to
// do so.
{
if (Spy(pZ)) Post(422, Spy.GetError());
return Server.Send(socket,pZ->Stream_v());
}

//==============================================================================

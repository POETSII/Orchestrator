//------------------------------------------------------------------------------

#include "Dummy.h"
#include "CommonBase.h"
#include "PMsg_p.hpp"
#include "rand.h"
#include "mpi.h"
#include "Pglobals.h"
#include <stdio.h>

//==============================================================================

Dummy::Dummy(int argc,char * argv[],string d):
  CommonBase(argc,argv,d,string(__FILE__))
{
                                       // Spin on incoming MPI messages
/*
PMsg_p Pkt;                            // Send out a bunch of random packets
Pkt.Src(Urank);
char buf[512] = "Ooogle";
for(int i=0;i<10;i++) {
  int len = strlen(buf)+1;             // Include the terminal '\0'
  Pkt.Put<char>(1,buf,len);
  Pkt.Key(Q::TEST);
  Pkt.Send(Q::ROOT); // dangerous: sending to a literal rank before procmap is loaded
  buf[0]++;
}
  */
                                       // Load the message map
FnMap[PMsg_p::KEY(Q::MONI,Q::DEVI,Q::REQ)] = &Dummy::OnMoniDeviReq;
//FnMap[PMsg_p::KEY(Q::MONI,Q::INJE,Q::REQ ,Q::N000)] = &MonServer::OnMoniInjeReq

MPISpinner();

}

//------------------------------------------------------------------------------

void Dummy::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sDummy dump +++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sKey        Method\n",os);
WALKMAP(unsigned,pMeth,FnMap,i)
{
  fprintf(fp,"%s%#010x %" PTR_FMT "\n",os,(*i).first,
                        OSFixes::getAddrAsUint((*i).second));
}
CommonBase::Dump(off+2,fp);
fprintf(fp,"%sDummy dump -----------------------------------------------\n",os);

}

//------------------------------------------------------------------------------

unsigned Dummy::OnMoniDeviReq(PMsg_p * pZ)
// Request data for a named and resolved device
{
//pZ->FDump("Dummy_OnMoniDeviReq");
double T = MPI_Wtime();                // Timestamp: On entering Mothership
pZ->Put<double>(-4,&T);
pZ->Mode(3);                           // Change mode
pZ->L(2,Q::ACK);                       // Change it to an ACK and send it back
T = MPI_Wtime();                       // Timestamp: On leaving Mothership
pZ->Put<double>(-40,&T);
pZ->Send(pPmap->U.MonServer);          // To the MonServer
/*******************************************************************************
OK, done with the admin. Let's squirt some dummy data back to the Monitor.
STOP commands don't do anything.
START commands send out a bunch of dummy messages, with an interval set by the
monkey.
The number of dummy messages is set in the Wizards back passage.
*****************************************************************************/
int cnt;
bool * pB = pZ->Get<bool>(0,cnt);      // If this is a STOP....
bool B = false;
if (pB!=0) B = *pB;
if (!B) return 0;                      // ....just go
                                       // OK, it's a START
unsigned * pU = pZ->Get<unsigned>(666,cnt);
unsigned Wcount = 16;                  // Default number
if (pU!=0) Wcount = *pU;               // An actual number

                                       // Unpack the interval
unsigned * pInt = pZ->Get<unsigned>(0,cnt);
unsigned Int = 1000;                      // Default interval
if (pInt!=0) Int = *pInt;              // An actual interval

int skt = -1;                          // Remote monitor socket
int * pskt = pZ->Get<int>(99,cnt);
if (pskt!=0) skt = *pskt;
else {
  Post(404);
  return 0;
}
void * pV = 0;                          // Remote monitor child window
void ** ppV = pZ->Get<void *>(98,cnt);
if (ppV!=0) pV = *ppV;
else {
  Post(408);
  return 0;
}

PMsg_p DZ;                             // Initialise dummy data
DZ.Key(Q::MONI,Q::SOFT,Q::DATA);
for (unsigned u=0;u<4;u++) DZ.Zname(u,pZ->Zname(u));
DZ.Put<int>(99,&skt);                  // Remote monitor socket
DZ.Put<void *>(98,&pV);                // Remote monitor child window

//DZ.FDump("Dummy_OnMoniDeviReq");
RandInt RNG;
string sig = "0uidfb";                                 DZ.Put(0,&sig);
string Su = "Collatz conjecture (unsigned)";           DZ.Put(1,&Su);
string Si = "Some bunch of random integers";           DZ.Put(2,&Si);
string Sd = "Double precision Fibonacci sequence";     DZ.Put(3,&Sd);
string Sf = "Collatz conjecture (float)";              DZ.Put(4,&Sf);
string Sb = "The moving finger writes....";            DZ.Put(5,&Sb);
unsigned Du = 3657683479;              // Dummy unsigned
int      Di = 1807230980;              // Guess....
double   Dd = 10.908e6;   double Dd_ = -654.2e5; double _;
float    Df = (float)Du;
bool     Db = false;

for (unsigned u=0;u<Wcount;u++) {      // Build and send some dummy data
  DZ.Put<unsigned>(1,&Du);
  DZ.Put<int>(2,&Di);
  DZ.Put<double>(3,&Dd);
  DZ.Put<float>(4,&Df);
  DZ.Put<bool>(5,&Db);
  T = MPI_Wtime();                     // Timestamp: On leaving Mothership
  DZ.Put<double>(-40,&T);
  DZ.Send(pPmap->U.MonServer);        // To the MonServer with it all!
                                       // Let us update:
  Du = ((Du%2)==0) ? Du/2 : (3*Du)+1;
  Di =  RNG.draw();
  _=Dd; Dd = Dd+Dd_; Dd_=_;
  int Df_=(int)(Df/2.0); Df = ((Df_%2)==0) ? Df/2.0 : (3.0*Df)+1.0;
  Db = ~Db;
  Sleep(Int);                          // Wait for it....
}


return 0;
}
 //    https://en.wikipedia.org/wiki/Collatz_conjecture
//==============================================================================





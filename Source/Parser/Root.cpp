//------------------------------------------------------------------------------

#include "Root.h"
#include "CommonBase.h"
#include "pthread.h"
#include <stdio.h>
#include "PMsg_p.hpp"
#include "Pglobals.h"
#include "Cli.h"
#include "flat.h"

const char * Root::prompt = "POETS>";

//------------------------------------------------------------------------------

void * kb_func(void * pPar)
// Thread function to listen to the keyboard, package up each line in a message
// and send it to the main thread in this process, where it gets picked up by
// the MPI spinner.
{
//printf("\nRoot::kb_func: thread starting\n\n"); fflush(stdout);
int len = 0;                           // Characters in buffer
for(;;) {
  if (len==0) Root::Prompt();          // Console prompt
  fflush(stdout);
  static const unsigned SIZE = 512;
  char buf[SIZE];
  buf[0] = '\0';                       // Borland bug: notes 21/7/17
  for(int j=1;j<SIZE;j++) buf[j]='x';
  fgets(buf,SIZE-1,stdin);             // Pull in keyboard string
  len=strlen(buf)-1;                   // Ignore trailing newline
  if (len==0) continue;                // Hard to see how
  if (buf[len]=='\n')buf[len]='\0';    // Replace trailing newline
  PMsg_p Pkt;
  Pkt.Put<char>(1,buf,len+2);          // Put it in a packet
  Pkt.Key(Q::KEYB);

//int cnt;                               // cnt now includes trailing '\0'
//char * obuf = Pkt.Get<char>(1,cnt);
//printf("len=%d, cnt=%d, obuf=%s\n",len,cnt,obuf);  fflush(stdout);

  Pkt.Src(0);
  Pkt.Send(0);                         // Send to root process main thread
  if (strcmp(buf,"exit")==0) break;    // User wants out - kill this thread
}

//printf("kb_func: thread closing\n"); fflush(stdout);
pthread_exit(NULL);
return NULL;
}

//==============================================================================

Root::Root(int argc,char * argv[],string d) :
  OrchBase(argc,argv,d,string(__FILE__))
{
                                       // Load the incoming event map
FnMapx[PMsg_p::KEY(Q::KEYB        )] = &Root::OnKeyb;
FnMapx[PMsg_p::KEY(Q::TEST        )] = &Root::OnTest;
//FnMapx[PMsg_p::KEY(Q::TEST,Q::FLOO)] = &Root::OnTestFloo;
FnMapx[PMsg_p::KEY(Q::LOG ,Q::FULL)] = &Root::OnLogP;

void * args = this;                    // Spin off a thread to handle keyboard
pthread_t kb_thread;
if(pthread_create(&kb_thread,NULL,kb_func,args))
  fprintf(stdout,"Error creating kb_thread\n");

MPISpinner();                          // Spin on *all* messages; exit on DIE
//printf("********* Root rank %d on the way out\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

Root::~Root()
{
//printf("********* Root rank %d destructor\n",Urank); fflush(stdout);

}

//------------------------------------------------------------------------------

unsigned Root::CmDrop(Cli * pC)
// Handle an unrecognised command from the monkey
{
if (pC==0) return 0;                   // Paranoia
Post(24,pC->Orig);
return 0;
}

//------------------------------------------------------------------------------

unsigned Root::CmExit(Cli * pC)
// Handle "exit" from the monkey. We don't use Bcast, because that'll shut
// everything down in random order, and we need the LogServer to be the last man
// standing (aside from us) so we can get the message acks back for the
// console
{
if (pC==0) return 0;                   // Paranoia
PMsg_p Pkt;                            // Burst closedown command to all ranks
Pkt.Src(Urank);
Pkt.Key(Q::EXIT);
int pL = pPmap->U.LogServer;           // Cache LogServer rank
for(int p=0;p<Usize;p++)
  if ((p!=Urank)&&(p!=pL)) {           // NOT the LogServer
    Post(50,pPmap->M[p],int2str(p));
    Pkt.Send(p);
  }
Post(50,pPmap->M[pL],int2str(pL));     // Ensure LogServer goes last
Post(50,Sderived,int2str(Urank));      // Root is going anyway
Pkt.Send(pL);
return 1;                              // Return closedown flag
}

//------------------------------------------------------------------------------

unsigned Root::CmRTCL(Cli * pC)
// Handle "RTCL" from the monkey. We've already parsed the command line, so we
// know it's OK. Rather than pack/unpack the data structure, just send the
// original command line string and the RTC can re-parse it - it's never very
// long. It's not like we're paying for the electricity.
{
if (pC==0) return 0;                   // Paranoia
//pC->Dump();
PMsg_p Pkt;
Pkt.Put(1,&(pC->Orig));                // Command line
Pkt.Src(Urank);                        // Me
Pkt.Key(Q::RTCL);                      // Key
Pkt.Send(pPmap->U.RTCL);               // Away we go
return 0;                              // Legitimate command exit
}

//------------------------------------------------------------------------------

unsigned Root::CmSyst(Cli * pC)
// Handle "system" from the monkey.
{
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_V,i) {     // Walk the clause list
  bool used = false;
  string scl = (*i).Cl;                // Pull out clause name
  if (strcmp(scl.c_str(),"ping")==0) used=CmSystPing(&(*i));
  if (strcmp(scl.c_str(),"show")==0) used=CmSystShow(&(*i));
  if (strcmp(scl.c_str(),"time")==0) used=CmSystTime(&(*i));
  if (!used) Post(25,scl,"system");    // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//------------------------------------------------------------------------------

bool Root::CmSystPing(Cli::Cl_t * pCl)
//
{
if (pCl->Pa_v.size()==0) return (Post(48,"ping","system","1"));
for(unsigned k=0;k<4;k++) {
//Post(27,uint2str(k));
  WALKVECTOR(Cli::Pa_t,pCl->Pa_v,i) {  // Walk the parameter (ping) list
//    printf("%s\n",(*i).Val.c_str()); fflush(stdout);
    string tgt = (*i).Val;             // Class name to be pinged
    if (Cli::StrEq(tgt,Sderived)) continue;           // Can't ping yourself
    WALKVECTOR(ProcMap::ProcMap_t,pPmap->vPmap,j) {   // Walk the process list
      if (Cli::StrEq((*j).P_class,Sderived)) continue;// Still can't ping self
      if ((Cli::StrEq(tgt,(*j).P_class))||(tgt=="*")) {
        /* Qt whinges about the following 2 lines - doesn't like taking the
           address of a temporary. Presumably it is being more strict about
           compliance with standards. In any case, this means annoyingly creating
           2 silly extra string objects just to get their addresses.
           Pkt.Put(2,&string(GetDate())); // Never actually used these
           Pkt.Put(3,&string(GetTime()));
        */
        string tD(GetDate());
        string tT(GetTime());
        PMsg_p Pkt;
        Pkt.Put(1,&((*j).P_class));    // Target process name
        Pkt.Put(2,&tD); // Never actually used these
        Pkt.Put(3,&tT);
        Pkt.Put<unsigned>(4,&k);       // Ping attempt
        Pkt.Src(Urank);                // Sending rank
        Pkt.Key(Q::SYST,Q::PING,Q::REQ);
        Pkt.Send((*j).P_rank);
      }
    }
  }
}
return true;                              // Legitimate command exit
}

//------------------------------------------------------------------------------

bool Root::CmSystShow(Cli::Cl_t * pCl)
// Monkey wants the list of processes
{
vector<string> vprocs;
if (pPmap!=0) pPmap->GetProcs(vprocs);
Post(29,uint2str(vprocs.size()));
Post(30,Sproc);
printf("\n");
WALKVECTOR(string,vprocs,i) printf("%s\n",(*i).c_str());
printf("\n");
fflush(stdout);
return true;                              // Legitimate command exit
}

//------------------------------------------------------------------------------

bool Root::CmSystTime(Cli::Cl_t * pCl)
// Monkey wants the time
{
string sT = GetTime();
string sD = GetDate();
Post(26,sD,sT);
return true;                              // Legitimate command exit
}

//------------------------------------------------------------------------------

unsigned Root::CmTest(Cli * pC)
// Test command handler. (It's already been trimmed)
{
if (pC->Cl_V.empty()) return 0;        // Nothing to do
WALKVECTOR(Cli::Cl_t,pC->Cl_V,i) {
  const char * cs = (*i).Cl.c_str();
  if (strcmp(cs,"time")==0) {          // Tell the monkey the time
    unsigned count = str2int((*i).GetP(0,"16"));
    for (unsigned i=0;i<=count;i++) Post(46,int2str(i,2),GetTime());
  }
  if (strcmp(cs,"floo")==0) {          // Initiate a flood event
    unsigned width = str2uint((*i).GetP(0,"2"));
    unsigned level = str2uint((*i).GetP(1,"4"));
    PMsg_p Z;
    Z.Key(Q::TEST,Q::FLOO);
    Z.Put<unsigned>(1,&width);
    Z.Put<unsigned>(2,&level);
    Z.Bcast();
    Post(51,uint2str(width),uint2str(level));
  }
}
return 0;
}

//------------------------------------------------------------------------------

unsigned Root::CmTopo(Cli * pC)
{
pP->Cm(pC);                            // Hand the command line to node graph


return 0;
}

//------------------------------------------------------------------------------

void Root::Dump(FILE * fp)
{
fprintf(fp,"Root dump+++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"Event handler table:\n");
fprintf(fp,"Key        Method\n");
WALKMAP(unsigned,pMeth,FnMapx,i)
  fprintf(fp,"%#010x 0x%#010p\n",(*i).first,(*i).second);
fprintf(fp,"prompt    = %s\n",prompt);

fprintf(fp,"Root dump-----------------------------------\n");
fflush(fp);
CommonBase::Dump(fp);
}

//------------------------------------------------------------------------------

unsigned Root::OnKeyb(PMsg_p * Z)
// Handle a message coming in from the monkey.
{
int cnt;
char * buf = Z->Get<char>(1,cnt);
//printf("Rank %d : Root::OnKeyb: %d ||%s||\n",Urank,cnt,buf);    fflush(stdout);
Cli Pc(buf);                           // Wake up the command line interpreter
int l,c;
if (Pc.Err(l,c)) {                     // Handle any syntax errors locally
  string eline = string(strlen(prompt),' ');
  eline += string(c-1,' ') + "^^^";
  printf("%s\nCommand line syntax error - line ignored\n",eline.c_str());
  Prompt();                            // Console prompt
  return 0;                            // Bail with "not closedown" value
}
Post(23,buf);                          // Copy to logfile
unsigned ret = ProcCmnd(&Pc);          // Try and make sense of it
Prompt();                              // Console prompt
return ret;
}

//------------------------------------------------------------------------------

unsigned Root::OnLogP(PMsg_p * Z)
// Handler for an expanded full posted message back from the LogServer
//      LOG|FULL|   -|   - (1:int)message_id,
//                         (2:char)message_type,
//                         (3:string)full_message
{
string sfull;                          // Full message
Z->Get(3,sfull);
int id = 0;                            // Message id
int * pid = 0;
int cnt;
pid = Z->Get<int>(1,cnt);
if (pid!=0) id = *pid;
char ty = 0;                           // Message type
char * pty = 0;
pty = Z->Get<char>(2,cnt);
if (pty!=0) ty = *pty;
string t = GetTime();
printf(" %s: %3d(%c) %s\n",t.c_str(),id,ty,sfull.c_str());
fflush(stdout);
Prompt();
return 0;
}

//------------------------------------------------------------------------------

unsigned Root::OnTest(PMsg_p * Z)
// This is an override for CommonBase::OnTest, which the rest of the Universe
// is using
// This is ADB playing; ignore me
{
//int cnt;
//char * buf = Z->Get<char>(1,cnt);
//printf("Rank %d : Root::OnTest: ||%s||\n",Urank,buf);    fflush(stdout);
return 0;
}

//------------------------------------------------------------------------------

unsigned Root::ProcCmnd(Cli * pC)
// Take the monkey command and point it to the right place
// All known monkey input is tabulated in Pglobals.h,
// (along with all known message layouts).
{
if (pC==0) return 1;                   // Paranoia
pC->Trim();
string scmnd = pC->Co;                 // Pull out the command string
// Note you can't use .size() here, because some strings have been resized to 4,
// so for them the size includes the '\0', whereas the others, it doesn't.....
if (strcmp(scmnd.c_str(),"exit")==0) return CmExit(pC);
if (strcmp(scmnd.c_str(),"test")==0) return CmTest(pC);
if (strcmp(scmnd.c_str(),"syst")==0) return CmSyst(pC);
if (strcmp(scmnd.c_str(),"rtcl")==0) return CmRTCL(pC);
if (strcmp(scmnd.c_str(),"topo")==0) return CmTopo(pC);
if (strcmp(scmnd.c_str(),"task")==0) return CmTask(pC);
return CmDrop(pC);
}

//------------------------------------------------------------------------------

void Root::Prompt(FILE * fp)
{
if (fp!=stdout) return;
fprintf(fp,"%s",Root::prompt);     // Console prompt
fflush(fp);
}

//==============================================================================




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
Root* parent = static_cast<Root*>(pPar);
int len = 0;                           // Characters in buffer
for(;;) {                              // Superloop
  if (len==0) Root::Prompt();          // Console prompt
  static const unsigned SIZE = 512;
  char buf[SIZE];
  buf[0] = '\0';                       // Borland bug: notes 21/7/17
  for(unsigned j=1;j<SIZE;j++) buf[j]='x';
  fgets(buf,SIZE-1,stdin);             // Pull in keyboard string
  len=strlen(buf)-1;                   // Ignore trailing newline
  if (len==0) continue;                // Hard to see how
  if (buf[len]=='\n')buf[len]='\0';    // Replace trailing newline
  PMsg_p Pkt;
  Pkt.comm = parent->Comms[0];         // comm is always our local one (index 0)
  Pkt.Put<char>(1,buf,len+2);          // Put it in a packet
  Pkt.Key(Q::KEYB);
  Pkt.Src(0);                          // From root process....
  Pkt.Send(0);                         // Send to root process main thread
                                       // User wants out - kill this thread
  if (strcmp(buf,"exit")==0) break;    // "exit" typed
  if (buf[0]==char(0)) break;          // ctrl-d in linux-land
  if (buf[0]==char(4)) break;          // ctrl-d in u$oft-land
}
pthread_exit(NULL);                    // Kill the keyboard thread
return NULL;
}

//==============================================================================

Root::Root(int argc,char * argv[],string d) :
  OrchBase(argc,argv,d,string(__FILE__))
{
CmCall = new CmCall_t(this);
CmRTCL = new CmRTCL_t(this);
CmSyst = new CmSyst_t(this);
CmTest = new CmTest_t(this);

injData.flag = 0;                      // Clear injector controls

FnMapx.push_back(new FnMap_t);    // create a new event map in the derived class
                                       // Load the default incoming event map
(*FnMapx[0])[PMsg_p::KEY(Q::KEYB         )] = &Root::OnKeyb;
(*FnMapx[0])[PMsg_p::KEY(Q::TEST         )] = &Root::OnTest;
(*FnMapx[0])[PMsg_p::KEY(Q::LOG  ,Q::FULL)] = &Root::OnLogP;
(*FnMapx[0])[PMsg_p::KEY(Q::INJCT,Q::REQ )] = &Root::OnInje;

void * args = this;                    // Spin off a thread to handle keyboard
pthread_t kb_thread;
if(pthread_create(&kb_thread,NULL,kb_func,args))
  fprintf(stdout,"Error creating kb_thread\n");
fflush(stdout);

MPISpinner();                          // Spin on *all* messages; exit on DIE
//printf("********* Root rank %d on the way out\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

Root::~Root()
{
delete CmCall;
delete CmRTCL;
delete CmSyst;
delete CmTest;
WALKVECTOR(FnMap_t*, FnMapx, F) delete *F;
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
                                       // Coming from batch?
if (!(CmCall->stack).empty()) {
  Post(35);
  return 0;
}
int pL, tpL, lIdx;
unsigned p;
pL = -1;                               // maybe we have no LogServer?
PMsg_p Pkt;                            // Burst closedown command to all ranks
Pkt.Src(Urank);
Pkt.Key(Q::EXIT);
for (unsigned cIdx=0;cIdx < Comms.size();cIdx++)
{
Pkt.comm = Comms[cIdx];                // on all communicators
if ((tpL = pPmap[cIdx]->U.LogServer) != Q::NAP) // is this the LogServer's local comm?
{
   pL = tpL;    // Yes. Cache LogServer rank
   lIdx = cIdx; // and comm index
   for(p=0;p<Usize[cIdx];p++)
   {
      if (((((int)cIdx != RootCIdx()) || ((int)p!=Urank))) && ((int)p!=tpL)) {  // NOT the LogServer
         Post(50,pPmap[cIdx]->M[p],int2str(p));
         Pkt.Send(p);
      }
   }
}
else
{
   for(p=0;p<Usize[cIdx];p++) // No. LogServer not on this comm. Shut everyone down.
   {
      if ((((int)cIdx != RootCIdx()) || ((int)p!=Urank))) {  // Don't need to send to self
         Post(50,pPmap[cIdx]->M[p],int2str(p));
         Pkt.Send(p);
      }
   }
}
}
Post(50,Sderived,int2str(Urank));            // Root is going anyway
if (pL >= 0)                                 // a Logserver existed?
{
Post(50,pPmap[lIdx]->M[pL],int2str(pL));     // Ensure LogServer goes last
Pkt.comm = Comms[lIdx];                      // Set LogServer's comm for the packet
Pkt.Send(pL);
}
return CommonBase::OnExit(&Pkt,0);           // Run the base class exit handler
// return 1;                                 // Return closedown flag
}

//------------------------------------------------------------------------------

unsigned Root::CmInje(Cli * pC)
// Handle "inject" from the monkey. The command is wrapped in a message and
// forwarded to the Injector process, but we have a quick scan through it first
// to see if there's a "FLAG" clause for the root.
{
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clauses, looking for "flag"
  if (strcmp((*i).Cl.c_str(),"flag")!=0) continue;
  WALKVECTOR(Cli::Pa_t,(*i).Pa_v,j)
    if (!(*j).Val.empty()) injData.flag = str2hex((*j).Val);
}
Post(55,hex2str(injData.flag));        // Tell the monkey
PMsg_p Msg;
unsigned cIdx = 0;
for (; cIdx < Comms.size(); cIdx++)
{
    if (pPmap[cIdx]->U.Injector != Q::NAP)
    {
       Msg.comm = Comms[cIdx];
       break;
    }
}
if (cIdx >= Comms.size())
{
   Post(160);
   return 0;                           // no injector, but should probably return normally
}
Msg.Put(1,&(pC->Orig));                // Wrap the command into a message
Msg.Key(Q::INJCT,Q::FLAG);             // Message type
Msg.Send(pPmap[cIdx]->U.Injector);     // Send it to the injector process
return 0;                              // Legitimate command exit
}

//------------------------------------------------------------------------------

unsigned Root::Connect(string svc)
{
if (unsigned err = CommonBase::Connect(svc)) return err;
FnMapx.push_back(new FnMap_t); // insert another function table

// Root doesn't have to sit in the same local universe as the LogServer,
// nor any injectors (which might conveniently be remote in many cases so
// that they can grab data from an external source). So these functions ought
// to be registered for new connections. Keyboard and test functionality, though
// will always be in the same universe and needn't be re-registered.
(*FnMapx[FnMapx.size()-1])[PMsg_p::KEY(Q::LOG  ,Q::FULL)] = &Root::OnLogP;
(*FnMapx[FnMapx.size()-1])[PMsg_p::KEY(Q::INJCT,Q::REQ )] = &Root::OnInje;
return MPI_SUCCESS;
}

//------------------------------------------------------------------------------

void Root::Dump(FILE * fp)
{
fprintf(fp,"Root dump+++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"Event handler table:\n");
unsigned cIdx = 0;
WALKVECTOR(FnMap_t*,FnMapx,F) {
  fprintf(fp,"Function table for comm %d:\n", cIdx++);
  fprintf(fp,"Key        Method\n");
  // tricky: dereference of F needs enclosing in parentheses
  // because the expansion of the macro plus operator precedence
  // means it would try to dereference the iterator obtained from
  // F. Not what is expected...
  WALKMAP(unsigned,pMeth,(**F),i)
    fprintf(fp,"%#010x 0x%#016x\n",(*i).first,(*i).second);
}
fprintf(fp,"prompt    = %s\n",prompt);
fprintf(fp,"Injector controls:\n");
fprintf(fp,"flag = %0x\n",injData.flag);
CmCall->Dump(fp);
CmRTCL->Dump(fp);
CmSyst->Dump(fp);
CmTest->Dump(fp);
fprintf(fp,"Root dump-----------------------------------\n");
fflush(fp);
CommonBase::Dump(fp);
}

//------------------------------------------------------------------------------

void Root::OnIdle()
{
Cli Cm = CmCall->Front();              // Anything in the batch queue?
if (Cm.Empty()) return;                // No - bail
                                       // EOF marker? - remove from call stack
if (Cm.Co[0]=='*') CmCall->stack.pop_back();
else ProcCmnd(&Cm);                    // Handle ordinary batch command
}

//------------------------------------------------------------------------------

unsigned Root::OnInje(PMsg_p * Z, unsigned cIdx)
// Handle a message coming in from the Injector.
{
string s;
Z->Get(1,s);                           // Unpack command string
Cli cmnd(s);                           // Rebuild command
Post(56,s);                            // Tell everyone
return ProcCmnd(&cmnd);                // Inject it into the command processor
}

//------------------------------------------------------------------------------

unsigned Root::OnKeyb(PMsg_p * Z, unsigned cIdx)
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

unsigned Root::OnLogP(PMsg_p * Z, unsigned cIdx)
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

unsigned Root::OnTest(PMsg_p * Z, unsigned cIdx)
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
if (scmnd[0]=='*') {                   // Special case - EOF marker
  CmCall->stack.pop_back();            // Remove from call stack
  return 0;
}
// Note you can't use .size() here, because some strings have been resized to 4,
// so for them the size includes the '\0', whereas the others, it doesn't.....
// Commands are handled in two ways:
// The more complicated ones have their own class, with a splitter functor
// (operator()()) to decode the clauses, whereas the simple ones have a
// simple method.
fd = CmPath->Fopen();                  // Sort out the detail file stream
const unsigned INVALIDCODE=999;        // Yukky yukky yuk yuk
unsigned code = INVALIDCODE;
if (strcmp(scmnd.c_str(),"call")==0) code = (*CmCall)(pC);
if (strcmp(scmnd.c_str(),"exit")==0) code = CmExit(pC);
if (strcmp(scmnd.c_str(),"grap")==0) code = (*CmGrph)(pC);
if (strcmp(scmnd.c_str(),"inje")==0) code = CmInje(pC);
if (strcmp(scmnd.c_str(),"path")==0) code = (*CmPath)(pC);
if (strcmp(scmnd.c_str(),"rtcl")==0) code = (*CmRTCL)(pC);
if (strcmp(scmnd.c_str(),"syst")==0) code = (*CmSyst)(pC);
if (strcmp(scmnd.c_str(),"test")==0) code = (*CmTest)(pC);
if (scmnd.at(0)==char(0)) code = CmExit(pC); // Ctrl-D behaviour in linux-land
if (scmnd.at(0)==char(4)) code = CmExit(pC); // Ctrl-D behaviour in Windoze
fd = CmPath->Fclose();                 // Reset detail file stream
if (code==INVALIDCODE) return CmDrop(pC);
return code;
}

//------------------------------------------------------------------------------

void Root::Prompt(FILE * fp)
{
if (fp!=stdout) return;
fprintf(fp,"%s",Root::prompt);     // Console prompt
fflush(fp);
}

//==============================================================================

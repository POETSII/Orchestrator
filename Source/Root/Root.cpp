//------------------------------------------------------------------------------

#include "Root.h"
#include "CommonBase.h"
#include "pthread.h"
#include <stdio.h>
#include "PMsg_p.hpp"
#include "Pglobals.h"
#include "Cli.h"
#include "flat.h"

#include "OSFixes.hpp"

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
for(;;) {
  if (len==0) Root::Prompt();          // Console prompt
  fflush(stdout);
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

//int cnt;                               // cnt now includes trailing '\0'
//char * obuf = Pkt.Get<char>(1,cnt);
//printf("len=%d, cnt=%d, obuf=%s\n",len,cnt,obuf);  fflush(stdout);

  Pkt.Src(0);
  Pkt.Send(0);                         // Send to root process main thread

  // User wants out - kill this thread
  if (strcmp(buf,"exit")==0 or buf[0]==0) break;
}

//printf("kb_func: thread closing\n"); fflush(stdout);
pthread_exit(NULL);
return NULL;
}

//==============================================================================

Root::Root(int argc,char * argv[],string d)
    :OrchBase(argc,argv,d,string(__FILE__))
{
echo         = false;                  // Batch subsystem opaque
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

/* Handle input arguments - grab the hdfPath and/or batchPath. */
std::string rawArgs;
std::string hdfPath;
std::string batchPath;
for (int i=1; i<argc; i++)
{
  rawArgs += argv[i];
  rawArgs += " ";
}

Cli cli(rawArgs);
if (cli.problem.prob)
{
  printf("Command-line error near character %d. Ignoring commandline "
         "arguments.\n", cli.problem.col);
}
else
{
  WALKVECTOR(Cli::Cl_t, cli.Cl_v, i)
  {
    std::string key = i->Cl;
    if (key=="batch") batchPath = i->GetP(0);
    if (key=="hdf") hdfPath = i->GetP(0);
  }
}

/* Queue batch message, if one was given to us. We do this by staging a Cli
 * entry using the batch system (see Root::OnIdle). */
if (!batchPath.empty())
{
  Equeue.push_front(Cli(dformat("call /file = \"%s\"", batchPath.c_str())));
}

/* Pass hardware description file to topology generation, if one was given to
 * us. We do this by staging a Cli entry using the batch system (see
 * Root::OnIdle).
 *
 * The reason we do this (as opposed to simply calling TopoLoad) is because we
 * haven't built the process map yet - as a consequence, we do not know the
 * rank of the LogServer processes, and so can't Post in the event of an error.
 * By staging the Cli entry onto the front of the batch queue, we can be sure
 * that MPISpinner will drain the input buffer before running our
 * command. Since that input buffer will contain messages that will register
 * all processes into our pPmap, we know we will have the logserver rank,
 * making Post work correctly. */
if (!hdfPath.empty())
{
  Equeue.push_front(Cli(dformat("topo /load = \"%s\"", hdfPath.c_str())));
}

MPISpinner();                          // Spin on *all* messages; exit on DIE
printf("********* Root rank %d on the way out\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

Root::~Root()
{
//printf("********* Root rank %d destructor\n",Urank); fflush(stdout);
WALKVECTOR(FnMap_t*, FnMapx, F)
    delete *F;
}

//------------------------------------------------------------------------------

void Root::CallEcho(Cli::Cl_t Cl)
// Monkey wants to see what's going on in the batch subsystem
{
string secho = Cl.GetP();
if (strcmp(secho.c_str(),"on" )==0) echo = true;
if (strcmp(secho.c_str(),"off")==0) echo = false;
Post(34,echo?"on":"off");
}

//------------------------------------------------------------------------------

void Root::CallFile(Cli::Cl_t Cl)
// Switch input from monkey to file
{
string sfile = Cl.GetP();              // Get the batch filename
if (find(stack.begin(),stack.end(),sfile)!=stack.end()) {
  Equeue.pop_front();                  // EOF marker in command Q now NFG
  Post(33,sfile);                      // Trap attempted batch recursion
  return;
}
if (sfile.empty()) {                   // Blank batch filename
  Post(31);
  return;
}
if (!file_readable(sfile.c_str())) {
  Post(32,sfile);
  return;
}

Cli Cx;
Cx.Co = "*";
Cx.Dump();
Equeue.push_front(Cx);
list<Cli> temp = Cli::File(sfile,true);// Bolt incoming file to front of queue
Equeue.splice(Equeue.begin(),temp);
stack.push_back(sfile);                // Store current batch file name

/*
FILE * fp = fopen(sfile.c_str(),"r");  // Try and open it?
if (fp==0) Post(32,sfile);             // Nope, for whatever reason
else {                                 // OK, at last, we're good to go
  stack.push_back(sfile);              // Store current batch file name
  static const unsigned SIZE = 512;
  for(;;) {                            // One line at a time....
    char buf[SIZE];
    char * ps = fgets(buf,SIZE-1,fp);  // Pull in the data; ends with "/n/0"
    if (ps!=0) {                       // Process it ?
      if (echo) Post(28,string(ps));   // Copy to monkey
      ProcCmnd(&Cli(string(ps)));      // Action it
    }
    else break;                        // EOF?
  }
  stack.pop_back();                    // Junk current stack frame
  fclose(fp);                          // Close current file
}
*/
}

//------------------------------------------------------------------------------

void Root::CallShow(Cli::Cl_t Cl)
// Monkey wants to see the call stack
{
FILE * fp = stdout;
fprintf(fp,"Batch call stack has %lu entries\n",stack.size());
WALKVECTOR(string,stack,i) fprintf(fp,"%s\n",(*i).c_str());
fprintf(fp,"Batch command queue has %lu entries\n",Equeue.size());
WALKLIST(Cli,Equeue,i) fprintf(fp,"%s\n",(*i).Orig.c_str());
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned Root::CmCall(Cli * pC)
// Pass command input to a file
{
if (pC==0) return 0;                   // Paranoia - nothing to call
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string scl = (*i).Cl;                // Pull out clause name
  if (strcmp(scl.c_str(),"echo")==0) { CallEcho(*i); continue; }
  if (strcmp(scl.c_str(),"file")==0) { CallFile(*i); continue; }
  if (strcmp(scl.c_str(),"show")==0) { CallShow(*i); continue; }
  Post(25,scl,"call");                 // Unrecognised clause
}
return 0;
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
if (!stack.empty()) {
  Post(35);
  return 0;
}
int p, pL, tpL, lIdx;
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
      if ((((static_cast<int>(cIdx) != RootCIdx())
            || (p!=static_cast<int>(Urank)))) && (p!=tpL))
      { // NOT the LogServer
        Post(50,pPmap[cIdx]->M[p],int2str(p));
        Pkt.Send(p);
      }
   }
}
else
{
   for(p=0;p<Usize[cIdx];p++) // No. LogServer not on this comm. Shut everyone down.
   {
      if (((static_cast<int>(cIdx) != RootCIdx())
            || (p!=static_cast<int>(Urank))))
      {  // Don't need to send to self
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

unsigned Root::CmRTCL(Cli * pC)
// Handle "RTCL" from the monkey. We've already parsed the command line, so we
// know it's OK. Rather than pack/unpack the data structure, just send the
// original command line string and the RTC can re-parse it - it's never very
// long. It's not like we're paying for the electricity.
{
if (pC==0) return 0;                   // Paranoia
//pC->Dump();
PMsg_p Msg;
unsigned cIdx = 0;
for (; cIdx < Comms.size(); cIdx++)
{
    if (pPmap[cIdx]->U.RTCL != Q::NAP)
    {
       Msg.comm = Comms[cIdx];
       break;
    }
}
if (cIdx >= Comms.size())
{
   Post(161);
   return 0;                           // no RTCL, but should probably return normally
}
Msg.Put(1,&(pC->Orig));                // Command line
Msg.Src(Urank);                        // Me
Msg.Key(Q::RTCL);                      // Key
Msg.Send(pPmap[cIdx]->U.RTCL);               // Away we go
return 0;                              // Legitimate command exit
}

//------------------------------------------------------------------------------

unsigned Root::CmSyst(Cli * pC)
// Handle "system" from the monkey.
{
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string scl = (*i).Cl;                // Pull out clause name
  if (strcmp(scl.c_str(),"conn")==0) { SystConn(*i); continue; }
  if (strcmp(scl.c_str(),"path")==0) { SystPath(*i); continue; }
  if (strcmp(scl.c_str(),"ping")==0) { SystPing(*i); continue; }
  if (strcmp(scl.c_str(), "run")==0) { SystRun(*i);  continue; }
  if (strcmp(scl.c_str(),"show")==0) { SystShow(*i); continue; }
  if (strcmp(scl.c_str(),"time")==0) { SystTime(*i); continue; }
  Post(25,scl,"system");               // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//------------------------------------------------------------------------------

unsigned Root::CmTest(Cli * pC)
// Test command handler. (It's already been trimmed)
{
if (pC->Cl_v.empty()) return 0;        // Nothing to do
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {
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
    WALKVECTOR(MPI_Comm,Comms,C)      // flood to all active comms.
    {
       Z.comm = *C;
       Z.Bcast();
    }
    Post(51,uint2str(width),uint2str(level));
  }
}
return 0;
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
WALKVECTOR(FnMap_t*,FnMapx,F)
{
fprintf(fp,"Function table for comm %d:\n", cIdx++);
fprintf(fp,"Key        Method\n");
// tricky: dereference of F needs enclosing in parentheses
// because the expansion of the macro plus operator precedence
// means it would try to dereference the iterator obtained from
// F. Not what is expected...
WALKMAP(unsigned,pMeth,(**F),i)
{
  //fprintf(fp,"%#010x 0x%#016x\n",(*i).first,(*i).second);
  fprintf(fp,"%#010x ",(*i).first);

  // Now for a horrible double type cast to get us a sensible function pointer.
  // void*s are only meant to point to objects, not functions. So we get to a
  // void** as a pointer to a function pointer is an object pointer. We can then
  // follow this pointer to get to the void*, which we then reinterpret to get
  // the function's address as a uint64_t.
  fprintf(fp,"%" PTR_FMT "\n",reinterpret_cast<uint64_t>(
                                *(reinterpret_cast<void**>(&((*i).second))))
          );
}
}
fprintf(fp,"prompt    = %s\n",prompt);

fprintf(fp,"Batch file echo %s\n",echo ? "ON" : "OFF");
fprintf(fp,"Injector controls:\n");
fprintf(fp,"flag = %0x\n",injData.flag);
fprintf(fp,"Root dump-----------------------------------\n");
fflush(fp);
CommonBase::Dump(fp);
}

//------------------------------------------------------------------------------

void Root::OnIdle()
{
if (Equeue.empty()) return;            // Batch queue empty?
Cli Cm = Equeue.front();               // Pull off the next one
Equeue.pop_front();                    // Erase from qeueue
if (Cm.Co[0]=='*') stack.pop_back();   // EOF marker? - remove from call stack
else ProcCmnd(&Cm);                    // Handle ordinary batch command
//  return;                              // Go round again
//}
//if (IsCallFile(Cm)) {                  // Special case: Is this call /file=????
//Cm.Dump();
//  CallFile(Cm.Cl_v[0]);
//  return;
//}
//ProcCmnd(&Cm);                         // Handle ordinary batch command
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
  stack.pop_back();                    // Remove from call stack
  return 0;
}
// Note you can't use .size() here, because some strings have been resized to 4,
// so for them the size includes the '\0', whereas the others, it doesn't.....
if (strcmp(scmnd.c_str(),"call")==0) return CmCall(pC);
if (strcmp(scmnd.c_str(),"exit")==0) return CmExit(pC);
if (strcmp(scmnd.c_str(),"inje")==0) return CmInje(pC);
if (strcmp(scmnd.c_str(),"link")==0) return CmLink(pC);
if (strcmp(scmnd.c_str(),"owne")==0) return CmOwne(pC);
if (strcmp(scmnd.c_str(),"plac")==0) return CmPlace(pC);
if (strcmp(scmnd.c_str(),"rtcl")==0) return CmRTCL(pC);
if (strcmp(scmnd.c_str(),"syst")==0) return CmSyst(pC);
if (strcmp(scmnd.c_str(),"task")==0) return CmTask(pC);
if (strcmp(scmnd.c_str(),"test")==0) return CmTest(pC);
if (strcmp(scmnd.c_str(),"topo")==0) return CmTopo(pC);

// Handle Ctrl-D behaviour in common shells. Ctrl-D repeatedly sends the EOF
// character. Check only the first character of the input.
if (scmnd.at(0)==0) return CmExit(pC);

return CmDrop(pC);
}

//------------------------------------------------------------------------------

void Root::Prompt(FILE * fp)
{
if (fp!=stdout) return;
fprintf(fp,"%s",Root::prompt);     // Console prompt
fflush(fp);
}

//------------------------------------------------------------------------------

void Root::SystConn(Cli::Cl_t Cl)
// Connect to a group of processes identified by a service name in the parameter
{
if (Cl.Pa_v.size()==0) {
  Post(47,"conn","system","1"); // need to be given a service to connect to
  return;
}
if (static_cast<int>(pPmap[0]->vPmap.size()) != Usize[0])
{
  Post(62); // can't connect to another universe before we know the size of our own.
}
string svc = Cl.Pa_v[0].Val; // get the service name
// connection is collective, so we have to organise everyone
WALKVECTOR(ProcMap::ProcMap_t,pPmap[0]->vPmap,j) {   // Walk the local process list
  if (Cli::StrEq((*j).P_class,Sderived)) continue;   // No need to notify self
  PMsg_p Pkt;
  Pkt.comm = Comms[0];           // local comm
  Pkt.Put(0,&svc);               // Target service name
  Pkt.Src(Urank);                // Sending rank
  Pkt.Key(Q::SYST,Q::CONN);      // It's a connection request
  Pkt.Send((*j).P_rank);
}
if (Connect(svc))
   Post(60,svc.c_str());
}
//------------------------------------------------------------------------------
void Root::SystPath(Cli::Cl_t Cl)
// Add a pathname to the list of to-be-appended paths
{

}

//------------------------------------------------------------------------------

void Root::SystPing(Cli::Cl_t Cl)
//
{
if (Cl.Pa_v.size()==0) {
  Post(48,"ping","system","1");
  return;
}
for(unsigned k=0;k<4;k++) {
//Post(27,uint2str(k));
  WALKVECTOR(Cli::Pa_t,Cl.Pa_v,i) {  // Walk the parameter (ping) list
//    printf("%s\n",(*i).Val.c_str()); fflush(stdout);
    string tgt = (*i).Val;             // Class name to be pinged
    if (Cli::StrEq(tgt,Sderived)) continue;           // Can't ping yourself
    unsigned cIdx = 0;
    for (; cIdx < Comms.size(); cIdx++)               // check all communicators
    {
       WALKVECTOR(ProcMap::ProcMap_t,pPmap[cIdx]->vPmap,j) {   // Walk the process list
         if (Cli::StrEq((*j).P_class,Sderived)) continue;// Still can't ping self
         if ((Cli::StrEq(tgt,(*j).P_class))||(tgt=="*")) {
           PMsg_p Pkt;
           Pkt.comm = Comms[cIdx];
           Pkt.Put(1,&((*j).P_class));    // Target process name
           string tD(GetDate());
           string tT(GetTime());
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
}
}

//------------------------------------------------------------------------------

void Root::SystRun(Cli::Cl_t Cl)
// Spawn a group of processes and connect to them. The run command should look like
// either: 1) a list of triplets: number of procs, hostname, executable; or 2) a
// file specification containing lines of this form. For this first version we
// are not allowing command-line arguments, although this capacity could be
// retrofitted if deemed essential.
{
// no arguments: bad command
if (Cl.Pa_v.size()==0) {
  Post(48,"run","system","1");
  return;
}
// 1 argument: a file
if (Cl.Pa_v.size()==1) {

  return;
}
if (Cl.Pa_v.size()==2) {
  Post(48,"run","system","1");
  return;
}
if (Cl.Pa_v.size()%3!=0) {
  Post(48,"ping","system","1");
  return;
}
}

//------------------------------------------------------------------------------

void Root::SystShow(Cli::Cl_t)
// Monkey wants the list of processes
{
//vector<string> vprocs;
//if (pPmap!=0) pPmap->GetProcs(vprocs);
vector<ProcMap::ProcMap_t> vprocs;
for (unsigned cIdx = 0; cIdx < Comms.size(); cIdx++)
{
if (pPmap[cIdx]!=0) pPmap[cIdx]->GetProcs(vprocs);
Post(29,uint2str(vprocs.size()),uint2str(cIdx));
Post(30,Sproc);
printf("\n");
printf("Processes for comm %d\n", cIdx);
//WALKVECTOR(string,vprocs,i) printf("%s\n",(*i).c_str());
WALKVECTOR(ProcMap::ProcMap_t,vprocs,i)
  printf("Rank %02d, %35s, created %s %s\n",
      (*i).P_rank,(*i).P_class.c_str(),(*i).P_TIME.c_str(),(*i).P_DATE.c_str());
printf("\n");
}
fflush(stdout);
}

//------------------------------------------------------------------------------

void Root::SystTime(Cli::Cl_t)
// Monkey wants the time
{
string sT = GetTime();
string sD = GetDate();
Post(26,sD,sT);
}

//==============================================================================

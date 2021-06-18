//------------------------------------------------------------------------------

#include "Root.h"
#include "CommonBase.h"
#include "OrchConfig.h"
#include "pthread.h"
#include <stdio.h>
#include "PMsg_p.hpp"
#include "Pglobals.h"
#include "Cli.h"
#include "flat.h"
#include <new>

#include "OSFixes.hpp"
//==============================================================================

const char * Root::prompt = "POETS>";
bool         Root::promptOn = true;

//------------------------------------------------------------------------------

void * kb_func(void * pPar)
// Thread function to listen to the keyboard, package up each line in a message
// and send it to the main thread in this process, where it gets picked up by
// the MPI spinner.
{
int len = 0;                           // Characters in buffer
for(;;) {                              // Superloop
  if (len==0) Root::Prompt();          // Console prompt
  static const unsigned SIZE = 512;
  char buf[SIZE];
  buf[0] = '\0';                       // Borland bug: notes 21/7/17
  for(unsigned j=1;j<SIZE;j++) buf[j]='x';
  if (fgets(buf,SIZE-1,stdin) == PNULL) continue; // Pull in keyboard string.
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
                                       // User wants out - kill this thread
  if (strcmp(buf,"exit")==0) break;    // "exit" typed
  if (buf[0]==char(0)) break;          // ctrl-d in linux-land
  if (buf[0]==char(4)) break;          // ctrl-d in u$oft-land
}
// Tell the user we're leaving immediately.
Root::promptOn = false;
printf("Exiting...\n");
pthread_exit(NULL);                    // Kill the keyboard thread
return NULL;
}

//==============================================================================

Root::Root(int argc,char * argv[],string d) :
  OrchBase(argc,argv,d,string(__FILE__))
{
if (!Config()) return;                 // Set up defaults from config file
injData.flag = 0;                      // Clear injector controls
                                       // Load the default incoming event map
FnMap[PMsg_p::KEY(Q::KEYB         )] = &Root::OnKeyb;
FnMap[PMsg_p::KEY(Q::TEST         )] = &Root::OnTest;
FnMap[PMsg_p::KEY(Q::LOG  ,Q::FULL)] = &Root::OnLogP;
FnMap[PMsg_p::KEY(Q::INJCT,Q::REQ )] = &Root::OnInje;
FnMap[PMsg_p::KEY(Q::PMAP         )] = &Root::OnPmap;

// Mothership acknowledgements
FnMap[PMsg_p::KEY(Q::MSHP, Q::ACK, Q::DEFD)] = &Root::OnMshipAck;
FnMap[PMsg_p::KEY(Q::MSHP, Q::ACK, Q::LOAD)] = &Root::OnMshipAck;
FnMap[PMsg_p::KEY(Q::MSHP, Q::ACK, Q::RUN )] = &Root::OnMshipAck;
FnMap[PMsg_p::KEY(Q::MSHP, Q::ACK, Q::STOP)] = &Root::OnMshipAck;

// Mothership requests
FnMap[PMsg_p::KEY(Q::MSHP, Q::REQ, Q::STOP)] = &Root::OnMshipReq;
FnMap[PMsg_p::KEY(Q::MSHP, Q::REQ, Q::BRKN)] = &Root::OnMshipReq;

// Spin off a thread to handle keyboard
void * args = this;
pthread_t kb_thread;
if(pthread_create(&kb_thread,NULL,kb_func,args))
  fprintf(stdout,"Error creating kb_thread\n");
fflush(stdout);

/* Grab default hdfPath from configuration. */
std::string hdfPath = pOC->Hardware();

/* Handle input arguments - grab the hdfPath and/or batchPath (the former
 * clobbers a definition from config). */
std::string rawArgs;
std::string batchPath;
for (int i=1; i<argc; i++)
{
  rawArgs += argv[i];
  rawArgs += " ";
}

Cli cli(rawArgs);
int row, column;
cli.Err(row, column);
if (row != -1)
{
  printf("Command-line error near character '%d'. Ignoring non-debug "
         "commandline arguments.\n", cli.problem.col);
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
  pCmCall->Equeue.push_front(Cli(dformat(
    "call /file = \"%s\"", batchPath.c_str())));
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
  pCmCall->Equeue.push_front(Cli(dformat(
    "load /engine = \"%s\"", hdfPath.c_str())));
}

MPISpinner();                          // Spin on *all* messages; exit on DIE
pthread_join(kb_thread, NULL);
}

//------------------------------------------------------------------------------

Root::~Root()
{
if (pOC!=0) delete pOC;
}

//------------------------------------------------------------------------------

bool Root::Config()
// Here we load the non-user facing defaults from a static configuration file.
// This file has a fixed name, and is resident in the same subdirectory as the
// Root binary. From this, all other file locations, paths and defaults are
// derived. There is a minimal bit of saving-us-from-cockups, but not much,
// because if this doesn't work the LogServer has no elaboration file, and we
// can't load applications or do anything, really.
{
pOC = new OrchConfig();                // Build the configuration object
if (pOC->ErrCnt()!=0) {                // Errors: do what we can for user f/b
  vector<pair<int,int> > ev = pOC->ExposeErr();
  printf("\n%u Orchestrator configuration file %s errors\n\n",
         pOC->ErrCnt(),pOC->Where().c_str());
  for(unsigned i=0;i<ev.size();i++)
    printf("%u: line %d code %d\n",i+1,ev[i].first,ev[i].second);
  printf("*** *Attempting* a graceful closedown\n\n");
  fflush(stdout);
  CmExit(0);                           // Close down the rest of the Universe
  return false;
}
                                       // OK, we're good
pCmPath->Reset();                      // Copy over the default paths



// TODO : copy over all the other defaults
Post(63,"TODO: Copy defaults out of Root::Config");

return true;
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

                                       // Coming from batch?
if (!pCmCall->stack.empty()) {
  Post(35);
  return 0;
}

PMsg_p Pkt;                            // Burst closedown command to all ranks
Pkt.Src(Urank);
Pkt.Key(Q::EXIT);
int pL = pPmap->U.LogServer;           // Cache LogServer rank
for(int p=0;p<Usize;p++)
  if ((p!=(int)Urank)&&(p!=pL)) {      // NOT the LogServer
    Post(50,pPmap->M[p],int2str(p));
    Pkt.Send(p);
  }
Post(50,pPmap->M[pL],int2str(pL));     // Ensure LogServer goes last
Post(50,Sderived,int2str(Urank));      // Root is going anyway
Pkt.Send(pL);

return CommonBase::OnExit(&Pkt);           // Run the base class exit handler
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
    if (!(*j).Va_v.empty()) injData.flag = str2hex((*j).Va_v[0]);
}
Post(55,hex2str(injData.flag));        // Tell the monkey
PMsg_p Msg;
Msg.Put(1,&(pC->Orig));                // Wrap the command into a message
Msg.Key(Q::INJCT,Q::FLAG);             // Message type
Msg.Send(pPmap->U.Injector);     // Send it to the injector process
return 0;                              // Legitimate command exit
}

//------------------------------------------------------------------------------

unsigned Root::CmRetu(Cli * pC)
// Handle a "return" command. (No effect from the console.) From inside a batch
// command, we skip the rest of the calling file.
// It has one clause, (which is the line number that generated it) so we don't
// need to loop - just grab the first one, and the first parameter is the line.
{
if (pCmCall->stack.empty()) return 0;  // We're in console mode
                                       // Tell the user what's going on
Post(38,pCmCall->stack.back(),pC->Cl_v[0].GetP());
Cli c = pCmCall->Equeue.front();       // Next command in batch queue
while (c.Co[0]!='*') {                 // While not EOF marker ("*")....
  pCmCall->Equeue.pop_front();         // Lose it
  c = pCmCall->Equeue.front();         // New next command
}
pCmCall->Equeue.pop_front();           // Lose the "*"
pCmCall->stack.pop_back();             // Modify the recursion trap stack
return 0;
}

//------------------------------------------------------------------------------

void Root::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n%sRoot dump ++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sEvent handler table:\n",os);
fprintf(fp,"%sKey        Method\n",os);
WALKMAP(unsigned,pMeth,FnMap,i)
{
  fprintf(fp,"%s%#010x %" PTR_FMT "\n",os,(*i).first,
                        OSFixes::getAddrAsUint((*i).second));
}
fprintf(fp,"%sprompt    = %s\n",os,prompt);
fprintf(fp,"%sInjector controls:\n",os);
fprintf(fp,"%sflag = %0x\n",os,injData.flag);
OrchBase::Dump(off+2,fp);
fprintf(fp,"%sRoot dump ----------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void Root::OnIdle()
{
Cli Cm = pCmCall->Front();             // Anything in the batch queue?
if (Cm.Empty()) return;                // No - bail                                      // Command comment echo to logserver?

if ((pCmCall->echo)&&(!Cm.Orig.empty())) {
  Post(22);
  Post(36,Cm.Orig);
}
                                       // EOF marker? - remove from call stack
if (Cm.Co[0]=='*') pCmCall->stack.pop_back();
else ProcCmnd(&Cm);                    // Handle ordinary batch command
}

//------------------------------------------------------------------------------

unsigned Root::OnInje(PMsg_p * Z)
// Handle a message coming in from the Injector.
{
string s;
Z->Get(1,s);                           // Unpack command string
Cli cmnd(s);                           // Rebuild command
Post(56,s);                            // Tell everyone
return ProcCmnd(&cmnd);                // Inject it into the command processor
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
Pc.Err(l,c);
if (l>=0) {                            // Handle any syntax errors locally
  string el = string(strlen(prompt),' ');
  el += string(c-1,' ') + "^^^";
  printf("%s\nCommand line syntax error - line ignored by POETS\n",el.c_str());
  /* This really requires more thought... errors, stdout, stderr, piping...
   * I found this because I compiled under gcc 9.3.0, which raises a warning.
  printf("...trying OS command shell:\n\n");
  system(buf);
  printf("\n"); */
  Prompt();                            // Console prompt
  return 0;                            // Bail with "not closedown" value
}
Post(23,buf);                          // Copy to logfile
unsigned ret = ProcCmnd(&Pc);          // Try and make sense of it
Prompt();                              // Console prompt
return ret;
}

//------------------------------------------------------------------------------

unsigned Root::OnMshipAck(PMsg_p * Z)
// Updates the mothership acknowledgement table. The table is primarily
// intended as a debugging and logging tool, but will be invaluable for
// orchestrating POETS jobs over multiple boxes.
{
    map<int,string>::iterator ackIt;
    unsigned key = Z->Key();
    bool acksMatch = false;
    string appName;
    string ackName;
    Z->Get(0, appName);

    // Figure out what the acknowledgement is.
    if (key == PMsg_p::KEY(Q::MSHP, Q::ACK, Q::DEFD)) ackName = "DEFINED";
    else if (key == PMsg_p::KEY(Q::MSHP, Q::ACK, Q::LOAD)) ackName = "READY";
    else if (key == PMsg_p::KEY(Q::MSHP, Q::ACK, Q::RUN)) ackName = "RUNNING";
    else if (key == PMsg_p::KEY(Q::MSHP, Q::ACK, Q::STOP)) ackName = "STOPPED";
    else
    {
        Post(183, hex2str(key), int2str(Z->Src()), int2str(Z->Tgt()));
        return 0;
    }

    // Store it
    mshipAcks[appName][Z->Src()] = ackName;

    // If all Motherships have changed state correctly, post about it.
    for (ackIt = mshipAcks[appName].begin(); ackIt != mshipAcks[appName].end();
         ackIt++)
    {
        if (ackIt->second != ackName)
        {
            acksMatch = false;
            break;
        }
    }
    if (acksMatch)
    {
        if (ackName == "DEFINED") Post(186, appName, "successfully deployed");
        if (ackName == "READY") Post(186, appName, "ready to start");
        if (ackName == "RUNNING") Post(186, appName, "running");
        if (ackName == "STOPPED") Post(186, appName, "stopped");
    }

    return 0;
}

//------------------------------------------------------------------------------

unsigned Root::OnMshipReq(PMsg_p * Z)
// Handles requests from Motherships to central control. Stops applications and
// manages error propagation.
{
    unsigned key = Z->Key();
    string appName;
    Z->Get(0, appName);
    if (key == PMsg_p::KEY(Q::MSHP, Q::REQ, Q::STOP))
    {
        MshipCommand(Cli("stop /app = " + appName).Cl_v[0], "stop");
    }
    else if (key == PMsg_p::KEY(Q::MSHP, Q::REQ, Q::BRKN))
    {
        deplStat[appName] == "ERROR";
    }
    else Post(183, hex2str(key), int2str(Z->Src()), int2str(Z->Tgt()));
    return 0;
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

unsigned Root::OnPmap(PMsg_p * Z)
// Here we intercept INCOMING messages loading the process map. Firstly, we
// hand the message directly down to CommonBase to load the process map.
// If the message was NOT from the LogServer, that's it.
// If it was, then we now know that our process map knows about the LogServer,
// so we can send it (the LogServer) the location of its message elaboration
// file.
{
unsigned ret = CommonBase::OnPmap(Z);  // Base class behaviour
                                       // Not from LogServer - bail
if (Z->Src()!=pPmap->U.LogServer) return ret;

PMsg_p Msg;                            // Tell logserver: message elaboration
Msg.Key(Q::LOG,Q::LOAD);               // Message type
Msg.Src(Urank);                        // Me
Msg.Tgt(pPmap->U.LogServer);           // Target (now we know we know it)
string MsgFile = pOC->Messages();      // Elaboration file location
Msg.Put(0,&MsgFile);
string LogFile = pOC->Log();           // Log output file
Msg.Put(1,&LogFile);
Msg.Send();                            // Kick it
return ret;                            // And go about our business
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
// All known monkey input is (should be) tabulated in Pglobals.h,
// (along with all known message layouts).
{
if (pC==0) return 1;                   // Paranoia
pC->Trim();
string scmnd = pC->Co;                 // Pull out the command string
if (scmnd.empty()) return 0;           // It has no commands - it's a comment
if (scmnd[0]=='*') {                   // Special case - EOF marker
  pCmCall->stack.pop_back();           // Remove from call stack
  return 0;
}
// Each command has its own class, with a splitter functor (operator()()) to
// decode the individual clauses.
fd = pCmPath->Fopen();                 // Sort out the detail file stream
                                       // Write a micro-header?
if ((pCmPath->OuMode==CmPath::Ou_Lolf)||(pCmPath->OuMode==CmPath::Ou_Ofgf))
  WriteUheader(pC);
const unsigned INVALIDCODE=999;        // Yukky yukky yuk yuk
unsigned code = INVALIDCODE;
if (scmnd=="call") code=(*pCmCall)(pC);// Call
if (scmnd=="comp") code=(*pCmComp)(pC);// Compose
if (scmnd=="depl") code=(*pCmDepl)(pC);// Deploy
if (scmnd=="dump") code=(*pCmDump)(pC);// Dump
if (scmnd=="exit") code=CmExit(pC);    // Exit
if (scmnd=="init") code=(*pCmInit)(pC);// Initialise
if (scmnd=="load") code=(*pCmLoad)(pC);// Load
if (scmnd=="path") code=(*pCmPath)(pC);// Path
if (scmnd=="plac") code=(*pCmPlac)(pC);// Place
if (scmnd=="reca") code=(*pCmReca)(pC);// Recall
if (scmnd=="retu") code=CmRetu(pC);    // Return
if (scmnd=="run")  code=(*pCmRun)(pC); // Run
if (scmnd=="show") code=(*pCmShow)(pC);// Show
if (scmnd=="stop") code=(*pCmStop)(pC);// Stop
if (scmnd=="syst") code=(*pCmSyst)(pC);// System
if (scmnd=="test") code=(*pCmTest)(pC);// Test
if (scmnd=="tlin") code=(*pCmTlin)(pC);// Tlink
if (scmnd=="unlo") code=(*pCmUnlo)(pC);// Unload
if (scmnd=="untl") code=(*pCmUntl)(pC);// Untypelink
if (scmnd.at(0)==char(0)) code=CmExit(pC); // Ctrl-D behaviour in linux-land
if (scmnd.at(0)==char(4)) code=CmExit(pC); // Ctrl-D behaviour in Windoze
fd = pCmPath->Fclose();                 // Reset detail file stream
if (code==INVALIDCODE) return CmDrop(pC);
return code;
}

//------------------------------------------------------------------------------

void Root::Prompt(FILE * fp)
{
if (fp!=stdout or !Root::promptOn) return;
fprintf(fp,"%s",Root::prompt);     // Console prompt
fflush(fp);
}

//------------------------------------------------------------------------------

void Root::WriteUheader(Cli * pC)
{
fprintf(fd,"%s\n%s %s ",Q::eline.c_str(),GetDate(),GetTime());
if (!pCmPath->lastfile.empty()) fprintf(fd,"file %s\n",
                                           pCmPath->lastfile.c_str());
fprintf(fd,"command [%s]\nfrom ",pC->Orig.c_str());
if (pCmCall->stack.empty()) fprintf(fd,"console\n");
else fprintf(fd,"batch file %s line %d\n",
                pCmCall->stack.back().c_str(),pC->problem.lin);
fprintf(fd,"%s\n\n",Q::eline.c_str());
fflush(fd);
Post(20,pC->Orig,pCmPath->lastfile);
}

//==============================================================================

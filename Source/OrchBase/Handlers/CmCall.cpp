//------------------------------------------------------------------------------

#include "CmCall.h"
#include "macros.h"
#include "Root.h"
#include "Pglobals.h"

//==============================================================================

CmCall::CmCall(OrchBase * p):par(p)
{
echo         = false;                  // Batch subsystem opaque

}

//------------------------------------------------------------------------------

CmCall::~CmCall()
{
}

//------------------------------------------------------------------------------

void CmCall::CaEcho(Cli::Cl_t Cl)
// Monkey wants to see what's going on in the batch subsystem
// POETS> call /echo=off|on
{
string secho = Cl.GetP();
if (secho=="on" ) echo = true;
if (secho=="off") echo = false;
par->Post(34,echo?"on":"off");
}

//------------------------------------------------------------------------------

void CmCall::CaFile(Cli::Cl_t Cl)
// Switch input from monkey to file
// POETS> call /file=file_name
// Two important data elements here: the command queue and the file stack
// The stack is used for nothing except a recursion trap.
// The command queue is a list of pending commands, read in from a batch file.
// Every/any time a "call /file=" is encountered, the commands in the file are
// immediately pushed onto the front of the queue, so that the 'next' command
// encountered is the first one from the file we've just pushed on.
// The end of the file is delineated by a special command "*" (which can never
// occour in nature). Having pushed the command file contents onto the list, we
// start to loop through it, executing commands as we go.
// Note that if we nest batch files, this routine will get nested, and so will
// the loop-walking code. But it's a class, so it doesn;t actually matter: the
// *code* here just executes in the context of a different (host machine) stack
// frame.
{
string sfile = Cl.GetP();              // Get the batch filename

if (sfile.empty()) {                   // Blank batch filename ?
  par->Post(31);
  return;
}

string op = Cl.GetO();                 // Append pathname?
if (op==string("+")) sfile = par->pCmPath->pathBatc + sfile;
if (!file_readable(sfile.c_str())) {   // Can we read it?
  par->Post(112,sfile);                // Apparently not.....
  return;
}
                                       // Trap attempted recursion
if (find(stack.begin(),stack.end(),sfile)!=stack.end()) {
  par->Post(33,sfile);                 // Flag it
  return;
}

list<Cli> temp = Cli::File(sfile,true);// Dismantle incoming file
if (temp.empty()) return;              // Save us from daft users

unsigned line=0;                       // Append a "line=" clause if it's a
WALKLIST(Cli,temp,i) {                 // return
  (*i).Trim();
  line++;
  if ((*i).Co=="retu") (*i)=Cli((*i).Orig+" /line="+uint2str(line));
}

int l,c;                               // Putative error coordinates
temp.back().Err(l,c);                  // Any errors will be in the last command
if (l>=0) {                            // Syntax errors inside file?
  par->Post(37,sfile,int2str(l),int2str(c));
  return;
}
                                       // And finally, we're good to go
echo = false;                          // Default start echo option
Cli Cx;
Cx.Co = "*";                           // Write EObatch to queue
Equeue.push_front(Cx);
Equeue.splice(Equeue.begin(),temp);    // Push file contents to front of queue
stack.push_back(sfile);                // Push current batch file name to stack
                                       // Now we just let Root::OnIdle handle it
}

//------------------------------------------------------------------------------

void CmCall::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%s CmCall+++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sMe,Parent      0x%#018lx,0x%#018lx\n",
            os,(uint64_t)this,(uint64_t)par);
fprintf(fp,"%secho    %s\n",os,echo?"TRUE":"FALSE");
fprintf(fp,"%sRecursion trap filename stack (%lu elements):\n",
            os,stack.size());
unsigned i=0;
for (;i<stack.size();i++) fprintf(fp,"%s%3u:%s\n",os,i,stack[i].c_str());
fprintf(fp,"%sBatch command queue (%lu elements):\n",os,Equeue.size());
i=1;
WALKLIST(Cli,Equeue,j) {
  fprintf(fp,"%sCommand %3u:\n",os,i++);
  (*j).Dump(off+2,fp);
}
fprintf(fp,"%sCmCall -------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

bool CmCall::IsEmpty()
{
  return Equeue.empty();
}

//------------------------------------------------------------------------------

Cli CmCall::Front()
// Provides a valid Cm object: if there was one in the Q, it gets popped off
// the front; otherwise an empty one is returned.
// *Note everything is done by value because Cli is quite small*
{
Cli Cm;
if (Equeue.empty()) return Cm;         // Batch queue empty?
Cm = Equeue.front();                   // Pull off the next one
Equeue.pop_front();                    // Erase from queue
return Cm;
}

//------------------------------------------------------------------------------

void CmCall::Show(FILE * fp)
// Monkey wants to see the call stack
// POETS> call /show
{
fprintf(fp,"\n%s\n",Q::pline.c_str());
fprintf(fp,"\nBatch processsing subsystem status at %s on %s\n\n",
           GetTime(),GetDate());
fprintf(fp,"Batch echo %s\n",echo?"ON":"OFF");
fprintf(fp,"Batch call stack has %lu entries :\n",stack.size());
unsigned i=0;
fprintf(fp,"+-----+-------------------------------------------------------+\n");
for (;i<stack.size();i++) fprintf(fp,"| %3u | %-50s    |\n",i,stack[i].c_str());
fprintf(fp,"+-----+-------------------------------------------------------+\n");
fprintf(fp,"Batch command queue has %lu entries :\n",Equeue.size());
i=1;
fprintf(fp,"+-------------+-----------------------------------------------+\n");
WALKLIST(Cli,Equeue,j)
  fprintf(fp,"| Command %3u | %-45s |\n",i++,(*j).Orig.c_str());
fprintf(fp,"+-------------+-----------------------------------------------+\n");
fprintf(fp,"\n%s\n",Q::sline.c_str());
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmCall::operator()(Cli * pC)
{
//printf("CmCall operator() splitter for ....\n");
if (pC==0) return 0;                   // Paranoia - nothing to call
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause name
  if (sCl=="echo") { CaEcho(*i); continue; }
  if (sCl=="file") { CaFile(*i); continue; }
//  if (strcmp(scl.c_str(),"show")==0) { Show(*i);   continue; }
  par->Post(25,sCl,"call");                 // Unrecognised clause
}
return 0;
}

//==============================================================================

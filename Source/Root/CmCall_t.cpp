//------------------------------------------------------------------------------

#include "CmCall_t.h"
#include "macros.h"
#include "Root.h"

//==============================================================================

CmCall_t::CmCall_t(Root * p):par(p)
{
echo         = false;                  // Batch subsystem opaque

}

//------------------------------------------------------------------------------

CmCall_t::~CmCall_t()
{
}

//------------------------------------------------------------------------------

void CmCall_t::Dump(FILE * fp)
{
fprintf(fp,"CmCall_t++++++++++++\n");
fprintf(fp,"Me,Parent      0x%#08p,0x%#08p\n",this,par);
fprintf(fp,"echo    %s\n",echo?"TRUE":"FALSE");
fprintf(fp,"Recursion trap filename stack (%u elements):\n",stack.size());
WALKVECTOR(string,stack,i) fprintf(fp,"%s\n",(*i).c_str());
fprintf(fp,"Batch command queue (%u elements):\n",Equeue.size());
WALKLIST(Cli,Equeue,i) (*i).Dump(fp);
fprintf(fp,"CmCall_t------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void CmCall_t::CaEcho(Cli::Cl_t Cl)
// Monkey wants to see what's going on in the batch subsystem
// POETS> call /echo=off|on
{
string secho = Cl.GetP();
if (strcmp(secho.c_str(),"on" )==0) echo = true;
if (strcmp(secho.c_str(),"off")==0) echo = false;
par->Post(34,echo?"on":"off");
}

//------------------------------------------------------------------------------

void CmCall_t::CaFile(Cli::Cl_t Cl)
// Switch input from monkey to file
// POETS> call /file=file_name
{
string sfile = Cl.GetP();              // Get the batch filename
if (find(stack.begin(),stack.end(),sfile)!=stack.end()) {
  Equeue.pop_front();                  // EOF marker in command Q now NFG
  par->Post(33,sfile);                      // Trap attempted batch recursion
  return;
}
if (sfile.empty()) {                   // Blank batch filename
  par->Post(31);
  return;
}
if (!file_readable(sfile.c_str())) {
  par->Post(32,sfile);
  return;
}

Cli Cx;
Cx.Co = "*";
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

void CmCall_t::CaShow(Cli::Cl_t Cl)
// Monkey wants to see the call stack
// POETS> call /show
{
FILE * fp = stdout;
fprintf(fp,"Batch call stack has %lu entries\n",stack.size());
WALKVECTOR(string,stack,i) fprintf(fp,"%s\n",(*i).c_str());
fprintf(fp,"Batch command queue has %lu entries\n",Equeue.size());
WALKLIST(Cli,Equeue,i) fprintf(fp,"%s\n",(*i).Orig.c_str());
fflush(fp);
}

//------------------------------------------------------------------------------

Cli CmCall_t::Front()
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

unsigned CmCall_t::operator()(Cli * pC)
{
//printf("CmCall_t operator() splitter for ....\n");
if (pC==0) return 0;                   // Paranoia - nothing to call
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string scl = (*i).Cl;                // Pull out clause name
  if (strcmp(scl.c_str(),"echo")==0) { CaEcho(*i); continue; }
  if (strcmp(scl.c_str(),"file")==0) { CaFile(*i); continue; }
  if (strcmp(scl.c_str(),"show")==0) { CaShow(*i); continue; }
  par->Post(25,scl,"call");                 // Unrecognised clause
}
return 0;
}

//==============================================================================


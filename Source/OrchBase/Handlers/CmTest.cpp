//------------------------------------------------------------------------------

#include "CmTest.h"
#include "OrchBase.h"

//==============================================================================

CmTest::CmTest(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmTest::~CmTest()
{
}

//------------------------------------------------------------------------------

void CmTest::Cm_Echo(Cli::Cl_t cl)
{
    /* Combine parameters. */
    std::string args;
    std::vector<Cli::Pa_t>::iterator arg;
    for (arg = cl.Pa_v.begin(); arg != cl.Pa_v.end(); arg++)
    {
        if (arg != cl.Pa_v.begin()) args += " ";
        args += arg->Concatenate();
    }

    /* Leave if trivial. */
    if (args.empty()) return;

    /* Post */
    par->Post(1, args);

    /* Write to microlog, if possible. */
    FILE* ulog = fopen(par->pCmPath->lastfile.c_str(), "a");
    if (ulog == PNULL) return;
    fprintf(ulog, "%s\n", args.c_str());
    fclose(ulog);
}

//------------------------------------------------------------------------------

void CmTest::Cm_Sleep(Cli::Cl_t cl)
// Pass an unsigned integer, we kip for a bit. Pass something else, nothing
// will happen (probably).
{
    OSFixes::sleep(str2uint(cl.Pa_v.begin()->Concatenate()));
}

//------------------------------------------------------------------------------

void CmTest::Dump(FILE * fp)
{
fprintf(fp,"CmTest+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
if (par==0) fprintf(fp,"OrchBase parent not defined\n");
else fprintf(fp,"Orchbase parent : %s\n",par->FullName().c_str());
fprintf(fp,"CmTest-------------------------------------------------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void CmTest::Show(FILE * fp)
{
fprintf(fp,"\nTest attributes and state:\n");
fprintf(fp,"\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmTest::operator()(Cli * pC)
// Handle "test" command from the monkey.
{
//printf("CmTest operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause name
  if (strcmp(sCl.c_str(),"echo")==0) {Cm_Echo(*i); continue;}
  if (strcmp(sCl.c_str(),"slee")==0) {Cm_Sleep(*i); continue;}
  par->Post(25,sCl,"test");           // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================

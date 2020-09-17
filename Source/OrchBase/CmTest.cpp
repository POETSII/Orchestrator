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
//  if (strcmp(scl.c_str(),"app" )==0) { par->Post(247,pC->Co,scl,(*i).GetP());continue;}
//  if (strcmp(scl.c_str(),"depl")==0) { par->Post(247,pC->Co,scl,(*i).GetP());continue;}
//  if (strcmp(scl.c_str(),"init")==0) { par->Post(247,pC->Co,scl,(*i).GetP());continue;}
//  if (strcmp(scl.c_str(),"run" )==0) { par->Post(247,pC->Co,scl,(*i).GetP());continue;}
//  if (strcmp(scl.c_str(),"stop")==0) { par->Post(247,pC->Co,scl,(*i).GetP());continue;}
  par->Post(25,sCl,"test");           // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================


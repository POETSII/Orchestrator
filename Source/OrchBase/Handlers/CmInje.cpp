//------------------------------------------------------------------------------

#include "CmInje.h"
#include "OrchBase.h"

//==============================================================================

CmInje::CmInje(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmInje::~CmInje()
{
}

//------------------------------------------------------------------------------

void CmInje::Dump(FILE * fp)
{
fprintf(fp,"CmInje+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
if (par==0) fprintf(fp,"OrchBase parent not defined\n");
else fprintf(fp,"Orchbase parent : %s\n",par->FullName().c_str());
fprintf(fp,"CmInje-------------------------------------------------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void CmInje::Show(FILE * fp)
{
fprintf(fp,"\nInjector attributes and state:\n");
fprintf(fp,"\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmInje::operator()(Cli * pC)
// Handle "inje(ct)" command from the INJECTOR PROCESS.
{
//printf("CmPath_t operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause name
//  if (sCl=="app" ) { par->Post(247,pC->Co,scl,(*i).GetP());continue;}
  par->Post(25,sCl,"injector");           // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================

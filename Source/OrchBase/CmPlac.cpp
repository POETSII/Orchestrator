//------------------------------------------------------------------------------

#include "CmPlac.h"
#include "OrchBase.h"

//==============================================================================

CmPlac::CmPlac(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmPlac::~CmPlac()
{
}

//------------------------------------------------------------------------------

void CmPlac::Dump(FILE * fp)
{
fprintf(fp,"CmPlac+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
if (par==0) fprintf(fp,"OrchBase parent not defined\n");
else fprintf(fp,"Orchbase parent : %s\n",par->FullName().c_str());
fprintf(fp,"CmPlac-------------------------------------------------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void CmPlac::Show(FILE * fp)
{
fprintf(fp,"\nPlacement subsystem attributes and state:\n");
fprintf(fp,"\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmPlac::operator()(Cli * pC)
// Handle "plac(e)" command from the monkey.
{
//printf("CmPath_t operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause name
  if (sCl=="app" ) { par->Post(247,sCl,(*i).GetP()); continue; }
  par->Post(25,sCl,"place");           // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================


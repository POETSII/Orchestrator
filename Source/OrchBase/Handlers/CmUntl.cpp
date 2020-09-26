//------------------------------------------------------------------------------

#include "CmUntl.h"
#include "OrchBase.h"

//==============================================================================

CmUntl::CmUntl(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmUntl::~CmUntl()
{
}

//------------------------------------------------------------------------------

void CmUntl::Dump(FILE * fp)
{
fprintf(fp,"CmUntl+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
if (par==0) fprintf(fp,"OrchBase parent not defined\n");
else fprintf(fp,"Orchbase parent : %s\n",par->FullName().c_str());
fprintf(fp,"CmUntl-------------------------------------------------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void CmUntl::Show(FILE * fp)
{
fprintf(fp,"\nUntypelinker attributes and state:\n");
fprintf(fp,"\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmUntl::operator()(Cli * pC)
// Handle "untl(ink)" command from the monkey.
{
//printf("CmUntl operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;               // Pull out clause name
  if (sCl=="app" ) { par->Post(247,sCl,(*i).GetP()); continue; }
  if (sCl=="tree") { par->Post(247,sCl,(*i).GetP()); continue; }
  par->Post(25,sCl,"untlink");         // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================

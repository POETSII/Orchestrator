//------------------------------------------------------------------------------

#include "CmUnpl.h"
#include "OrchBase.h"

//==============================================================================

CmUnpl::CmUnpl(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmUnpl::~CmUnpl()
{
}

//------------------------------------------------------------------------------

void CmUnpl::Dump(FILE * fp)
{
fprintf(fp,"CmUnpl+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
if (par==0) fprintf(fp,"OrchBase parent not defined\n");
else fprintf(fp,"Orchbase parent : %s\n",par->FullName().c_str());
fprintf(fp,"CmUnpl-------------------------------------------------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void CmUnpl::Show(FILE * fp)
{
fprintf(fp,"\nUnplacer attributes and state:\n");
fprintf(fp,"\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmUnpl::operator()(Cli * pC)
// Handle "unpl(ace)" command from the monkey.
{
//printf("CmUnpl operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause name
  if (sCl=="app" ) { par->Post(247,pC->Co,sCl,(*i).GetP()); continue; }
  par->Post(25,sCl,"unplace");           // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================


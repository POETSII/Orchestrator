//------------------------------------------------------------------------------

#include "CmBuil.h"
#include "OrchBase.h"

//==============================================================================

CmBuil::CmBuil(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmBuil::~CmBuil()
{
}

//------------------------------------------------------------------------------

void CmBuil::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sCmBuil +++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
if (par==0) fprintf(fp,"%sOrchBase parent not defined\n",os);
else fprintf(fp,"%sOrchbase parent : %s\n",os,par->FullName().c_str());
fprintf(fp,"%sCmBuil -------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void CmBuil::Show(FILE * fp)
{
fprintf(fp,"\nBuilder attributes and state:\n");
fprintf(fp,"\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmBuil::operator()(Cli * pC)
// Handle "buil(d)" command from the monkey.
{
//printf("CmPath_t operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause string
  string sCo = pC->Co;                 // Pull out command string
  string sPa = (*i).GetP();            // Pull out (simple) parameter
  if (sCl=="app" ) { par->Post(247,sCo,sCl,sPa); continue; }
  if (sCl=="depl") { par->Post(247,sCo,sCl,sPa); continue; }
  if (sCl=="init") { par->Post(247,sCo,sCl,sPa); continue; }
  if (sCl=="run" ) { par->Post(247,sCo,sCl,sPa); continue; }
  if (sCl=="stop") { par->Post(247,sCo,sCl,sPa); continue; }
  par->Post(25,sCl,"build");           // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================


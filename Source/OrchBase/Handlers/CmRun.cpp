//------------------------------------------------------------------------------

#include "CmRun.h"
#include "OrchBase.h"
#include "Pglobals.h"

//==============================================================================

CmRun::CmRun(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmRun::~CmRun()
{
}

//------------------------------------------------------------------------------

void CmRun::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sCmRun +++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
if (par==0) fprintf(fp,"%sOrchBase parent not defined\n",os);
else fprintf(fp,"%sOrchbase parent : %s\n",os,par->FullName().c_str());
fprintf(fp,"%sCmRun -------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmRun::operator()(Cli * pC)
// Handle "run" command from the monkey.
{
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause string
  string sCo = pC->Co;                 // Pull out command string
  string sPa = (*i).GetP();            // Pull out (simple) parameter
  if (sCl=="app" ) { par->MshipCommand(*i, "run"); continue; }
  par->Post(25,sCl,"run");             // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================

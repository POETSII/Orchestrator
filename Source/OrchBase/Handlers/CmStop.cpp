//------------------------------------------------------------------------------

#include "CmStop.h"
#include "OrchBase.h"
#include "Pglobals.h"

//==============================================================================

CmStop::CmStop(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmStop::~CmStop()
{
}

//------------------------------------------------------------------------------

void CmStop::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sCmStop +++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
if (par==0) fprintf(fp,"%sOrchBase parent not defined\n",os);
else fprintf(fp,"%sOrchbase parent : %s\n",os,par->FullName().c_str());
fprintf(fp,"%sCmStop -------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmStop::operator()(Cli * pC)
// Handle "stop" command from the monkey.
{
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause string
  string sCo = pC->Co;                 // Pull out command string
  string sPa = (*i).GetP();            // Pull out (simple) parameter
  if (sCl=="app" ) { par->MshipCommand(*i, "stop"); continue; }
  par->Post(25,sCl,"stop");            // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================

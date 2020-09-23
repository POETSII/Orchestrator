//------------------------------------------------------------------------------

#include "CmUnlo.h"
#include "Apps_t.h"
#include "OrchBase.h"

//==============================================================================

CmUnlo::CmUnlo(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmUnlo::~CmUnlo()
{
}

//------------------------------------------------------------------------------

void CmUnlo::Cm_App(Cli::Cl_t cl)
// Unload an application from the Orchestrator database
{
                                       // Walk the parameters
for(unsigned i=0;i<cl.Pa_v.size();i++) {
  string as = cl.GetP(i);              // Application to go
  if (as[0]=='*') Apps_t::DelAll();    // Special case - all of them
  if (Apps_t::FindApp(as)==0) par->Post(64,as);
  else Apps_t::DelApp(as);             // Nope - just the one
}

}

//------------------------------------------------------------------------------

void CmUnlo::Dump(FILE * fp)
{
fprintf(fp,"CmUnlo+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
if (par==0) fprintf(fp,"OrchBase parent not defined\n");
else fprintf(fp,"Orchbase parent : %s\n",par->FullName().c_str());
fprintf(fp,"CmUnlo-------------------------------------------------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void CmUnlo::Show(FILE * fp)
{
fprintf(fp,"\nUnloader attributes and state:\n");
fprintf(fp,"\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmUnlo::operator()(Cli * pC)
// Handle "unlo(ad)" command from the monkey.
{
//printf("CmUnlo operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause name
  if (sCl=="app" ) { Cm_App(*i);                            continue; }
  if (sCl=="cons") { par->Post(247,pC->Co,sCl,(*i).GetP()); continue; }
  if (sCl=="engi") { par->Post(247,pC->Co,sCl,(*i).GetP()); continue; }
  if (sCl=="grap") { par->Post(247,pC->Co,sCl,(*i).GetP()); continue; }
  if (sCl=="plac") { par->Post(247,pC->Co,sCl,(*i).GetP()); continue; }
  if (sCl=="pola") { par->Post(247,pC->Co,sCl,(*i).GetP()); continue; }
  if (sCl=="type") { par->Post(247,pC->Co,sCl,(*i).GetP()); continue; }
  par->Post(25,sCl,"unload");           // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================


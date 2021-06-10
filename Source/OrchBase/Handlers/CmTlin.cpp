//------------------------------------------------------------------------------

#include "CmTlin.h"
#include "OrchBase.h"
#include "Apps_t.h"
#include "DS_integ.h"
#include "Pglobals.h"

//==============================================================================

CmTlin::CmTlin(OrchBase * p):par(p)
{
fd   = par->fd;
}

//------------------------------------------------------------------------------

CmTlin::~CmTlin()
{
}

//------------------------------------------------------------------------------

void CmTlin::Cm_App(Cli::Cl_t cl)
// Typelink a named application OR all of them.
{
set<GraphI_t*> gis;
if (par->GetGraphIs(cl,gis)==1) return;
WALKSET(GraphI_t*,gis,i)
{
  par->Post(234,(*i)->Name());
  unsigned lecnt = (*i)->TypeLink();
  ecnt += lecnt;
  if (lecnt != 0) par->Post(254,(*i)->Name(),uint2str(lecnt));
  else par->Post(249,(*i)->Name());
}
}

//------------------------------------------------------------------------------

void CmTlin::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sCmTlin +++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
if (par==0) fprintf(fp,"%sOrchBase parent not defined\n",os);
else fprintf(fp,"%sOrchbase parent : %s\n",os,par->FullName().c_str());
fprintf(fp,"%sCmTlin -------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void CmTlin::ReportTLinkEnd()
{
fprintf(fd,"%s\n%s %s\n\n",Q::dline.c_str(),GetDate(),GetTime());
fprintf(fd,"\nTypelinking exhibits %u accumulated errors in %lu msecs\n\n%s\n",
        ecnt,mTimer(t0),Q::sline.c_str());
fflush(fd);
}

//------------------------------------------------------------------------------

void CmTlin::ReportTLinkStart()
{
ecnt = 0;                              // Accumulated errors
t0 = mTimer();
fprintf(fd,"%s\n%s %s\n\nOrchestrator contains %lu applications\n"
           "Typelinking...\n\n",
           Q::pline.c_str(),GetDate(),GetTime(),Apps_t::Apps_m.size());
fflush(fd);
}

//------------------------------------------------------------------------------

void CmTlin::Show(FILE * fp)
{
fprintf(fp,"\nType linker attributes and state:\n");
fprintf(fp,"\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmTlin::operator()(Cli * pC)
// Handle "buil(d)" command from the monkey.
{

if (pC==0) return 0;                   // Paranoia
fd = par->fd;
ReportTLinkStart();
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause name
  if (sCl=="app" ) { Cm_App(*i);                    continue; }
  par->Post(25,sCl,"tlink");           // Unrecognised clause
}
ReportTLinkEnd();
return 0;                              // Legitimate command exit
}

//==============================================================================

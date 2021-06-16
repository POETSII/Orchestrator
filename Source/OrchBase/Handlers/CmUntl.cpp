//------------------------------------------------------------------------------

#include "CmUntl.h"
#include "OrchBase.h"
#include "Apps_t.h"
#include "Pglobals.h"

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

void CmUntl::Cm_App(Cli::Cl_t cl)
// Un-typelink a named application OR all of them.
{
set<GraphI_t*> gis;
if (par->GetGraphIs(cl,gis)==1) return;
WALKSET(GraphI_t*,gis,i)
{
  par->Post(255,(*i)->Name());
  (*i)->UnTLink();
  par->Post(256,(*i)->Name());
}
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

void CmUntl::ReportUTLinkEnd()
{
fprintf(fd,"%s\n%s %s\n\n",Q::dline.c_str(),GetDate(),GetTime());
fprintf(fd,"\nUn-typelinking complete in %lu msecs\n\n%s\n",
        mTimer(t0),Q::sline.c_str());
fflush(fd);
}

//------------------------------------------------------------------------------

void CmUntl::ReportUTLinkStart()
{
t0 = mTimer();
fprintf(fd,"%s\n%s %s\n\nOrchestrator contains %lu applications\n"
           "Un-typelinking...\n\n",
           Q::pline.c_str(),GetDate(),GetTime(),Apps_t::Apps_m.size());
fflush(fd);
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
if (pC==0) return 0;                   // Paranoia
fd = par->fd;
ReportUTLinkStart();
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;               // Pull out clause name
  if (sCl=="app" ) { Cm_App(*i);                     continue; }
  par->Post(25,sCl,"untlink");         // Unrecognised clause
}
ReportUTLinkEnd();
return 0;                              // Legitimate command exit
}

//==============================================================================

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
string apName = cl.GetP();             // Application name

if (apName.empty()) {                  // ...not supplied ?
  par->Post(244,"---");
  return;
}
                                       // Not "*" AND not there ?
Apps_t * pA = Apps_t::FindApp(apName);
if ((apName!="*")&&(pA==0)) {
  par->Post(244,apName);
  return;
}
                                       // OK, good to go
if (pA!=0) UnTypeLink(pA);             // Un-typelink the named application
                                       // ... or all of them ?
else WALKMAP(string,Apps_t *,Apps_t::Apps_m,i) UnTypeLink((*i).second);
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

void CmUntl::UnTypeLink(Apps_t * pA)
// UnTypelink a whole application.
{
WALKVECTOR(GraphI_t *,pA->GraphI_v,i) (*i)->UnTLink();
}

//------------------------------------------------------------------------------

unsigned CmUntl::operator()(Cli * pC)
// Handle "untl(ink)" command from the monkey.
{
//printf("CmUntl operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
fd = par->fd;
ReportUTLinkStart();
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;               // Pull out clause name
  if (sCl=="app" ) { Cm_App(*i);                     continue; }
  if (sCl=="tree") { par->Post(247,sCl,(*i).GetP()); continue; }
  par->Post(25,sCl,"untlink");         // Unrecognised clause
}
ReportUTLinkEnd();
return 0;                              // Legitimate command exit
}

//==============================================================================

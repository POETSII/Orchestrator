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
ecnt = 0;
}

//------------------------------------------------------------------------------

CmTlin::~CmTlin()
{
}

//------------------------------------------------------------------------------

void CmTlin::Cm_App(Cli::Cl_t cl)
// Typelink a named application OR all of them.
{
string apName = cl.GetP();             // Application name

if (apName.empty()) {                  // ...not supplied ?
  par->Post(244,"---");
  ecnt++;
  return;
}
                                       // Not "*" AND not there ?
Apps_t * pA = Apps_t::FindApp(apName);
if ((apName!="*")&&(pA==0)) {
  par->Post(244,apName);
  ecnt++;
  return;
}
                                       // OK, good to go
if (pA!=0) TypeLink(pA);               // Type link the named application
                                       // ... or all of them ?
else WALKMAP(string,Apps_t *,Apps_t::Apps_m,i) TypeLink((*i).second);
}

//------------------------------------------------------------------------------

void CmTlin::Cm_Grap(Cli::Cl_t cl)
{
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

void CmTlin::TypeLink(Apps_t * pA)
// Typelink a whole application.
{
WALKVECTOR(GraphI_t *,pA->GraphI_v,i) ecnt += (*i)->TypeLink();
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
  if (sCl=="grap") { Cm_Grap(*i);                   continue; }
  par->Post(25,sCl,"tlink");           // Unrecognised clause
}
ReportTLinkEnd();
return 0;                              // Legitimate command exit
}

//==============================================================================

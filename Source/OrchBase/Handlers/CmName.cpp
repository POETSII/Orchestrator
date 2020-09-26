//------------------------------------------------------------------------------

#include "CmName.h"
#include "OrchBase.h"

//==============================================================================

CmName::CmName(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmName::~CmName()
{
}

//------------------------------------------------------------------------------

void CmName::Dump(FILE * fp)
{
fprintf(fp,"CmName+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
if (par==0) fprintf(fp,"OrchBase parent not defined\n");
else fprintf(fp,"Orchbase parent : %s\n",par->FullName().c_str());
fprintf(fp,"CmName-------------------------------------------------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void CmName::Show(FILE * fp)
{
fprintf(fp,"\nNameServer attributes and state:\n");
fprintf(fp,"\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmName::operator()(Cli * pC)
// Handle "name(server)" command from the monkey.
{
//printf("CmPath_t operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause name
//  if (strcmp(scl.c_str(),"app" )==0) { par->Post(247,scl,(*i).GetP());continue;}
//  if (strcmp(scl.c_str(),"depl")==0) { par->Post(247,scl,(*i).GetP());continue;}
//  if (strcmp(scl.c_str(),"init")==0) { par->Post(247,scl,(*i).GetP());continue;}
//  if (strcmp(scl.c_str(),"run" )==0) { par->Post(247,scl,(*i).GetP());continue;}
//  if (strcmp(scl.c_str(),"stop")==0) { par->Post(247,scl,(*i).GetP());continue;}
  par->Post(25,sCl,"nameserver");      // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================

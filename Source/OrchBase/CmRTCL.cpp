//------------------------------------------------------------------------------

#include "CmRTCL.h"
#include "OrchBase.h"

//==============================================================================

CmRTCL::CmRTCL(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmRTCL::~CmRTCL()
{
}

//------------------------------------------------------------------------------

void CmRTCL::Dump(unsigned off,FILE * fp)
{
fprintf(fp,"CmRTCL+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
if (par==0) fprintf(fp,"OrchBase parent not defined\n");
else fprintf(fp,"Orchbase parent : %s\n",par->FullName().c_str());
fprintf(fp,"CmRTCL-------------------------------------------------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void CmRTCL::Show(FILE * fp)
{
fprintf(fp,"\nReal time clock controller attributes and state:\n");
fprintf(fp,"\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmRTCL::operator()(Cli * pC)
// Handle "RTCL" command from the monkey.
{
//printf("CmRTCL operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause name
//  if (strcmp(scl.c_str(),"app" )==0) { par->Post(247,pC->Co,scl,(*i).GetP());continue;}
//  if (strcmp(scl.c_str(),"depl")==0) { par->Post(247,pC->Co,scl,(*i).GetP());continue;}
//  if (strcmp(scl.c_str(),"init")==0) { par->Post(247,pC->Co,scl,(*i).GetP());continue;}
//  if (strcmp(scl.c_str(),"run" )==0) { par->Post(247,pC->Co,scl,(*i).GetP());continue;}
//  if (strcmp(scl.c_str(),"stop")==0) { par->Post(247,pC->Co,scl,(*i).GetP());continue;}
  par->Post(25,sCl,"RTCL");            // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================


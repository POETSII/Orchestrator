//------------------------------------------------------------------------------

#include "CmExec.h"
#include "OrchBase.h"

//==============================================================================

CmExec::CmExec(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmExec::~CmExec()
{
}

//------------------------------------------------------------------------------

void CmExec::Dump(FILE * fp)
{
fprintf(fp,"CmExec+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
if (par==0) fprintf(fp,"OrchBase parent not defined\n");
else fprintf(fp,"Orchbase parent : %s\n",par->FullName().c_str());
fprintf(fp,"CmExec-------------------------------------------------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void CmExec::Show(FILE * fp)
{
fprintf(fp,"\nExec attributes and state:\n");
fprintf(fp,"\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmExec::operator()(Cli * pC)
// Handle "buil(d)" command from the monkey.
{
//printf("CmPath_t operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
par->Post(247,pC->Co,"exec","stuff");
return 0;                              // Legitimate command exit
}

//==============================================================================

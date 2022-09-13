//------------------------------------------------------------------------------

#include "CmMoni.h"
#include "OrchBase.h"
#include "Pglobals.h"

//==============================================================================

CmMoni::CmMoni(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmMoni::~CmMoni()
{
}

//------------------------------------------------------------------------------

void CmMoni::Dump(FILE * fp)
{
fprintf(fp,"CmMoni+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
if (par==0) fprintf(fp,"OrchBase parent not defined\n");
else fprintf(fp,"Orchbase parent : %s\n",par->FullName().c_str());
fprintf(fp,"CmMoni-------------------------------------------------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void CmMoni::Show(FILE * fp)
{
fprintf(fp,"\nMoni attributes and state:\n");
fprintf(fp,"\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void CmMoni::Cm_Spy()
// Sends a message to the MonServer, commanding it to toggle its Spy. This also
// sends the path for the spy to write to, to the MonServer.
{
    PMsg_p message;
    message.Key(Q::MONI, Q::SPY);
    message.Src(par->Urank);
    message.Tgt(par->pPmap->U.MonServer);
    message.Put(0, &par->pCmPath->pathMonS);
    message.Send();
}

//------------------------------------------------------------------------------

unsigned CmMoni::operator()(Cli * pC)
// Handle "moni" command from the monkey.
{
//printf("CmMoni operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause name
  if (strcmp(sCl.c_str(),"spy" )==0) { Cm_Spy(); continue; }
  par->Post(25,sCl,"moni");           // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================

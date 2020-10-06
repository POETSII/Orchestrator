//------------------------------------------------------------------------------

#include "CmShow.h"
#include "OrchBase.h"
#include "OrchConfig.h"
#include "Apps_t.h"
#include "Root.h"

//==============================================================================

CmShow::CmShow(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmShow::~CmShow()
{
}

//------------------------------------------------------------------------------

void CmShow::Cm_Engine(FILE* fp)
{
    if (par->pE == PNULL) fprintf(fp, "No engine (hardware model) loaded.\n");
    else fprintf(fp,
                 "Engine '%s' loaded:\n"
                 "Author: %s\n"
                 "Version: %s\n"
                 "From file (may be built-in): %s\n",
                 par->pE->Name().c_str(),
                 par->pE->author.c_str(),
                 par->pE->version.c_str(),
                 par->pE->fileOrigin.c_str());
}

//------------------------------------------------------------------------------

void CmShow::Dump(FILE * fp)
{
fprintf(fp,"CmShow+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
if (par==0) fprintf(fp,"OrchBase parent not defined\n");
else fprintf(fp,"Orchbase parent : %s\n",par->FullName().c_str());
fprintf(fp,"CmShow-------------------------------------------------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmShow::operator()(Cli * pC)
// Handle "show" command from the monkey.
{
//printf("CmShow operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause name
  string sCo = pC->Co;                 // Pull out command name
  string sPa = (*i).GetP();            // Pull out simple parameter name
  FILE * f = par->fd;                  // Save some typing
  if (sCl=="apps") { Apps_t::Show(f);             continue;  }
  if (sCl=="batc") { par->pCmCall->Show(f);       continue;  }
  if (sCl=="comp") { par->pCmComp->Show(f);       continue;  }
  if (sCl=="engi") { Cm_Engine(f);                continue;  }
  if (sCl=="name") { par->Post(247,sCo,sCl,sPa);  continue;  }
  if (sCl=="pars") { par->pCmLoad->Show(f);       continue;  }
  if (sCl=="path") { par->pCmPath->Show(f);       continue;  }
  if (sCl=="plac") { par->Post(247,sCo,sCl,sPa);  continue;  }
  if (sCl=="syst") {
    par->pPmap->Show(f);
    dynamic_cast<Root *>(par)->pOC->Show(f);
    par->pCmCall->Show(f);
    continue;
  }
  par->Post(25,sCl,"show");            // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================

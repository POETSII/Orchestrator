//------------------------------------------------------------------------------

#include "CmDump.h"
#include "OrchBase.h"
#include "Apps_t.h"

//==============================================================================

CmDump::CmDump(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmDump::~CmDump()
{
}

//------------------------------------------------------------------------------

void CmDump::Cm_Engine(Cli::Cl_t cl)
{
    /* Can't dump an engine if there's no engine to dump, yo. */
    if (par->pE == PNULL)
    {
        par->Post(139);
        return;
    }

    /* Figure out the dump path, if any. */
    std::string op = cl.GetO(0);
    std::string parameter = cl.GetP(0);
    std::string filePath;
    if (op == std::string("+"))
    {
        filePath = par->pCmPath->pathEngi + parameter;
    }
    else filePath = parameter;

    if (filePath.empty()) par->pE->Dump();
    else
    {
        FILE * fp = fopen(filePath.c_str(), "w");
        if (fp == PNULL) par->Post(132, filePath);
        else
        {
            par->pE->Dump(fp);
            fclose(fp);
        }
    }
}

//------------------------------------------------------------------------------

void CmDump::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sCmDump +++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
if (par==0) fprintf(fp,"%sOrchBase parent not defined\n",os);
else fprintf(fp,"%sOrchbase parent : %s\n",os,par->FullName().c_str());
fprintf(fp,"%sCmDump -------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void CmDump::Show(FILE * fp)
{
fprintf(fp,"\nDumper attributes and state:\n");
fprintf(fp,"\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmDump::operator()(Cli * pC)
// Handle "dump" command from the monkey.
{
//printf("CmPath_t operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause name
  string sCo = pC->Co;                 // Pull out command name
  string sPa = (*i).GetP();            // Pull out simple parameter name
  FILE * f = par->fd;                  // Save some typing
  if (sCl=="apps") { Apps_t::DumpAll(f);          continue; }
  if (sCl=="batc") { par->Post(247,sCo,sCl,sPa);  continue; }
  if (sCl=="bina") { par->Post(247,sCo,sCl,sPa);  continue; }
  if (sCl=="engi") { Cm_Engine(*i);                continue; }
  if (sCl=="name") { par->Post(247,sCo,sCl,sPa);  continue; }
  if (sCl=="path") { par->Post(247,sCo,sCl,sPa);  continue; }
  if (sCl=="plac") { par->Post(247,sCo,sCl,sPa);  continue; }
  if (sCl=="syst") { par->Post(247,sCo,sCl,sPa);  continue; }
  par->Post(25,sCl,"dump");            // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================

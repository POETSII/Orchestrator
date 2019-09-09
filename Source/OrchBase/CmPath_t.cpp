//------------------------------------------------------------------------------

#include "CmPath_t.h"
#include "OrchBase.h"
#include "FileName.h"

//==============================================================================

CmPath_t::CmPath_t(OrchBase * p):par(p)
{
OuMode  = Ou_Stdo;                     // Output mode: details to console
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmPath_t::~CmPath_t()
{
}

//------------------------------------------------------------------------------

void CmPath_t::Dump(FILE * fp)
{
fprintf(fp,"CmPath_t++++++++++++\n");
if (par==0) fprintf(fp,"OrchBase parent not defined\n");
else fprintf(fp,"Orchbase parent : %s\n",par->FullName().c_str());
fprintf(fp,"pathBina : %s\n",pathBina.empty()?"---":pathBina.c_str());
fprintf(fp,"pathGrap : %s\n",pathGrap.empty()?"---":pathGrap.c_str());
fprintf(fp,"pathOutp : %s\n",pathOutp.empty()?"---":pathOutp.c_str());
fprintf(fp,"pathPlat : %s\n",pathPlat.empty()?"---":pathPlat.c_str());
fprintf(fp,"CmPath_t------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

FILE * CmPath_t::Fclose()
// Called (by Root::ProcCmnd) at the end of every command, to reset the detail
// output file stream pointer
{
switch (OuMode) {
  case Ou_Stdo : return stdout;
  case Ou_Lolf : fclose(par->fd); return stdout;
  case Ou_Ofgf : fclose(par->fd); return stdout;
  default      : return stdout;
}
}

//------------------------------------------------------------------------------

FILE * CmPath_t::Fopen()
// Called (by Root::ProcCmnd) at the start of every command, to set the detial
// output file stream pointer
{
                                       // Generate the current detail OP mode
GenerateMode();                        // (It may have changed)
switch (OuMode) {
  case Ou_Stdo : return stdout;        // Nothing to do
  case Ou_Lolf : return GetLolfp();    // New little file
  case Ou_Ofgf : return GetOfgfp();    // One hooge file
  default      : return stdout;        // Keep the compiler happy
}
}

//------------------------------------------------------------------------------

void CmPath_t::GenerateMode()
// Generate the output mode
{
if (pathOutp.empty()) {                // Default eveything to the console
  OuMode = Ou_Stdo;
  return;
}
                                       // If the last character is a '\', it's
                                       // a path.....
if (pathOutp[pathOutp.size()-1]==char('\\')) {    // It's a path....
  FileName Fn(pathOutp+"base");        // Sanity check:
  if (Fn.Err()) {                      // Are we?
    OuMode = Ou_Stdo;                  // Cockup
    par->Post(239,pathOutp);           // Not a valid filepath
    return;
  }
  OuMode = Ou_Lolf;                    // Lots of little files
  return;
}
                                       // OK, it's a complete filename
FileName Fn(pathOutp);                 // Just checking
if (Fn.Err()) {
  OuMode = Ou_Stdo;                    // Cockup
  par->Post(241,pathOutp);             // Not a valid file
  return;
}
OuMode = Ou_Ofgf;                      // One fucking great file
}

//------------------------------------------------------------------------------

FILE * CmPath_t::GetLolfp()
// Generate (and open) a filename just for the upcoming. We don't yet know what
// it is, so all we can do is generate a generic name and timestamp it
{
string T(GetTime());
for(unsigned i=0;i<T.size();i++) if (T[i]==':')T[i]='_';   // Lose the :
for(unsigned i=0;i<T.size();i++) if (T[i]=='.')T[i]='_';   // Lose the .
T = pathOutp + "POETS_" + T + ".log";  // Build the filename
FILE * fp = fopen(T.c_str(),"w");      // Try to open it
if (fp==0) {                           // The path might not exist?
  par->Post(246,T);
  return stdout;
}
return fp;                             // It's all good to go
}

//------------------------------------------------------------------------------

FILE * CmPath_t::GetOfgfp()
// The full name and path has been supplied by the user, and could be anything
// But we don't care, we just (try to) open it for append and let the monkey
// get on with it
{
FILE * fp = fopen(pathOutp.c_str(),"a");
if (fp==0) {                           // Cockup?
  par->Post(241,pathOutp);
  return stdout;
}
return fp;                             // You asked for it, you got it
}

//------------------------------------------------------------------------------

void CmPath_t::Show(FILE * fp)
{
fprintf(fp,"\nUser-defined filepath details:\n");
fprintf(fp,"Binary output filepath : %s\n",
           pathBina.empty()?"---":pathBina.c_str());
fprintf(fp,"Source input filepath  : %s\n",
           pathGrap.empty()?"---":pathGrap.c_str());
fprintf(fp,"ASCII output filepath  : %s\n",
           pathOutp.empty()?"---":pathOutp.c_str());
fprintf(fp,"HW definition filepath : %s\n",
           pathPlat.empty()?"---":pathPlat.c_str());
fprintf(fp,"\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned CmPath_t::operator()(Cli * pC)
// Handle "path" command from the monkey.
// We look at the parameter list before the clause, solely because it makes the
// coding easier. If it's a dud path, we store the dud-ness (OK), in case the
// clause needs it. If the command evolves onto anything more complex we may
// need to change this.
{
//printf("CmPath_t operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string scl = (*i).Cl;                // Pull out clause name
  FileName Fn((*i).GetP()+"base");     // Turn it into a legal filepath
  bool OK = !Fn.Err();                 // Do it make sense?
  if (strcmp(scl.c_str(),"bina")==0) { if (OK) pathBina = (*i).GetP();
                                       else par->Post(239,scl); continue; }
  if (strcmp(scl.c_str(),"clea")==0) { pathBina.clear();
                                       pathGrap.clear();
                                       pathOutp.clear();
                                       pathPlat.clear();        continue; }
  if (strcmp(scl.c_str(),"dump")==0) { Dump();                  continue; }
  if (strcmp(scl.c_str(),"grap")==0) { if (OK) pathGrap = (*i).GetP();
                                       else par->Post(239,scl); continue; }
  if (strcmp(scl.c_str(),"outp")==0) { pathOutp = (*i).GetP();  continue; }
  if (strcmp(scl.c_str(),"plat")==0) { if (OK) pathPlat = (*i).GetP();
                                       else par->Post(239,scl); continue; }
  if (strcmp(scl.c_str(),"show")==0) { Show();                  continue; }
  par->Post(25,scl,"system");          // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//------------------------------------------------------------------------------

//==============================================================================


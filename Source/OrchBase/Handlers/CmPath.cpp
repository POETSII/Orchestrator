//------------------------------------------------------------------------------

#include "CmPath.h"
#include "OrchBase.h"
#include "OrchConfig.h"
#include "Pglobals.h"
#include "FileName.h"
#include "Root.h"

//==============================================================================

CmPath::CmPath(OrchBase * p):par(p)
{
OuMode  = Ou_Stdo;                     // Output mode: details to console
par->fd = stdout;                      // Tell OrchBase
}

//------------------------------------------------------------------------------

CmPath::~CmPath()
{
}

//------------------------------------------------------------------------------

void CmPath::Clear()
{
pathApps.clear();
pathBatc.clear();
pathBina.clear();
pathEngi.clear();
pathLog .clear();
pathMout.clear();
pathMshp.clear();
pathPlac.clear();
pathStag.clear();
pathSupe.clear();
pathTrac.clear();
pathUlog.clear();
}

//------------------------------------------------------------------------------

bool CmPath::Cm_Path(bool OK,string & rpath,string newval, string oldval)
// If OK, rpath is set to newval, return FALSE
// Except if.... newval == "~", in which case rpath = oldval, return FALSE
// ..and if neither, return TRUE
{
if (OK) {                              // Replace path with a new valid value
  FileName::Force1Linux(newval);       // Linuxify it
  rpath = newval;
  return false;
}
if (newval == "~") {                   // Local reset of this path only
  rpath = oldval;
  return false;
}
return true;                           // Whatever it was, it isn't valid
}

//------------------------------------------------------------------------------

void CmPath::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sCmPath +++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
if (par==0) fprintf(fp,"%sOrchBase parent not defined\n",os);
else fprintf(fp,"%sOrchbase parent : %s\n",os,par->FullName().c_str());
fprintf(fp,"%spathApps : %s\n",os,pathApps.empty()?"---":pathApps.c_str());
fprintf(fp,"%spathBatc : %s\n",os,pathBatc.empty()?"---":pathBatc.c_str());
fprintf(fp,"%spathBina : %s\n",os,pathBina.empty()?"---":pathBina.c_str());
fprintf(fp,"%spathEngi : %s\n",os,pathEngi.empty()?"---":pathEngi.c_str());
fprintf(fp,"%spathLog  : %s\n",os,pathLog .empty()?"---":pathLog .c_str());
fprintf(fp,"%spathPlac : %s\n",os,pathPlac.empty()?"---":pathPlac.c_str());
fprintf(fp,"%spathStag : %s\n",os,pathStag.empty()?"---":pathStag.c_str());
fprintf(fp,"%spathSupe : %s\n",os,pathSupe.empty()?"---":pathSupe.c_str());
fprintf(fp,"%spathTrac : %s\n",os,pathTrac.empty()?"---":pathTrac.c_str());
fprintf(fp,"%spathUlog : %s\n",os,pathUlog.empty()?"---":pathUlog.c_str());
fprintf(fp,"%sCmPath -------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

FILE * CmPath::Fclose()
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

FILE * CmPath::Fopen()
// Called (by Root::ProcCmnd) at the start of every command, to set the detail
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

void CmPath::GenerateMode()
// Generate the output mode
{
if (pathUlog.empty()) {                // Default eveything to the console
  OuMode = Ou_Stdo;
  return;
}
                                       // If the last character is a '/', it's
                                       // a path.....
if (pathUlog[pathUlog.size()-1]==char('/')) {    // It's a path....
  FileName Fn(pathUlog+"base");        // Sanity check:
  if (Fn.Err()) {                      // Are we?
    OuMode = Ou_Stdo;                  // Cockup
    par->Post(239,pathUlog);           // Not a valid filepath
    return;
  }
  OuMode = Ou_Lolf;                    // Lots of little files
  return;
}
                                       // OK, it's a complete filename
FileName Fn(pathUlog);                 // Just checking
if (Fn.Err()) {
  OuMode = Ou_Stdo;                    // Cockup
  par->Post(241,pathUlog);             // Not a valid file
  return;
}
OuMode = Ou_Ofgf;                      // One fucking great file
}

//------------------------------------------------------------------------------

FILE * CmPath::GetLolfp()
// Generate (and open) a filename just for the upcoming. We don't yet know what
// it is, so all we can do is generate a generic name and timestamp it
{
// Slow time
time_t timeNtv;
time(&timeNtv);
char timeBuf[sizeof "YYYY-MM-DDTHH:MM:SS"];
strftime(timeBuf,sizeof timeBuf,"%Y_%m_%dT%H_%M_%S",localtime(&timeNtv));
string T = pathUlog + "Microlog_" + string(timeBuf);
FILE * fp = 0;
unsigned inc = 0;
string tT;
// Find file that doesn't exist (to avoid clobbering other logs.
while (fp==0) {
  tT=T+"p"+uint2str(inc)+".plog";
  if ((fp = fopen(tT.c_str(),"r"))) {
    inc++;
    fclose(fp);
    fp=0;
    if (inc>1000) break;           // Don't get stuck forever.
  }
  else break;
}
// Try to open it (mode change). The path might not exist? (inc>1000)
if (!(fp = fopen(tT.c_str(),"w"))) {
  OuMode = Ou_Stdo;                // Cockup
  par->Post(246,tT);
  return stdout;
}
lastfile = tT;                     // Save last file generated
return fp;                         // It's all good to go
}

//------------------------------------------------------------------------------

FILE * CmPath::GetOfgfp()
// The full name and path has been supplied by the user, and could be anything
// But we don't care, we just (try to) open it for append and let the monkey
// get on with it
{
FILE * fp = fopen(pathUlog.c_str(),"a");
if (fp==0) {                           // Cockup?
  par->Post(241,pathUlog);
  lastfile.clear();
  return stdout;
}
lastfile = pathUlog;
return fp;                             // You asked for it, you got it
}

//------------------------------------------------------------------------------

void CmPath::Reset()
// Reset all the paths from whatever values the user has put them back to
// "factory defaults"
{
Root * pR = dynamic_cast<Root *>(par); // Config manager is in Root
pathApps = pR->pOC->Apps();
pathBatc = pR->pOC->Batch();
pathBina = pR->pOC->Binaries();
pathEngi = pR->pOC->Engine();
pathLog  = pR->pOC->Log();
pathMout = pR->pOC->RemoteOut();
pathMshp = pR->pOC->RemoteMshp();
pathPlac = pR->pOC->Place();
pathStag = pR->pOC->Stage();
pathSupe = pR->pOC->Supervisors();
pathTrac = pR->pOC->Trace();
pathUlog = pR->pOC->Ulog();

UpdateMotherships();
}

//------------------------------------------------------------------------------

void CmPath::Show(FILE * fp)
{
fprintf(fp,"\nUser-defined filepath details:\n");
fprintf(fp,"Applications           : %s\n",pathApps.empty()?"---":pathApps.c_str());
fprintf(fp,"Batch files            : %s\n",pathBatc.empty()?"---":pathBatc.c_str());
fprintf(fp,"Final binaries         : %s\n",pathBina.empty()?"---":pathBina.c_str());
fprintf(fp,"Engine definitions     : %s\n",pathEngi.empty()?"---":pathEngi.c_str());
fprintf(fp,"Log files              : %s\n",pathLog .empty()?"---":pathLog .c_str());
fprintf(fp,"Remote app output      : %s\n",pathMout.empty()?"---":pathMout.c_str());
fprintf(fp,"Remote mothership files: %s\n",pathMshp.empty()?"---":pathMshp.c_str());
fprintf(fp,"Placement control      : %s\n",pathPlac.empty()?"---":pathPlac.c_str());
fprintf(fp,"Binary stageing        : %s\n",pathStag.empty()?"---":pathStag.c_str());
fprintf(fp,"Supervisor binaries    : %s\n",pathSupe.empty()?"---":pathSupe.c_str());
fprintf(fp,"Trace files            : %s\n",pathTrac.empty()?"---":pathTrac.c_str());
fprintf(fp,"MicroLog files         : %s\n",pathUlog.empty()?"---":pathUlog.c_str());
fprintf(fp,"\n");
fflush(fp);
}

//------------------------------------------------------------------------------

/* Pushes pathMout to Motherships. Note - pathMshp isn't actually used by the
 * Mothership. */
void CmPath::UpdateMotherships()
{
    PMsg_p out;
    out.Src(par->Urank);
    out.Key(Q::PATH);
    out.Put(0,&pathMout);

    /* Send to all Motherships, if any. */
    std::vector<ProcMap::ProcMap_t>::iterator procIt;
    for (procIt=par->pPmap->vPmap.begin();
         procIt!=par->pPmap->vPmap.end(); procIt++)
        if (procIt->P_class != csMOTHERSHIPproc) out.Send(procIt->P_rank);
}

//------------------------------------------------------------------------------

unsigned CmPath::operator()(Cli * pC)
// Handle "path" command from the monkey.
// We look at the parameter list before the clause, solely because it makes the
// coding easier. If it's a dud path, we store the dud-ness (OK), in case the
// clause needs it. If the command evolves onto anything more complex we may
// need to change this.
{
//printf("CmPath_t operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
Root * pR = dynamic_cast<Root *>(par); // Config manager is in Root
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause name
  string sPa = (*i).GetP();            // Pull out new path
  FileName Fn(sPa+"xxxx");             // Create a temporary 'legal' filepath
  bool OK = !Fn.Err();                 // Do it make sense?
  if (strcmp(sCl.c_str(),"apps")==0) {
    if (Cm_Path(OK,pathApps,sPa,pR->pOC->Apps())) par->Post(239,sCl);
    continue;
  }
  if (sCl=="batc") {
    if (Cm_Path(OK,pathBatc,sPa,pR->pOC->Batch())) par->Post(239,sCl);
    continue;
  }
  if (sCl=="bina") {
    if (Cm_Path(OK,pathBina,sPa,pR->pOC->Binaries())) par->Post(239,sCl);
    continue;
  }
  if (sCl=="clea") {
    Clear();
    continue;
  }
  if (sCl=="engi") {
    if (Cm_Path(OK,pathEngi,sPa,pR->pOC->Engine())) par->Post(239,sCl);
    continue;
  }
  if (sCl=="log" ) {
    if (Cm_Path(OK,pathLog,sPa,pR->pOC->Log())) par->Post(239,sCl);
    continue;
  }
  if (sCl=="plac") {
    if (Cm_Path(OK,pathPlac,sPa,pR->pOC->Place())) par->Post(239,sCl);
    if (par->pPlacer != PNULL) par->pPlacer->outFilePath = pathPlac;
    continue;
  }
  if (sCl=="rese") {
    Reset();
    continue;
  }
  if (sCl=="mout") {
    if (Cm_Path(OK,pathMout,sPa,pR->pOC->RemoteOut())) par->Post(239,sCl);
    else UpdateMotherships();
    continue;
  }
  if (sCl=="mshp") {
    if (Cm_Path(OK,pathMshp,sPa,pR->pOC->RemoteMshp())) par->Post(239,sCl);
    continue;
  }
  if (sCl=="stag") {
    if (Cm_Path(OK,pathStag,sPa,pR->pOC->Stage())) par->Post(239,sCl);
    if (par->pComposer != PNULL) par->pComposer->setOutputPath(pathStag);
    continue;
  }
  if (sCl=="supe") {
    if (Cm_Path(OK,pathSupe,sPa,pR->pOC->Supervisors())) par->Post(239,sCl);
    continue;
  }
  if (sCl=="trac") {
    if (Cm_Path(OK,pathTrac,sPa,pR->pOC->Trace())) par->Post(239,sCl);
    continue;
  }
  if (sCl=="ulog") {
    if (Cm_Path(OK,pathUlog,sPa,pR->pOC->Ulog())) par->Post(239,sCl);
    continue;
  }
  par->Post(25,sCl,"path");            // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================

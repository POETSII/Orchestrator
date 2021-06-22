//------------------------------------------------------------------------------

#include "CmSyst.h"
#include "macros.h"
#include "Root.h"
#include "Pglobals.h"
#include "Apps_t.h"
#include "DS_integ.h"

//==============================================================================

CmSyst::CmSyst(OrchBase * p):par(p)
{

}

//------------------------------------------------------------------------------

CmSyst::~CmSyst()
{
}

//------------------------------------------------------------------------------

void CmSyst::Dump(FILE * fp)
{
fprintf(fp,"CmSyst+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

fprintf(fp,"CmSyst-------------------------------------------------------\n\n");
fflush(fp);

}

//------------------------------------------------------------------------------

void CmSyst::SyInte(Cli::Cl_t Cl)
// Launch datastructure integrity check. It's tucked away in 'system' because
// it could so easily get very slow. As DBT puts it: This is where Time goes
// to die.
{
long t0 = mTimer();                    // Start outer wallclock
                                       // Integrity check ON everything
WALKMAP(string,Apps_t *,Apps_t::Apps_m,i) {
  par->Post(252,(*i).first);
  long t1 = mTimer();                  // Start inner wallclock
  DS_integ Xi('*',(*i).second,par);    // Integrity check FOR everything
  unsigned ecnt = Xi.ErrCnt();
  unsigned wcnt = Xi.WarCnt();
  par->Post(251,(*i).first,uint2str(ecnt),uint2str(wcnt),long2str(mTimer(t1)));
}
par->Post(253,long2str(mTimer(t0)));
}

//------------------------------------------------------------------------------

void CmSyst::SyPing(Cli::Cl_t Cl)
//
{
if (Cl.Pa_v.size()==0) {
  par->Post(48,"ping","system","1");
  return;
}
for(unsigned k=0;k<4;k++) {
//Post(27,uint2str(k));
  WALKVECTOR(Cli::Pa_t,Cl.Pa_v,i) {  // Walk the parameter (ping) list
    string tgt = (*i).Va_v[0];         // Class name to be pinged
    if (Cli::StrEq(tgt,par->Sderived)) continue;       // Can't ping yourself
    WALKVECTOR(ProcMap::ProcMap_t,par->pPmap->vPmap,j) { // Walk process list
                                       // Still can't ping self
      if (Cli::StrEq((*j).P_class,par->Sderived)) continue;
      if ((Cli::StrEq(tgt,(*j).P_class))||(tgt=="*")) {
        PMsg_p Pkt;
        Pkt.Put(1,&((*j).P_class));    // Target process name
        string tD(GetDate());
        string tT(GetTime());
        Pkt.Put(2,&tD);                // Never actually used these
        Pkt.Put(3,&tT);
        Pkt.Put<unsigned>(4,&k);       // Ping attempt
        Pkt.Src(par->Urank);           // Sending rank
        Pkt.Key(Q::SYST,Q::PING,Q::REQ);
        Pkt.Send((*j).P_rank);
      }
    }
  }
}
}

//------------------------------------------------------------------------------

void CmSyst::SyRun(Cli::Cl_t Cl)
// Spawn a group of processes and connect to them. The run command should look like
// either: 1) a list of triplets: number of procs, hostname, executable; or 2) a
// file specification containing lines of this form. For this first version we
// are not allowing command-line arguments, although this capacity could be
// retrofitted if deemed essential.
{
// no arguments: bad command
if (Cl.Pa_v.size()==0) {
  par->Post(48,"run","system","1");
  return;
}
// 1 argument: a file
if (Cl.Pa_v.size()==1) {
  return;
}
if (Cl.Pa_v.size()==2) {
  par->Post(48,"run","system","1");
  return;
}
if (Cl.Pa_v.size()%3!=0) {
  par->Post(48,"ping","system","1");
  return;
}
}

//------------------------------------------------------------------------------

void CmSyst::SyShow(Cli::Cl_t)
// Monkey wants the list of processes
{
printf("Orchestrator processes\n");
WALKVECTOR(ProcMap::ProcMap_t,par->pPmap->vPmap,i)
  printf("Rank %02d, %35s, created %s %s\n",
      (*i).P_rank,(*i).P_class.c_str(),(*i).P_TIME.c_str(),(*i).P_DATE.c_str());
fflush(stdout);

// More detailed information in the microlog
par->pPmap->Show(fd);
}

//------------------------------------------------------------------------------

void CmSyst::SyTime(Cli::Cl_t)
// Monkey wants the time
{
string sT = GetTime();
string sD = GetDate();
par->Post(26,sD,sT);
}

//------------------------------------------------------------------------------

unsigned CmSyst::operator()(Cli * pC)
// Handle "system" command from the monkey
{
if (pC==0) return 0;                   // Paranoia
fd = par->fd;
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause name
  if (sCl=="ping") { SyPing(*i); continue; }
  if (sCl=="run" ) { SyRun(*i);  continue; }
  if (sCl=="show") { SyShow(*i); continue; }
  if (sCl=="time") { SyTime(*i); continue; }
//  if (strcmp(scl.c_str(),"trac")==0) { Trac(*i); continue; }
  if (sCl=="inte") { SyInte(*i); continue; }
  par->Post(25,sCl,"system");          // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================

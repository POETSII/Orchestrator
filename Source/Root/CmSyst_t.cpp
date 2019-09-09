//------------------------------------------------------------------------------

#include "CmSyst_t.h"
#include "macros.h"
#include "root.h"
#include "Pglobals.h"

//==============================================================================

CmSyst_t::CmSyst_t(Root * p):par(p)
{

}

//------------------------------------------------------------------------------

CmSyst_t::~CmSyst_t()
{
}

//------------------------------------------------------------------------------

void CmSyst_t::Dump(FILE * fp)
{
fprintf(fp,"CmSyst_t++++++++++++\n");

fprintf(fp,"CmSyst_t------------\n\n");
fflush(fp);

}

//------------------------------------------------------------------------------

void CmSyst_t::Conn(Cli::Cl_t Cl)
// Connect to a group of processes identified by a service name in the parameter
{
if (Cl.Pa_v.size()==0) {
  par->Post(47,"conn","system","1"); // need to be given a service to connect to
  return;
}
if (par->pPmap[0]->vPmap.size() != par->Usize[0])
{
  par->Post(62); // can't connect to another universe before we know the size of our own.
}
string svc = Cl.Pa_v[0].Val; // get the service name
// connection is collective, so we have to organise everyone
WALKVECTOR(ProcMap::ProcMap_t,par->pPmap[0]->vPmap,j) {   // Walk the local process list
  if (Cli::StrEq((*j).P_class,par->Sderived)) continue;   // No need to notify self
  PMsg_p Pkt;
  Pkt.comm = par->Comms[0];           // local comm
  Pkt.Put(0,&svc);               // Target service name
  Pkt.Src(par->Urank);                // Sending rank
  Pkt.Key(Q::SYST,Q::CONN);      // It's a connection request
  Pkt.Send((*j).P_rank);
}
if (par->Connect(svc))
   par->Post(60,svc.c_str());
}

//------------------------------------------------------------------------------

void CmSyst_t::Ping(Cli::Cl_t Cl)
//
{
if (Cl.Pa_v.size()==0) {
  par->Post(48,"ping","system","1");
  return;
}
for(unsigned k=0;k<4;k++) {
//Post(27,uint2str(k));
  WALKVECTOR(Cli::Pa_t,Cl.Pa_v,i) {  // Walk the parameter (ping) list
//    printf("%s\n",(*i).Val.c_str()); fflush(stdout);
    string tgt = (*i).Val;             // Class name to be pinged
    if (Cli::StrEq(tgt,par->Sderived)) continue;           // Can't ping yourself
    unsigned cIdx = 0;
    for (; cIdx < par->Comms.size(); cIdx++)               // check all communicators
    {
       WALKVECTOR(ProcMap::ProcMap_t,par->pPmap[cIdx]->vPmap,j) { // Walk process list
         if (Cli::StrEq((*j).P_class,par->Sderived)) continue;// Still can't ping self
         if ((Cli::StrEq(tgt,(*j).P_class))||(tgt=="*")) {
           PMsg_p Pkt;
           Pkt.comm = par->Comms[cIdx];
           Pkt.Put(1,&((*j).P_class)); // Target process name
           string tD(GetDate());
           string tT(GetTime());
           Pkt.Put(2,&tD);             // Never actually used these
           Pkt.Put(3,&tT);
           Pkt.Put<unsigned>(4,&k);    // Ping attempt
           Pkt.Src(par->Urank);             // Sending rank
           Pkt.Key(Q::SYST,Q::PING,Q::REQ);
           Pkt.Send((*j).P_rank);
         }
       }
    }
  }
}
}

//------------------------------------------------------------------------------

void CmSyst_t::Run(Cli::Cl_t Cl)
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

void CmSyst_t::Show(Cli::Cl_t)
// Monkey wants the list of processes
{
//vector<string> vprocs;
//if (pPmap!=0) pPmap->GetProcs(vprocs);
vector<ProcMap::ProcMap_t> vprocs;
for (unsigned cIdx = 0; cIdx < par->Comms.size(); cIdx++)
{
if (par->pPmap[cIdx]!=0) par->pPmap[cIdx]->GetProcs(vprocs);
par->Post(29,uint2str(vprocs.size()),uint2str(cIdx));
par->Post(30,par->Sproc);
printf("\n");
printf("Processes for comm %d\n", cIdx);
//WALKVECTOR(string,vprocs,i) printf("%s\n",(*i).c_str());
WALKVECTOR(ProcMap::ProcMap_t,vprocs,i)
  printf("Rank %02d, %35s, created %s %s\n",
      (*i).P_rank,(*i).P_class.c_str(),(*i).P_TIME.c_str(),(*i).P_DATE.c_str());
printf("\n");
}
fflush(stdout);
}

//------------------------------------------------------------------------------

void CmSyst_t::Time(Cli::Cl_t)
// Monkey wants the time
{
string sT = GetTime();
string sD = GetDate();
par->Post(26,sD,sT);
}

//------------------------------------------------------------------------------

unsigned CmSyst_t::operator()(Cli * pC)
// Handle "system" command from the monkey
{
printf("CmSyst_t operator() splitter for ....\n");
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string scl = (*i).Cl;                // Pull out clause name
  if (strcmp(scl.c_str(),"conn")==0) { Conn(*i); continue; }
//  if (strcmp(scl.c_str(),"path")==0) { Path(*i); continue; }
  if (strcmp(scl.c_str(),"ping")==0) { Ping(*i); continue; }
  if (strcmp(scl.c_str(), "run")==0) { Run(*i);  continue; }
  if (strcmp(scl.c_str(),"show")==0) { Show(*i); continue; }
  if (strcmp(scl.c_str(),"time")==0) { Time(*i); continue; }
  par->Post(25,scl,"system");               // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//------------------------------------------------------------------------------

//==============================================================================


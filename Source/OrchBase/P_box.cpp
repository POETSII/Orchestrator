//------------------------------------------------------------------------------

#include "P_box.h"
#include "P_graph.h"
#include "Config_t.h"
#include "CommonBase.h"
#include "OrchBase.h"
#include "P_core.h"

//==============================================================================

P_box::P_box(P_graph * _p,string _s):par(_p),pMothBin(0),pConfig(0)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
pMothBin=0;
}

//------------------------------------------------------------------------------

P_box::~P_box()
{
if (pMothBin!=0) delete pMothBin;
WALKVECTOR(P_board *,P_boardv,i) delete *i;
}

//------------------------------------------------------------------------------

void P_box::BoxDat_cb(P_box * const & p)
// Debug callback for vertex data
{
if (p!=0) fprintf(dfp,"box(D): %s",p->Name().c_str());
else fprintf(dfp,"box(D): ***");
fflush(dfp);
}

//------------------------------------------------------------------------------

void P_box::BoxKey_cb(unsigned const & u)
// Debug callback for vertex key
{
fprintf(dfp,"box(K): %03u",u);
fflush(dfp);
}

//------------------------------------------------------------------------------

void P_box::Build(Config_t * pCfg)
// Assemble the inner structure of the (single) box, as outlined in the pConfig
// object handed in.
{
if (pCfg==0) {
  par->par->Post(906);
  return;
}
pConfig = pCfg;                        // Save configuration backpointer
for (unsigned iboard=0;iboard<pCfg->GetBoards();iboard++) {
  P_board * pB = new P_board(this);
  pB->AutoName("Bo");                  // Derive the name off the uid
  pB->addr.SetBoard(iboard+1);         // Ordinal address number
  pB->addr |= addr;                    // Inherit parent box address field
  P_boardv.push_back(pB);
  for (unsigned icore=0;icore<pCfg->GetCores();icore++) {
    P_core * pC = new P_core(pB);
    pC->AutoName("Co");                // Derive the name off the uid
    pC->addr.SetCore(icore+1);         // Ordinal address number
    pC->addr |= pB->addr;              // Inherit parent board address field(s)
    pB->P_corev.push_back(pC);
    for (unsigned ithread=0;ithread<pCfg->GetThreads();ithread++) {
      P_thread * pT = new P_thread(pC);
      pT->AutoName("Th");              // Derive the name off the uid
      pT->addr.SetThread(ithread+1);   // Ordinal address number
      pT->addr |= pC->addr;            // Inherit parent core address field(s)
      pC->P_threadv.push_back(pT);
    }
  }
}



}

//------------------------------------------------------------------------------
            /*
void P_box::Clean0()
// Remove all the null thread->device crosslinks
{
WALKVECTOR(P_board *,P_boardv,i) (*i)->Clean0();
}
              */
//------------------------------------------------------------------------------

void P_box::Dump(FILE * fp)
{
string s = FullName();
fprintf(fp,"P_box %35s+++++++++++++++++++++++++++++++++++++++\n",s.c_str());
fprintf(fp,"NameBase %s\n",FullName().c_str());
fprintf(fp,"Parent         %#08p\n",par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());
if (pMothBin!=0) pMothBin->Dump(fp);
else fprintf(fp,"No mothership binary (supervosir) defined\n");
fprintf(fp,"BOARDS IN BOX %35s+++++++++++++++++++++++++++++++\n",s.c_str());
WALKVECTOR(P_board *,P_boardv,i) (*i)->Dump(fp);
fprintf(fp,"BOARDS IN BOX %35s-------------------------------\n",s.c_str());
NameBase::Dump(fp);
DumpChan::Dump(fp);
fprintf(fp,"P_box %35s---------------------------------------\n",s.c_str());
fflush(fp);
}

//==============================================================================




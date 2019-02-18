//------------------------------------------------------------------------------

#include "P_owner.h"

//==============================================================================

P_owner::P_owner()
{

}

//------------------------------------------------------------------------------
  
P_owner::P_owner(string s)
{
Name(s);
}

//------------------------------------------------------------------------------

P_owner::~P_owner()
{

}

//------------------------------------------------------------------------------

void P_owner::Disown()
// To disconnect a particular owner from every task it owns
{
WALKVECTOR(P_task *,P_taskv,i) (*i)->pOwn=0;     // Kill the backpointers
P_taskv.clear();                       // Kill the forward pointers
}

//------------------------------------------------------------------------------

void P_owner::Disown(P_task * pT)
// To disconnect a particular owner from a particular task
{
if (pT->pOwn!=this) return;            // It wasn't connected to me anyway
pT->pOwn = 0;                          // Kill the backpointer
WALKVECTOR(P_task *,P_taskv,i)
  if ((*i)==pT) {
    P_taskv.erase(i);                  // Must return now: loop iterator is bad
    return;
  }
}

//------------------------------------------------------------------------------

void P_owner::Dump(FILE * fp)
{
fprintf(fp,"P_owner+++++++++++++++++++++++++++++++++\n");
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me             %#018lx\n", (uint64_t) this);
fprintf(fp,"...owns:\n");
WALKVECTOR(P_task *,P_taskv,i) fprintf(fp,"%s\n",(*i)->FullName().c_str());
NameBase::Dump(fp);
fprintf(fp,"P_owner---------------------------------\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void P_owner::Own(P_task * pT)
// To connect an owner to a task
{
if (pT==0) return;                     // Paranoia
if (pT->pOwn!=0) pT->pOwn->Disown(pT); // It already has an owner - kill link
pT->pOwn = this;                       // Back pointer to me
P_taskv.push_back(pT);                 // Forward pointer to task
}

//==============================================================================




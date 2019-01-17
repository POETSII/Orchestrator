//
// This is NOT a translation unit. Do not #include stuff here.
//
//==============================================================================

unsigned OrchBase::CmLink(Cli * pC)
// Handle link command from monkey. It's already been trimmed.
{
if (pC->Cl_v.empty()) return 0;        // Nothing to do
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {
  const char * cs = (*i).Cl.c_str();
  if (strcmp(cs,"cons")==0) { LinkCons(*i); continue; }
  if (strcmp(cs,"dump")==0) { LinkDump(*i); continue; }
  if (strcmp(cs,"link")==0) { LinkLink(*i); continue; }
  if (strcmp(cs,"nser")==0) { LinkNser(*i); continue; }
  if (strcmp(cs,"path")==0) { LinkPath(*i); continue; }
  if (strcmp(cs,"plac")==0) { LinkPlac(*i); continue; }
  if (strcmp(cs,"unli")==0) { LinkUnli(*i); continue; }
  Post(25,(*i).Cl,"link");
}
return 0;
}

//------------------------------------------------------------------------------

void OrchBase::LinkCons(Cli::Cl_t Cl)
//
{

}

//------------------------------------------------------------------------------

void OrchBase::LinkDump(Cli::Cl_t Cl)
// Provide a link-specific dump. The monkey gets
// 1. A list of all non-empty threads, with a device map
// 2. The inverse: a list of all linked tasks, with a thread map
{
FILE * fp = stdout;                    // For now
string s = "Console";                  // For now
fprintf(fp,"link /dump %35s++++++++++++++++++++++++++++++++++++++\n",s.c_str());
if (pE == 0) fprintf(fp,"No topology graph\n");
else if (pE->is_empty()) fprintf(fp,"Topology graph empty\n");
else {
  fprintf(fp,"Devices on threads :\n");

  // Iterate over all threads.
  WALKPDIGRAPHNODES(AddressComponent, P_board*,
                    unsigned, P_link*,
                    unsigned, P_port*, pE->G,boardIterator)
  {
    WALKPDIGRAPHNODES(AddressComponent, P_mailbox*,
                      unsigned, P_link*,
                      unsigned, P_port*,
                      pE->G.NodeData(boardIterator)->G,mailboxIterator)
    {
      WALKMAP(AddressComponent, P_core*,
              pE->G.NodeData(boardIterator)->
              G.NodeData(mailboxIterator)->P_corem, coreIterator)
      {
        WALKMAP(AddressComponent, P_thread*,
                coreIterator->second->P_threadm, threadIterator)
        {

          if (!threadIterator->second->P_devicel.empty())
          {
            fprintf(fp,"Thread %s : %u devices\n",
                    threadIterator->second->FullName().c_str(),
                    threadIterator->second->P_devicel.size());

            WALKLIST(P_device*, threadIterator->second->P_devicel,
                     deviceIterator)
            {
              fprintf(fp, "       %s\n",
                      (*deviceIterator)->FullName().c_str());
            }
          }
        }
      }
    }
  }

  fprintf(fp,"Threads on devices :\n");
  if (P_taskm.empty()) fprintf(fp,"No tasks loaded\n");
  else {
    WALKMAP(string,P_task *,P_taskm,i) {
      D_graph * pgr = ((*i).second)->pD;
      fprintf(fp,"Task %s\n",(*i).first.c_str());
      if (pgr->G.SizeNodes()==0) fprintf(fp,"...is empty\n");
      else {
        WALKPDIGRAPHNODES(unsigned,P_device *,unsigned,P_message *,
                          unsigned,P_pin *,pgr->G,i) {
          P_device * pDe = pgr->G.NodeData(i);
            if (pDe->pP_thread!=0)
              fprintf(fp,"Device %s : thread %s\n",
                    pDe->FullName().c_str(),pDe->pP_thread->FullName().c_str());
        }
      }
    }
  }
}
fprintf(fp,"link /dump %35s--------------------------------------\n",s.c_str());
fflush(fp);
}

//------------------------------------------------------------------------------

void OrchBase::LinkLink(Cli::Cl_t Cl)
// Routine to cross-link a task. The task must be loaded, and a topology
// defined.
{
if (Cl.Pa_v.empty()) return;           // Nothing to do
string st = Cl.Pa_v[0].Val;            // Task to load
if (pP==0) {                           // No topology?
  Post(151,st);
  return;
}
if (pP->IsEmpty()) {                   // Still no topology?
  Post(151,st);
  return;
}
if (P_taskm.find(st)==P_taskm.end()) { // No task?
  Post(152,st);
  return;
}
P_task   * pT  = P_taskm[st];          // We know it's there
if (pT->linked) {                      // Task already linked?
  Post(153,st);
  return;
}
                                       // OK, good to go:

/*

               MOVED THE LOOP BELOW INTO THE PLACEMENT CLASS AND
               MODIFIED TO PLACE ONE DEVICE TYPE PER CORE

// WALKPDIGRAPHNODES(unsigned,P_device *,unsigned,P_message *,unsigned,P_pin *,pD->G,i)
//  if (i!=pD->G.NodeEnd()) {
//    P_device * pDe = pD->G.NodeData(i);// For each device.....
//    pPlace->GetNext(pTh,pCo,pBo,pBx);  // ...get a thread
//    pPlace->Xlink(pDe,pTh);            // And link them
//  }

*/
// Place and set the link flag if the placement was successful.
if (!pPlace->Place(pT)) pT->LinkFlag();


/*
unsigned lim = (*pP->pConfigl.begin())->ThreadsPerBox();
for(unsigned i=0;i<lim*4;i++) {
  bool f = pPlace->GetNext(pTh,pCo,pBo,pBx);
  printf("%c Th:%s Co:%s Bo:%s Bx:%s\n",f ? 'X' : ' ',
          pTh->FullName().c_str(),pCo->FullName().c_str(),
          pBo->FullName().c_str(),pBx->FullName().c_str());
  fflush(stdout);
}
*/
}

//------------------------------------------------------------------------------

void OrchBase::LinkNser(Cli::Cl_t Cl)
// Command from monkey to upload a linked task to the nameserver
{
Cl.Dump();
if (Cl.Pa_v.empty()) return;           // Nothing to do
string st = Cl.Pa_v[0].Val;            // Task to upload
if (P_taskm.find(st)==P_taskm.end()) { // No task there?
  Post(156,st);
  return;
}
P_task * pT  = P_taskm[st];            // OK, it's there
if (!pT->linked) {                     // Task not linked anyway?
  Post(157,st);
  return;
}
if (pT->pOwn==0) {                      // Task has no owner?
  Post(158,st);
  return;
}
Ns_el ns;                              // OK, so let's do it
ns.PutT(st,pT->pOwn->Name(),Id());     // The task itself
                                       // Walk the devices
WALKPDIGRAPHNODES(unsigned,P_device *,unsigned,P_message *,unsigned,P_pin *,
  pT->pD->G,i) {
  P_device * pD = pT->pD->G.NodeData(i);
//  vector<string> inpn = pD->NSGetinpn();
//  vector<string> inpt = pD->NSGetinpt();
//  vector<string> oupn = pD->NSGetoupn();
//  vector<string> oupt = pD->NSGetoupt();
pD->Dump();
pD->pP_devtyp->Dump();
pT->Dump();
pT->pOwn->Dump();
pD->pP_thread->dump();
pD->pP_thread->parent->dump();

string s0 = pD->Name();
string s1 = pD->pP_devtyp->Name();
string s2 = pT->pSup->Name();
string s3 = st;
string s4 = pT->pOwn->Name();
vector<string> vs0 = pD->NSGetinpn();
vector<string> vs1 = pD->NSGetinpt();
vector<string> vs2 = pD->NSGetoupn();
vector<string> ns3 = pD->NSGetoupt();
P_addr_t p0 = pD->addr;

  ns.PutD(pD->Name(),                  // Device name
          pD->pP_devtyp->Name(),       // Device type
          pT->pSup->Name(),            // Supervisor name
          st,                          // Task name
          pT->pOwn->Name(),            // Owner name
          pD->NSGetinpn(),             // Vector of input pin names
          pD->NSGetinpt(),             // Vector of input pin types
          pD->NSGetoupn(),             // Vector of output pin names
          pD->NSGetoupt(),             // Vector of output pin types
          pD->Id(),                    // Device ID
          pD->addr,                    // Hardware address
          pD->attr,                    // Attribute
          pD->pP_thread->parent->instructionBinary->Id());   // Binary file ID
}
ns.Put<unsigned>(0,&(ns.keyv));               // Key vector
ns.Key(Q::CANDC,Q::LOAD);
ns.Src(pPmap[0]->U.Root);     // Operating as root we must be in the local pMap.
int C;
if ((C = NameSCIdx()) < 0) Post(162);
else
{
   ns.comm = Comms[C];
   ns.Tgt(pPmap[C]->U.NameServer);
   ns.Send();
}
}

//------------------------------------------------------------------------------

void OrchBase::LinkPath(Cli::Cl_t Cl)
//
{

}

//------------------------------------------------------------------------------

void OrchBase::LinkPlac(Cli::Cl_t Cl)
//
{

}

//------------------------------------------------------------------------------

void OrchBase::LinkUnli(Cli::Cl_t Cl)
// Routine to unlink either something or everything
{
if (Cl.Pa_v.empty()) return;           // Nothing to do
string st = Cl.Pa_v[0].Val;            // Task to unload
if (st=="*") {                         // We unlink everything?
  UnlinkAll();
  return;
}
if (P_taskm.find(st)==P_taskm.end()) { // No task there anyway?
  Post(154,st);
  return;
}
P_task * pT  = P_taskm[st];            // Just the one, then. We know it's there
if (!pT->linked) {                     // Task not linked anyway?
  Post(155,st);
  return;
}
Unlink(st);                            // OK, it's there and linked. Unlink it
}

//------------------------------------------------------------------------------

void OrchBase::UnlinkAll()
// As the name implies; we don't need to worry about tidying anything up,
// 'cos it's all got to go. No side effects or stuff left out here - on exit,
// the device <-> thread links are all deleted. Not ethe placement moduke needs
// not know - the topology is left untouched.
{

if (pE == 0) return;
else if (pE->is_empty()) return;
else {
  // Iterate over all threads.
  WALKPDIGRAPHNODES(AddressComponent, P_board*,
                    unsigned, P_link*,
                    unsigned, P_port*, pE->G,boardIterator)
  {
    WALKPDIGRAPHNODES(AddressComponent, P_mailbox*,
                      unsigned, P_link*,
                      unsigned, P_port*,
                      pE->G.NodeData(boardIterator)->G,mailboxIterator)
    {
      WALKMAP(AddressComponent, P_core*,
              pE->G.NodeData(boardIterator)->
              G.NodeData(mailboxIterator)->P_corem, coreIterator)
      {
        WALKMAP(AddressComponent, P_thread*,
                coreIterator->second->P_threadm, threadIterator)
        {
            threadIterator->second->P_devicel.clear();
        }
      }
    }
  }
}

if (P_taskm.empty()) return;           // No tasks at all
WALKMAP(string,P_task *,P_taskm,i) {   // Walk them that's there
  P_task * pTa = (*i).second;          // Task
  if (!pTa->linked) continue;          // Not linked anyway - skip it
  pTa->linked = false;                 // Not linked now
  D_graph * pDg = pTa->pD;             // Device graph
  WALKPDIGRAPHNODES(unsigned,P_device *,unsigned,P_message *,
                    unsigned,P_pin *,pDg->G,j)
    pDg->G.NodeData(j)->pP_thread=0;   // Kill link into threads
}

}

//------------------------------------------------------------------------------

void OrchBase::Unlink(string tname)
// Unlink a single named task. We know it's there (modulo paranoia) so we don't
// report anything
{
if (P_taskm.empty()) return;           // Just in case
if (P_taskm.find(tname)==P_taskm.end()) return;
P_task * pTa = P_taskm[tname];         // Finally get the actual task
if (!pTa->linked) return;              // Not linked anyway - skip it
pTa->linked = false;                   // Not linked now
D_graph * pDg = pTa->pD;               // Device graph - walk it
set<P_thread *> touched;
WALKPDIGRAPHNODES(unsigned,P_device *,unsigned,P_message *,
                  unsigned,P_pin *,pDg->G,i) {
  P_device * pDe = pDg->G.NodeData(i); // Each device.....
  P_thread * pTh = pDe->pP_thread;     // Linked thread
  if (pTh==0) Post(907,pDe->FullName());
  WALKLIST(P_device *,pTh->P_devicel,j) if ((*j)==pDe) {
    (*j)=0;
    touched.insert(pTh);
  }
}
if (pP==0) Post(908);
WALKSET(P_thread *,touched,i) (*i)->P_devicel.remove(0);

}

//==============================================================================

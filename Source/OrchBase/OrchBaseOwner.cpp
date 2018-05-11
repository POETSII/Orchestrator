//==============================================================================

unsigned OrchBase::CmOwne(Cli * pC)
// Handle owner command from monkey. It's already been trimmed.
{
if (pC->Cl_v.empty()) return 0;        // Nothing to do
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {
  const char * cs = (*i).Cl.c_str();
  if (strcmp(cs,"atta")==0) { OwneAtta(*i); continue; }
  if (strcmp(cs,"dump")==0) { OwneDump(*i); continue; }
  if (strcmp(cs,"show")==0) { OwneShow(*i); continue; }
//  if (strcmp(cs,"task")==0) { OwneTask(*i); continue; }
  Post(25,(*i).Cl,"link");
}
return 0;
}

//------------------------------------------------------------------------------

void OrchBase::OwneAtta(Cli::Cl_t Cl)
// Introduce/delete an owner, and/or attach/disconnect to a task
{
//Cl.Dump();
//WALKMAP(string,P_owner *,P_ownerm,i) (*i).second->Dump();

                                       // Do the owner bit first:
if (Cl.GetP().empty()) return;         // Empty command
string O = Cl.GetO();                  // Cache zeroth operator
string P = Cl.GetP();                  // Cache zeroth parameter

if ((O!="+")&&(O!="-")&&(!O.empty()))Post(159,O);// Just checking

map<string,P_owner *>::iterator iO;    // Putative owner
if (O=="-") {                          // Delete an owner
  iO = P_ownerm.find(P);               // Find it
  if (iO==P_ownerm.end()) {            // Not there anyway
    Post(161,P);                       // Complain
    return;
  }
  (*iO).second->Disown();              // Disconnect it
  P_ownerm.erase(P);                   // Kill it
  delete (*iO).second;
  return;
}

if (O=="+") {                          // Add an owner
  iO = P_ownerm.find(P);               // Find it
                                       // One does not pre-exist? Create it
  if (iO==P_ownerm.end()) {
    P_ownerm[P] = new P_owner(P);
    iO = P_ownerm.find(P);
  }
}
                                       // Here iff the owner exists (it's iO)
if (Cl.Pa_v.size()==1) return;         // Nothing to do: clause list exhausted
                                       // Here if we want to modulate the
                                       // task ownerships
for (unsigned i=1;!Cl.GetP(i).empty();i++) {     // Walk the parameters (tasks)
  string Pi = Cl.GetP(i);              // Cache the parameter (task name)
  string Oi = Cl.GetO(i);              // Cache the operator
  if (P_taskm.find(Pi)==P_taskm.end()) {
    Post(160,Pi);                      // Task not there?
    continue;
  }
  P_task * pT = P_taskm[Pi];           // Get the task
  if (Oi=="+")(*iO).second->Own(pT);   // Own it
  if (Oi=="-")(*iO).second->Disown(pT);// Or not
}

//WALKMAP(string,P_owner *,P_ownerm,i) (*i).second->Dump();

}

//------------------------------------------------------------------------------

void OrchBase::OwneDump(Cli::Cl_t Cl)
{
Cl.Dump();
WALKMAP(string,P_owner *,P_ownerm,i) (*i).second->Dump();
}

//------------------------------------------------------------------------------

void OrchBase::OwneShow(Cli::Cl_t Cl)
//
{
Cl.Dump();
WALKMAP(string,P_owner *,P_ownerm,i) (*i).second->Dump();
}

//==============================================================================




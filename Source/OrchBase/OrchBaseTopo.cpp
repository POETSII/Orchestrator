//
// This is NOT a translation unit. Do not #include stuff here.
//
//==============================================================================

void OrchBase::ClearBoxConfig()
{
}

//------------------------------------------------------------------------------

void OrchBase::ClearBoxConfig(string)
{
}

//------------------------------------------------------------------------------

void OrchBase::ClearTopo()
{
if (pE==0) return;
Post(134,pE->FullName());
delete pE;
pE = 0;
}

//------------------------------------------------------------------------------

unsigned OrchBase::CmTopo(Cli * pC)
// Handle topo command from monkey. It's already been trimmed.
{
if (pC->Cl_v.empty()) return 0;        // Nothing to do
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {
  const char * cs = (*i).Cl.c_str();
  if (strcmp(cs,"clea")==0) { TopoClea(*i);  continue; }
  if (strcmp(cs,"conf")==0) { TopoConf(*i);  continue; }
  if (strcmp(cs,"cons")==0) { TopoCons(*i);  continue; }
  if (strcmp(cs,"dump")==0) { TopoDump(*i);  continue; }
  if (strcmp(cs,"disc")==0) { TopoDisc(*i);  continue; }
  if (strcmp(cs,"load")==0) { TopoLoad(*i);  continue; }
  if (strcmp(cs,"mode")==0) { TopoMode(*i);  continue; }
  if (strcmp(cs,"path")==0) { TopoPath(*i);  continue; }
  if (strcmp(cs,"save")==0) { TopoSave(*i);  continue; }
  if (strcmp(cs,"set1")==0) { TopoSet1(*i);  continue; }
  if (strcmp(cs,"set2")==0) { TopoSet2(*i);  continue; }
  Post(25,(*i).Cl,"topo");
}
return 0;
}

//------------------------------------------------------------------------------

void OrchBase::TopoClea(Cli::Cl_t Cl)
// Routine to remove stuff from the topology description part of the database
{
                                       // Clear the topology datastructure
if (Cl.Pa_v.empty()) { ClearTopo(); return; }
WALKVECTOR(Cli::Pa_t,Cl.Pa_v,i) {      // Loop through things to remove
  string st = (*i).Val;                // Thing to remove
                                       // Clear all box configurations
  if (st=="*" ) { ClearBoxConfig(); continue; }
                                       // Clear everything
  if (st=="**") { ClearBoxConfig(); ClearTopo();    continue; }
                                       // Clear a specific named box config.
  ClearBoxConfig(st);
}
}

//------------------------------------------------------------------------------

void OrchBase::TopoConf(Cli::Cl_t Cl)
// Load a named box configuration file
{
if (Cl.Pa_v.empty()) return;
WALKVECTOR(Cli::Pa_t,Cl.Pa_v,i) {      // Loop through parameters
  string st = (*i).Val;
  Post(131,st);                        // That's all for now
}
}

//------------------------------------------------------------------------------


void OrchBase::TopoCons(Cli::Cl_t Cl)
// Apply constraints
{
if (Cl.Pa_v.size()<2) {               // Command make sense?
    Post(48,Cl.Cl,"topology","2");
    return;
}
string constraint = Cl.Pa_v[0].Val;     // Unpack constraint name
if (!pPlace->pCon) pPlace->pCon = new Constraints();
pPlace->pCon->Constraintm[constraint] = str2uint(Cl.Pa_v[1].Val);
}

//------------------------------------------------------------------------------


void OrchBase::TopoDump(Cli::Cl_t Cl)
{
if (pE==0){Post(139);return;}
if (Cl.Pa_v.empty()) pE->dump();
WALKVECTOR(Cli::Pa_t,Cl.Pa_v,i) {      // Loop through streams to dump to
  string st = (*i).Val;                // Dump stream
  if (st.empty()) pE->dump();
  st = topopath + st;
  FILE * fp = fopen(st.c_str(),"w");
  if (fp==0) Post(132,st);
  else {
    pE->dump(fp);
    fclose(fp);
  }
}

}

//------------------------------------------------------------------------------

void OrchBase::TopoDisc(Cli::Cl_t Cl)
// Discover the hardware topology
{
Post(133,"started");
Post(133,"complete");
}

//------------------------------------------------------------------------------

void OrchBase::TopoLoad(Cli::Cl_t Cl)
// Load a topology in from a file
{
    ClearTopo();
    pE = new P_engine("");
    pE->parent = this;
    std::string inputFilePath = Cl.Pa_v[0].Val;

    HardwareFileParser parser;
    try
    {
        parser.load_file(inputFilePath.c_str());
        parser.populate_hardware_model(pE);
        Post(140, inputFilePath.c_str());
    }
    catch (OrchestratorException& exception)
    {
        Post(141, inputFilePath.c_str(), ("\n" + exception.message).c_str());
        ClearTopo();
    }
}

//------------------------------------------------------------------------------

void OrchBase::TopoMode(Cli::Cl_t Cl)
{
Post(136);
}

//------------------------------------------------------------------------------

void OrchBase::TopoPath(Cli::Cl_t Cl)
{
Post(137,topopath);                    // Tell the monkey the current path
if (Cl.Pa_v.empty()) return;           // Nothing to do - go without change
topopath = Cl.Pa_v[0].Val;             // Update the path string
Post(103,taskpath);                    // Tell the monkey
}

//------------------------------------------------------------------------------

void OrchBase::TopoSave(Cli::Cl_t Cl)
{
Post(136);
}

//------------------------------------------------------------------------------

void OrchBase::TopoSet1(Cli::Cl_t Cl)
{
    ClearTopo();
    pE = new P_engine("Aesop [1 box]");
    pE->parent = this;
    AesopDeployer deployer;
    Post(138, pE->Name());
    deployer.deploy(pE);
}

//------------------------------------------------------------------------------

void OrchBase::TopoSet2(Cli::Cl_t Cl)
{
    ClearTopo();
    pE = new P_engine("Aesop [2 boxes]");
    pE->parent = this;
    MultiAesopDeployer deployer(2);
    Post(138, pE->Name());
    deployer.deploy(pE);
}

//==============================================================================

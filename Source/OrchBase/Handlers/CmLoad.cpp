//------------------------------------------------------------------------------

#include "CmLoad.h"
#include "OrchBase.h"
#include "OrchConfig.h"
#include "FileName.h"
#include "Apps_t.h"
#include "DS_XML.h"
#include "Root.h"
#include "XValid.h"
#include "Pglobals.h"
#include "SimpleDeployer.h"
#include "MultiSimpleDeployer.h"

//==============================================================================
/* This handles the "load" command. The only slight weirdness - see
documentation - is the XML Validator and builder. The class (CMLoad) gets heaved
into existance by the constructor of OrchBase, but the configuration file is
not read until the Root constructor is executed, so we can't load the validator
here because we don't know where the grammar definition file is. So we
load-on-demand when the user tries to execute "load" for the first time.
We do the same for the XML database builder, not because we have to, but
because of the intrinsic elegance of code symmetry.
*/

//------------------------------------------------------------------------------

CmLoad::CmLoad(OrchBase * p):par(p)
{
pXV     = 0;                           // We have no validation file
pXB     = 0;                           // Datastructure builder
}

//------------------------------------------------------------------------------

CmLoad::~CmLoad()
{
if (pXV!=0) delete pXV;
if (pXB!=0) delete pXB;
}

//------------------------------------------------------------------------------

void CmLoad::Cm_App(Cli::Cl_t cl)
// Load an xml application from a file into the Orchestrator database
{
string op = cl.GetO();                 // Optional operator
string fn = cl.GetP();                 // Not optional filename
if (fn.empty()) {                      // Filename not supplied?
  par->Post(250,cl.Cl);
  return;
}
                                       // Append pathname?
if (op==string("+")) fn = par->pCmPath->pathApps + fn;
if (!file_readable(fn.c_str())) {      // Can we read it?
  par->Post(112,fn);                   // Apparently not.....
  return;
}
                                      // Application file there already?
if (Apps_t::FindFile(fn)!=0) {
  par->Post(242,fn);
  return;
}
                                       // OK, good to go. fn==full i/p filename
par->Post(235,fn);
unsigned g_ecnt,c_ecnt,ecnt;
long t0 = mTimer();                    // ... start wallclock
pXV->Validate(fn);
pXV->ErrCnt(g_ecnt,c_ecnt);            // XML validator error counters
ecnt = g_ecnt + c_ecnt;                // Total errors = grammar + client
if (ecnt!=0) par->Post(201,"validation",uint2str(ecnt),long2str(mTimer(t0)));
else {
  pXB->PBuild(pXV->ClRoot());
  par->Post(65,fn,long2str(mTimer(t0)));
}

}

//------------------------------------------------------------------------------

void CmLoad::Cm_Engine(Cli::Cl_t cl)
{
    std::string op = cl.GetO(0);
    std::string parameter = cl.GetP(0);

    /* Whatever happens, we're clearing the topology. */
    par->ClearTopo();

    /* If no argument is passed to this command, the topology is cleared and
     * nothing else happens. */
    if (parameter.empty()) return;

    /* Otherwise, identify the engine to load according to the following
     * rules:
     *
     * - If there is no operator, use the parameter as a file path.
     *
     * - If the operator is a "+", use the parameter as a file name, and
     *   prepend that name with a previously-loaded engine path.
     *
     * - If the operator is a "?", load a prebaked "special" configuration.
     */

    if (op == std::string("?"))
    {
        if (parameter == "1_box_prototype")
        {
            par->pE = new P_engine("Simple [1 box]");
            par->pE->parent = par;
            par->pE->Npar(par);
            SimpleDeployer deployer;
            par->Post(138, par->pE->Name());
            deployer.deploy(par->pE);
        }

        else if (parameter == "2_box_prototype")
        {
            par->pE = new P_engine("Simple [2 boxes]");
            par->pE->parent = par;
            par->pE->Npar(par);
            MultiSimpleDeployer deployer(2);
            par->Post(138, par->pE->Name());
            deployer.deploy(par->pE);
        }

        else
        {
            par->Post(170, parameter);
            return;
        }

        par->PlacementReset();
        par->ComposerReset();
        par->BuildMshipMap();
    }

    else
    {
        std::string filePath;
        if (op == std::string("+"))
        {
            filePath = par->pCmPath->pathEngi + parameter;
        }
        else filePath = parameter;

        /* Let there be engine. */
        par->pE = new P_engine("");
        par->pE->parent = par;
        par->pE->Npar(par);

        HardwareFileReader reader;
        try
        {
            reader.load_file(filePath.c_str());
            reader.populate_hardware_model(par->pE);
            par->PlacementReset();
            par->ComposerReset();
            par->Post(140, filePath.c_str());
            par->BuildMshipMap();
        }
        catch (OrchestratorException& exception)
        {
            par->Post(141, filePath.c_str(),
                      ("\n" + exception.message).c_str());
            par->ClearTopo();
        }
    }
}

//------------------------------------------------------------------------------

void CmLoad::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sCmLoad +++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
if (par==0) fprintf(fp,"%sOrchBase parent not defined\n",os);
else fprintf(fp,"%sOrchbase parent : %s\n",os,par->FullName().c_str());
fprintf(fp,"%sCmLoad -------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void CmLoad::Show(FILE * fp)
// Pretty-printer for loader, which is (currently) effectively the XML parser,
// because the object has no other internal state.
// THIS MAY CHANGE WHEN MLV/GMB START LOADING OTHER STUFF
{
fprintf(fp,"\n%s\n",Q::pline.c_str());
fprintf(fp,"\nLoader subsystem status at %s on %s\n\n",GetTime(),GetDate());
                                       // XML parser may not yet exist
if (pXV==0) pXV = new XValid(dynamic_cast<Root *>(par)->pOC->Grammar(),par->fd);
pXV->ShowV(fp);
                                       // Database builder also.....
if (pXB==0) pXB = new DS_XML(par);
pXB->Show(fp);
fprintf(fp,"\n%s\n",Q::sline.c_str());
}

//------------------------------------------------------------------------------

unsigned CmLoad::operator()(Cli * pC)
// Handle "load" command from the monkey.
{
//printf("CmPath_t operator() splitter for ....\n");
//fflush(stdout);
                                       // Just the once
if (pXV==0) pXV = new XValid(dynamic_cast<Root *>(par)->pOC->Grammar(),par->fd);
else pXV->SetOChan(par->fd);
if (pXB==0) pXB = new DS_XML(par);
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause name
  string sCo = pC->Co;                 // Pull out command name
  string sPa = (*i).GetP();            // Pull out simple parameter name
  if (sCl=="app" ) { Cm_App(*i);                  continue;  }
  if (sCl=="bcon") { par->Post(247,sCo,sCl,sPa);  continue;  }
  if (sCl=="cons") { par->Post(247,sCo,sCl,sPa);  continue;  }
  if (sCl=="engi") { Cm_Engine(*i);               continue;  }
  if (sCl=="plac") { par->Post(247,sCo,sCl,sPa);  continue;  }
  if (sCl=="pola") { par->Post(247,sCo,sCl,sPa);  continue;  }
  if (sCl=="pole") { par->Post(247,sCo,sCl,sPa);  continue;  }
  par->Post(25,sCl,"load");           // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================

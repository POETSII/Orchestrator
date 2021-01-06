//------------------------------------------------------------------------------

#include "OrchConfig.h"
#include "FileName.h"

// This is where the software expects to find the configuration file
const string OrchConfig::WhereAmI = string("../Config/Orchestrator.ocfg");

//==============================================================================
/* Seems a lot of typing for bugger all functionality. This heaves an
Orchestrator configuration file in from somewhere and sets up a whole bunch of
default strings.
There's a hierarchy in the config file (the sections) that is a bit pointless,
but is sort of enforced by the sort of semantic check in the constructor.
Any errors, the whole thing is wiped.
Semantic diagnostics are stored in vector<pair<int line, int code> >, and
exposed via vector<pair<int,int> > OrchHead::ExposeErr()
*/
//==============================================================================

OrchConfig::OrchConfig()
// It's all a bit repetitive, because the sections in the config file all (for
// now) have the same structure. But who knows what next thursday may bring?
{
ecnt = 0;                              // So far, so good, then. No errors

JNJ P(WhereAmI);                      // Parse defining file
ecnt = P.ErrCnt();                     // Syntax cockups?
if (ecnt!=0) {                         // If so, bail
  if (P.Td.t==Lex::S_0) IncErr(0,6);
  else IncErr(0,7);
  return;
}
vH sects;
P.GetSect(sects);                      // Get the sections
WALKVECTOR(UIF::Node *,sects,i) {      // Walk the sections
  vH names;
  P.GetNames(*i,names);                // Get the section name(s)
  if (names.empty()) continue;         // Ditch the unnamed section
  if (names.size()!=1) IncErr(*i,1);   // Too many names - bleat
  string s;
  string sn = names[0]->str;           // Section name
  vH varis,valus;
  if (sn=="Orchestrator_header") {
    P.GetVari(*i,varis);               // Get the variables in this section
    WALKVECTOR(UIF::Node *,varis,k) {  // And walk them
      P.LocValu(*k,valus);             // Get value(s) of current variable
                                       // Too many values - bleat
      if (valus.size()>1) IncErr(P.LocRecd(*k),2);
      if (valus.empty()) s.clear();    // None at all (valid value)
      else s = valus[0]->str;          // OK, we have a value of some sort
      if ((*k)->str=="name"   ) Orchestrator_header.name    = s;
      if ((*k)->str=="author" ) Orchestrator_header.author  = s;
      if ((*k)->str=="date"   ) Orchestrator_header.date    = s;
      if ((*k)->str=="version") Orchestrator_header.version = s;
    }
  }
  if (sn=="default_paths") {           // And again....
    P.GetVari(*i,varis);
    WALKVECTOR(UIF::Node *,varis,k) {
      P.LocValu(*k,valus);
      if (valus.size()>1) IncErr(P.FndRecd(*k),3);
      if (valus.empty()) s.clear();
      else s = valus[0]->str;
      if ((*k)->str=="apps"        ) default_paths.apps        = s;
      if ((*k)->str=="engine"      ) default_paths.engine      = s;
      if ((*k)->str=="place"       ) default_paths.place       = s;
      if ((*k)->str=="log"         ) default_paths.log         = s;
      if ((*k)->str=="ulog"        ) default_paths.ulog        = s;
      if ((*k)->str=="trace"       ) default_paths.trace       = s;
      if ((*k)->str=="binaries"    ) default_paths.binaries    = s;
      if ((*k)->str=="stage"       ) default_paths.stage       = s;
      if ((*k)->str=="batch"       ) default_paths.batch       = s;
      if ((*k)->str=="supervisors" ) default_paths.supervisors = s;
      if ((*k)->str=="remote_mship") default_paths.remote_mship = s;
    }
  }
  if (sn=="setup_files") {
    P.GetVari(*i,varis);
    WALKVECTOR(UIF::Node *,varis,k) {
      P.LocValu(*k,valus);
      if (valus.size()>1) IncErr(P.FndRecd(*k),4);
      if (valus.empty()) s.clear();
      else s = valus[0]->str;
      if ((*k)->str=="messages"   ) setup_files.messages    = s;
      if ((*k)->str=="grammar"    ) setup_files.grammar     = s;
      if ((*k)->str=="placement"  ) setup_files.placement   = s;
      if ((*k)->str=="hardware"   ) setup_files.hardware    = s;
    }
  }
  if (sn=="flags") {
    P.GetVari(*i,varis);
    WALKVECTOR(UIF::Node *,varis,k) {
      P.LocValu(*k,valus);
      if (valus.size()>1) IncErr(P.FndRecd(*k),5);
      if (valus.empty()) s.clear();
      else s = valus[0]->str;
      if ((*k)->str=="build"   ) flags.build    = s;
    }
  }
}
if (ecnt!=0) Init();              // Any errors, kill the lot
Force2Linux();                    // Force all the separators to linux-land
}

//------------------------------------------------------------------------------

OrchConfig::~OrchConfig()
{

}

//------------------------------------------------------------------------------

void OrchConfig::Force2Linux()
// Neither FileName nor Windoze care if the path seperator is '/' or '\', but
// linux does, so for the sake of world peace, we simply force all the paths
// to linux-speak here
{
FileName::Force1Linux(default_paths.apps        );
FileName::Force1Linux(default_paths.engine      );
FileName::Force1Linux(default_paths.place       );
FileName::Force1Linux(default_paths.log         );
FileName::Force1Linux(default_paths.ulog        );
FileName::Force1Linux(default_paths.trace       );
FileName::Force1Linux(default_paths.binaries    );
FileName::Force1Linux(default_paths.stage       );
FileName::Force1Linux(default_paths.batch       );
FileName::Force1Linux(default_paths.supervisors );

FileName::Force1Linux(setup_files.messages      );
FileName::Force1Linux(setup_files.grammar       );
FileName::Force1Linux(setup_files.hardware      );
FileName::Force1Linux(setup_files.placement     );
}

//------------------------------------------------------------------------------

void OrchConfig::IncErr(UIF::Node * pn,int code)
// Handle the post-mortem info for the user. We're a bit desperate here
// because not everything is in place, so we're trying to be as helpful as we
// can before bailing
{
ecnt++;                                // We got called - there's a problem
int pos = 0;
if (pn!=0) pos = pn->pos;              // Where from?
                                       // Load the error structure
err_v.push_back(pair<int,int>(pos,code));
}

//------------------------------------------------------------------------------

void OrchConfig::Init()
// Kill everything as a warning to everything else
{
Orchestrator_header.name.clear();
Orchestrator_header.author.clear();
Orchestrator_header.date.clear();
Orchestrator_header.version.clear();

default_paths.apps.clear();
default_paths.engine.clear();
default_paths.place.clear();
default_paths.log.clear();
default_paths.ulog.clear();
default_paths.trace.clear();
default_paths.binaries.clear();
default_paths.stage.clear();
default_paths.batch.clear();
default_paths.supervisors.clear();

setup_files.messages.clear();
setup_files.grammar.clear();
setup_files.hardware.clear();
setup_files.placement.clear();

flags.build.clear();
}

//------------------------------------------------------------------------------

void OrchConfig::Show(FILE * fp)
{
fprintf(fp,"\nOrchConfig::+++++++++++++++++++++++++++++++++++++++++++++++++\n");

fprintf(fp,"ecnt        = %u\n",ecnt);
fprintf(fp,"file        = %s\n",file.c_str());

fprintf(fp,"\nOrchestrator_header:\n");
fprintf(fp,"name        = %s\n",Orchestrator_header.name.c_str());
fprintf(fp,"author      = %s\n",Orchestrator_header.author.c_str());
fprintf(fp,"date        = %s\n",Orchestrator_header.date.c_str());
fprintf(fp,"version     = %s\n",Orchestrator_header.version.c_str());

fprintf(fp,"\ndefault_paths:\n");
fprintf(fp,"apps        = %s\n",default_paths.apps.c_str());
fprintf(fp,"engine      = %s\n",default_paths.engine.c_str());
fprintf(fp,"place       = %s\n",default_paths.place.c_str());
fprintf(fp,"log         = %s\n",default_paths.log.c_str());
fprintf(fp,"ulog        = %s\n",default_paths.ulog.c_str());
fprintf(fp,"trace       = %s\n",default_paths.trace.c_str());
fprintf(fp,"binaries    = %s\n",default_paths.binaries.c_str());
fprintf(fp,"stage       = %s\n",default_paths.stage.c_str());
fprintf(fp,"batch       = %s\n",default_paths.batch.c_str());
fprintf(fp,"supervisors = %s\n",default_paths.supervisors.c_str());

fprintf(fp,"\nsetup_files:\n");
fprintf(fp,"messages    = %s\n",setup_files.messages.c_str());
fprintf(fp,"grammar     = %s\n",setup_files.grammar.c_str());
fprintf(fp,"hardware    = %s\n",setup_files.hardware.c_str());
fprintf(fp,"placement   = %s\n",setup_files.placement.c_str());

fprintf(fp,"\nflags:\n");
fprintf(fp,"build       = %s\n",flags.build.c_str());

fprintf(fp,"\nOrchConfig::Show---------------------------------------------\n");

}

//------------------------------------------------------------------------------

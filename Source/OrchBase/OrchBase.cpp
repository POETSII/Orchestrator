//------------------------------------------------------------------------------

#include "OrchBase.h"
#include "T_gen.h"
#include "Apps_t.h"

//==============================================================================

OrchBase::OrchBase(int argc,char * argv[],string d,string sfile) :
  CommonBase(argc,argv,d,sfile)
{
pE        = 0;                         // Hardware engine? MLV????
pTG       = new T_gen(this);           // PoL task generator
pPlace    = new Placement(this);       // Xlink controller
Name("O_");                            // NameBase root name
CmPath    = new CmPath_t(this);        // Create path command handler
CmGrph    = new CmGrph_t(this);        // Create graph command handler
fd        = stdout;                    // Just in case
}

//------------------------------------------------------------------------------

OrchBase::~OrchBase()
{
if (pE!=0) delete pE;                  // Kill the P-node graph
                                       // None of the below cannot not be here
delete pPlace;                         // Cross link controller
delete pTG;                            // PoL generator
delete CmPath;                         // Path command handler
delete CmGrph;                         // Graph command handler
                                       // Kill entire user application database
WALKMAP(string,Apps_t *,Apps_t::Apps_m,i) delete (*i).second;
}

//------------------------------------------------------------------------------

void OrchBase::Dump(FILE * fp)
{
fprintf(fp,"OrchBase dump+++++++++++++++++++++++++++++++\n");  fflush(fp);
fprintf(fp,"NameBase %s\n",FullName().c_str());
NameBase::Dump(fp);
pPlace->Dump(fp);
pTG->Dump();
CmPath->Dump(fp);
CmGrph->Dump(fp);
WALKMAP(string,Apps_t *,Apps_t::Apps_m,i) (*i).second->Dump(fp);
fprintf(fp,"OrchBase dump-------------------------------\n");  fflush(fp);
}

//==============================================================================

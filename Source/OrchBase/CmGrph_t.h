#ifndef __CmGrph_tH__H
#define __CmGrph_tH__H

//==============================================================================
/* Splitter for graph commands
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
#include "OpsGrph_t.h"


class CmGrph_t : public OpsGrph_t
{
public:
              CmGrph_t(OrchBase *);
virtual ~     CmGrph_t();

unsigned      operator()(Cli *);

};

//==============================================================================
   
#endif

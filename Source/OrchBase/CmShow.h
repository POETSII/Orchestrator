#ifndef __CmShowH__H
#define __CmShowH__H

//==============================================================================
/* Show command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmShow
{
public:
              CmShow(OrchBase *);
virtual ~     CmShow();

void          Dump(FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================
   
#endif
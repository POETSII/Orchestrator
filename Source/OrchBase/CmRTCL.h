#ifndef __CmRTCLH__H
#define __CmRTCLH__H

//==============================================================================
/* Real time clock contoller command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmRTCL
{
public:
              CmRTCL(OrchBase *);
virtual ~     CmRTCL();

void          Dump(unsigned = 0,FILE * = stdout);
void          Show(FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================
   
#endif

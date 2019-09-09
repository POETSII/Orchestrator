#ifndef __CmRTCL_tH__H
#define __CmRTCL_tH__H

//==============================================================================
/* Splitter for RTCL commands
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class Root;

class CmRTCL_t
{
public:
              CmRTCL_t(Root *);
virtual ~     CmRTCL_t();

void          Dump(FILE * = stdout);
unsigned      operator()(Cli *);

Root *        par;

};

//==============================================================================
   
#endif

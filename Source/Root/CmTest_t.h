#ifndef __CmTest_tH__H
#define __CmTest_tH__H

//==============================================================================
/* Splitter for test commands
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class Root;

class CmTest_t
{
public:
              CmTest_t(Root *);
virtual ~     CmTest_t();

void          Dump(FILE * = stdout);
unsigned      operator()(Cli *);

Root *        par;

};

//==============================================================================
   
#endif

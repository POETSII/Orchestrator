#ifndef __CmTestH__H
#define __CmTestH__H

//==============================================================================
/* Test command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmTest
{
public:
              CmTest(OrchBase *);
virtual ~     CmTest();

void          Dump(FILE * = stdout);
void          Show(FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;

};

//==============================================================================

#endif

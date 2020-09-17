#ifndef __CmLoadH__H
#define __CmLoadH__H

//==============================================================================
/* Load command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;
class XValid;
class DS_XML;

//==============================================================================

class CmLoad
{
public :
              CmLoad(OrchBase *);
virtual ~     CmLoad();

void          Cm_App(Cli::Cl_t);
void          Dump(unsigned = 0,FILE * = stdout);
void          Show(FILE *);
unsigned      operator()(Cli *);

private :
OrchBase *    par;
XValid *      pXV;
DS_XML *      pXB;

};

//==============================================================================
   
#endif

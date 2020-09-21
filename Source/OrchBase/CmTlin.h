#ifndef __CmTlinH__H
#define __CmTlinH__H

//==============================================================================
/* Typelink command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class Apps_t;
class OrchBase;

//==============================================================================

class CmTlin
{
public :
              CmTlin(OrchBase *);
virtual ~     CmTlin();

void          Cm_App(Cli::Cl_t);
void          Cm_Grap(Cli::Cl_t);
void          Dump(unsigned = 0,FILE * = stdout);
void          ReportTLinkEnd();
void          ReportTLinkStart();
void          Show(FILE * = stdout);
void          TypeLink(Apps_t *);
unsigned      operator()(Cli *);

private :
OrchBase *    par;
FILE *        fd;
unsigned      ecnt;
long          t0;

};

//==============================================================================

#endif

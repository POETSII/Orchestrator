#ifndef __CmUntlH__H
#define __CmUntlH__H

//==============================================================================
/* Untlink command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class Apps_t;
class OrchBase;

//==============================================================================

class CmUntl
{
public:
              CmUntl(OrchBase *);
virtual ~     CmUntl();

void          Cm_App(Cli::Cl_t);
void          Dump(FILE * = stdout);
void          ReportUTLinkEnd();
void          ReportUTLinkStart();
void          Show(FILE * = stdout);
void          UnTypeLink(Apps_t *);
unsigned      operator()(Cli *);

OrchBase *    par;
FILE *        fd;
unsigned      ecnt;
long          t0;

};

//==============================================================================

#endif

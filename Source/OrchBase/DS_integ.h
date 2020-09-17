#ifndef __DS_integ__H
#define __DS_integ__H

class SupT_t;
class OrchBase;
class Apps_t;
class DevT_t;
class PinT_t;
class GraphI_t;
class GraphT_t;
#include <stdio.h>
#include <string>
#include "OSFixes.hpp"
using namespace std;

//==============================================================================

class DS_integ
{
public:
                    DS_integ(char,Apps_t *,OrchBase *);
virtual ~           DS_integ(void);

void                CheckApps();
void                CheckInstanceGraph();
void                CheckPlace();
void                CheckSystem();
void                CheckTLink();
void                CheckTypeTree();
void                Dump(unsigned = 0,FILE * = stdout);
unsigned            ErrCnt() { return ecnt; }
void                IntegGraph(GraphI_t *);
void                IntegType(GraphT_t *);
void                ReportIntegEnd();
void                ReportIntegStart();
void                SubCheckDevType(DevT_t *);
void                SubCheckInPin(PinT_t *);
void                SubCheckOutPin(PinT_t *);
void                SubCheckSup(SupT_t *);
unsigned            WarCnt() { return wcnt; }

unsigned            itype;
string              apName;
OrchBase *          par;
unsigned            ecnt;
unsigned            wcnt;
long                t0;
FILE *              fd;
Apps_t *            pA;
};

//==============================================================================

#endif



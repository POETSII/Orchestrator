#ifndef __OpsGrph_tH__H
#define __OpsGrph_tH__H

#include <stdio.h>
#include "Cli.h"
class OrchBase;
class GraphI_t;
#include <map.h>
using namespace std;

//==============================================================================

class OpsGrph_t
{
public:
                       OpsGrph_t(OrchBase *);
virtual ~              OpsGrph_t();
unsigned               BuildApp(string);
void                   Dump(FILE * = stdout);
//string                 GetTypeLinkFile(string);
string                 GetDetailFile(string);
void                   GrDela(Cli::Cl_t);
void                   GrFile(Cli::Cl_t);
void                   GrOutp(Cli::Cl_t);
void                   GrRety(Cli::Cl_t);
void                   GrShow(Cli::Cl_t);
string                 sBank(map<string,unsigned> &,unsigned,string);
void                   sBankShow(FILE *,map<string,unsigned> &);
void                   Show(FILE * = stdout);
unsigned               TypeLink(string);
unsigned               Validate(string);
void                   WriteBinaryGraph(GraphI_t *);
unsigned               XLink(string);
unsigned               XLink2(string);
void                   Xunlink(GraphI_t *);

OrchBase *             par;
FILE *                 fd;             // Detail output file

};

//==============================================================================

#endif

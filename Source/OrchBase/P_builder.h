#ifndef __P_builderH__H
#define __P_builderH__H

#include <stdio.h>
#include "OrchBase.h"
#include "OSFixes.hpp"
#include <fstream>
#include <string>
#include <cstdlib>
#include "build_defs.h"
using namespace std;

/*
#ifndef __BORLANDC__
#include "i_graph.h"
#include <QtCore>
#endif
*/

//==============================================================================

class P_builder
{
public:
                    P_builder(OrchBase *);
virtual ~           P_builder();

void                Build(GraphI_t * = 0);
void                Clear(GraphI_t * = 0);
void                Dump(FILE * = stdout);
void                Load(const string&);

OrchBase *            par;

private:

unsigned GenFiles(GraphI_t*);
unsigned CompileBins(GraphI_t*);

unsigned GenSupervisor(GraphI_t*);
unsigned WriteCoreVars(std::string&, unsigned, P_core*, ofstream&);
unsigned WriteThreadVars(std::string&, unsigned, unsigned, P_thread*, ofstream&);

};

//==============================================================================

#endif

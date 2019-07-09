#ifndef __P_builderH__H
#define __P_builderH__H

#include <stdio.h>
#include "OrchBase.h"
#include <fstream>
#include <string>
#include <cstdlib>
#include "build_defs.h"
using namespace std;

#ifndef __BORLANDC__
#include "i_graph.h"
#include <QtCore>
#endif

//==============================================================================

class P_builder
{
public:
                    P_builder(int, char**, OrchBase *);
virtual ~           P_builder();

void                Build(P_task * = 0);
void                Clear(P_task * = 0);
void                Dump(FILE * = stdout);
void                Load(const string&);

OrchBase *            par;

#ifndef __BORLANDC__

QCoreApplication      app;          // Order swapped to suppress initialisation warning
map<string, I_Graph*> defs;

private:

void Preplace(P_task*);
unsigned GenFiles(P_task*);
void CompileBins(P_task*);

unsigned GenSupervisor(P_task*);
unsigned WriteCoreVars(std::string&, unsigned, P_core*, P_thread* , ofstream&);
unsigned WriteThreadVars(std::string&, unsigned, unsigned, P_thread*, ofstream&);

#endif
};

//==============================================================================

#endif

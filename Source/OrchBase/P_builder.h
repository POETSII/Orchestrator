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
map<string, I_Graph*> defs;
QCoreApplication      app;

private:

void Preplace(P_task*);
void GenFiles(P_task*);
void CompileBins(P_task*);
void WriteThreadVars(string&, unsigned int, unsigned int, P_thread*, fstream&);

#endif
};

//==============================================================================

#endif

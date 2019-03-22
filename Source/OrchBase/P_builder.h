#ifndef __P_builderH__H
#define __P_builderH__H

#include <stdio.h>
#include "OrchBase.h"
#include <fstream>
#include <string>
#include <cstdlib>
#include "build_defs.h"
using namespace std;

//==============================================================================

class P_builder
{
public:
            P_builder(int, char**, OrchBase *);
virtual ~   P_builder();

void        Build(P_task * = 0);
void        Clear(P_task * = 0);
void        Dump(FILE * = stdout);
void        Load(const string&);

OrchBase *            par;

multimap<string,P_task*> defs;

private:

void Preplace(P_task*);
void GenFiles(P_task*);
void CompileBins(P_task*);
void WriteThreadVars(string&, unsigned int, unsigned int, P_thread*, fstream&);

};

//==============================================================================

#endif





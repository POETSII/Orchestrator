#ifndef __A_builderH__H
#define __A_builderH__H

#include <stdio.h>
#include "OrchBase.h"
#include "Apps_t.h"
#include <string>
using namespace std;

//==============================================================================

class A_builder
{
public:
                   A_builder(OrchBase *);
virtual ~          A_builder();

void               Build(Apps_t * = 0);
unsigned           DoIt(Apps_t *);
void               Dump(FILE * = stdout);
unsigned           Integ(Apps_t *);
bool               Load(Apps_t *);
void               UnDoIt(Apps_t *);
unsigned           Validate(string);

OrchBase *         par;
string             loadlog;
FILE *             fl;

};

//==============================================================================

#endif





#ifndef __ConstraintsH__H
#define __ConstraintsH__H

#include <stdio.h>
#include "NameBase.h"

//==============================================================================

class Constraints : public NameBase
{
public:
                    Constraints();
virtual ~           Constraints();

map<string, unsigned> Constraintm;

void                Dump(FILE * = stdout);

};

//==============================================================================

#endif





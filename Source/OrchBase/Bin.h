#ifndef __BinH__H
#define __BinH__H

#include "NameBase.h"
#include <stdio.h>

//==============================================================================

class Bin : public NameBase
{
public:
                    Bin(FILE * = 0);
virtual ~           Bin();

void                Dump(FILE * = stdout);
FILE*               Binary;

};

//==============================================================================

#endif





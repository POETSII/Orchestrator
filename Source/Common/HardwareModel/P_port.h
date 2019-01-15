#ifndef __P_portH__H
#define __P_portH__H

#include <stdio.h>
#include "NameBase.h"
#include "pdigraph.hpp"    

//==============================================================================

class P_port : public NameBase, protected DumpChan
{
public:
                    P_port(string);
virtual ~           P_port();

void                Dump(FILE * = stdout);
static void         PrtDat_cb(P_port * const &);
static void         PrtKey_cb(unsigned const &);

};

//==============================================================================

#endif




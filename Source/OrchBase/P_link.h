#ifndef __P_linkH__H
#define __P_linkH__H

#include <stdio.h>
#include "NameBase.h"
#include "pdigraph.hpp"    

//==============================================================================

class P_link : public NameBase, protected DumpChan
{
public:
                    P_link(string);
virtual ~           P_link();

void                Dump(FILE * = stdout);
static void         LnkDat_cb(P_link * const &);
static void         LnkKey_cb(unsigned const &);

};

//==============================================================================

#endif





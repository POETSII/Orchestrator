#ifndef __P_superH__H
#define __P_superH__H

#include <stdio.h>
#include "P_box.h"
#include "P_device.h"
#include <vector>
using namespace std;

//==============================================================================

class P_super : public P_device
{
public:
                    P_super();
                    P_super(string);
virtual ~           P_super();

void                Attach(P_box *);
void                Detach(P_box *);
void                Detach();
void                Dump(FILE * = stdout);

vector<P_box *>     P_boxv;

};

//==============================================================================

#endif





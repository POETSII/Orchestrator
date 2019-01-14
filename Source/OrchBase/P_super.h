#ifndef __P_superH__H
#define __P_superH__H

#include <stdio.h>
#include "PoetsBox.h"
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

void                Attach(PoetsBox *);
void                Detach(PoetsBox *);
void                Detach();
void                Dump(FILE * = stdout);

vector<PoetsBox *>     PoetsBoxv;

};

//==============================================================================

#endif





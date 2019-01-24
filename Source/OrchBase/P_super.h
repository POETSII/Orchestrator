#ifndef __P_superH__H
#define __P_superH__H

#include <algorithm>
#include <stdio.h>
#include "P_board.h"
#include "P_device.h"
#include <vector>
class P_board;
using namespace std;

//==============================================================================

class P_super : public P_device
{
public:
                    P_super();
                    P_super(string);
virtual ~           P_super();

void                Attach(P_board *);
void                Detach(P_board *);
void                Detach();
void                Dump(FILE * = stdout);

vector<P_board*>    P_boardv;  // Points to all boards that contain this
                               // supervisor
};

//==============================================================================

#endif

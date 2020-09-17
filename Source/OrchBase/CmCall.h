#ifndef __CmCallH__H
#define __CmCallH__H

//==============================================================================
/* Call batch command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmCall
{
public:
                 CmCall(OrchBase *);
virtual ~        CmCall();

void             CaEcho(Cli::Cl_t);
void             CaFile(Cli::Cl_t);
void             Dump(unsigned = 0,FILE * = stdout);
Cli              Front();
void             Show(FILE * = stdout);
unsigned         operator()(Cli *);

OrchBase *       par;
bool             echo;
vector <string>  stack;
list<Cli>        Equeue;

};

//==============================================================================
   
#endif

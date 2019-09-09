#ifndef __CmCall_tH__H
#define __CmCall_tH__H

//==============================================================================
/* Splitter for call batch commands
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class Root;

class CmCall_t
{
public:
                 CmCall_t(Root *);
virtual ~        CmCall_t();

void             Dump(FILE * = stdout);

void             CaEcho(Cli::Cl_t);
void             CaFile(Cli::Cl_t);
void             CaShow(Cli::Cl_t);
Cli              Front();
unsigned         operator()(Cli *);

Root *           par;
bool             echo;
vector <string>  stack;
list<Cli>        Equeue;

};

//==============================================================================
   
#endif

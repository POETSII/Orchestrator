#ifndef __Config_tH__H
#define __Config_tH__H

#include <stdio.h>
#include "NameBase.h"
class P_graph;

//==============================================================================

// VERY MUCH A TEMPORARY EMBODIMENT

class Config_t : public NameBase
{
public:
                  Config_t(P_graph *,string);
virtual ~         Config_t();

void              Dump(FILE * = stdout);
unsigned          GetBMem();         // Return available memory on each board
unsigned          GetBoards();       // Return board count in the box
unsigned          GetCores();        // Return core count in a board
unsigned          GetThreads();         // Return thread count in a core
void              SetBMem(unsigned);    // Assign available memory on each board
void              SetBoards(unsigned);  // Assign board count in the box
void              SetCores(unsigned);   // Assign core count in a board
void              SetThreads(unsigned); // Assign thread count in a core
unsigned          ThreadsPerBox();   // Guess

unsigned          bMem;
unsigned          boards; 
unsigned          cores;
unsigned          threads;
P_graph *         par;

};

//==============================================================================

#endif





#ifndef __T_GenH__H
#define __T_GenH__H

#include <stdio.h>
//#include "common.h"
#include "rand.h"
#include "filename.h"
#include "dfprintf.h"
#include "Apps_t.h"

//==============================================================================
/* Test- and proof-of-life generator. Strongly derived from SpiNNaker....
*/

class T_gen {
public:
              T_gen(OrchBase *);
virtual ~     T_gen(void);
//void          Build(P_task *);
//void          BuildClique(P_task *);
//void          BuildParameters(P_task *);
//void          BuildRandom(P_task *);
//void          BuildRing(P_task *);
//void          BuildTree(P_task *);
void          Dump(FILE * = stdout);
//string        FindQualifier  (int,char * [],string);

OrchBase *    par;

};

//==============================================================================

#endif




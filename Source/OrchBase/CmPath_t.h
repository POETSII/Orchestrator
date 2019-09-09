#ifndef __CmPath_tH__H
#define __CmPath_tH__H

//==============================================================================
/* Splitter for path commands
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmPath_t
{
public:
              CmPath_t(OrchBase *);
virtual ~     CmPath_t();

void          Dump(FILE * = stdout);
FILE *        Fclose();
FILE *        Fopen();
void          GenerateMode();
FILE *        GetLolfp();
FILE *        GetOfgfp();
void          Show(FILE * = stdout);
unsigned      operator()(Cli *);

OrchBase *    par;
string        pathBina;
string        pathGrap;
string        pathOutp;
string        pathPlat;
enum Outype {Ou_Stdo=0,Ou_Lolf,Ou_Ofgf} OuMode;

};

//==============================================================================
   
#endif

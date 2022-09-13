#ifndef __CmPathH__H
#define __CmPathH__H

//==============================================================================
/* Path command handler
*/
//==============================================================================

#include <stdio.h>
#include "Cli.h"
class OrchBase;

class CmPath
{
public:
              CmPath(OrchBase *);
virtual ~     CmPath();

void          Clear();
bool          Cm_Path(bool,string &,string,string,bool);
void          Dump(unsigned = 0,FILE * = stdout);
FILE *        Fclose();
FILE *        Fopen();
void          GenerateMode();
FILE *        GetLolfp();
FILE *        GetOfgfp();
void          Reset();
void          Show(FILE * = stdout);
void          UpdateMotherships();
unsigned      operator()(Cli *);

OrchBase *    par;
string        pathApps;
string        pathBatc;
string        pathBina;
string        pathEngi;
string        pathLog;
string        pathMonS;
string        pathMout;
string        pathMshp;
string        pathPlac;
string        pathStag;
string        pathSupe;
string        pathTrac;
string        pathUlog;
enum Outype {Ou_Stdo=0,Ou_Lolf,Ou_Ofgf} OuMode;
string        lastfile;

};

//==============================================================================

#endif

#ifndef __XValidH__H
#define __XValidH__H

#include <string>
#include "OSFixes.hpp"
using namespace std;
#include "XMLTreeDump.h"

//==============================================================================

class XValid
{
public:
typedef       xmlTreeDump::node xnode;
              XValid(string,FILE * =stdout);
virtual ~     XValid(void);

void          AddStencil(xnode *);
xnode *       ClRoot() { return (Validee==0) ? 0 : Validee->Root(); }
void          DeleteTag(xnode *);
void          Dump(unsigned = 0,FILE * = stdout);
void          ErrCnt(unsigned &, unsigned &);
void          Init(string,FILE *);
void          InitValidation();
unsigned      ReportClientStart();
unsigned      ReportClientEnd(xnode * =0);
unsigned      ReportGrammarEnd();
void          ReportGrammarStart();
void          SetOChan(FILE *);
void          ShowP(FILE * = stdout);
void          ShowV(FILE * = stdout);
void          Validate(string);
void          Validate(string,FILE *);
void          Validate2(xnode * =0);
void          ValidorCheck(xnode *);

private:
unsigned      c_ecnt;                  // Client validation error count
unsigned      g_ecnt;                  // Grammar error count
xmlTreeDump   Validor;                 // Grammar definition
xmlTreeDump * Validee;                 // Current file under analysis
FILE *        fo;                      // Stream for output
FILE *        gfp;                     // Grammar file input stream
FILE *        cfp;                     // Client file input stream
long          t0;                      // Wallclock time

const static string sline;             // --- Pretty-print separators
const static string dline;             // ...
const static string aline;             // ***
const static string eline;             // ===
const static string pline;             // +++

};

//==============================================================================

#endif

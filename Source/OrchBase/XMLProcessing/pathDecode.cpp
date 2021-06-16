//---------------------------------------------------------------------------

#include "pathDecode.h"
#include "lex.h"
#include "macros.h"
#include "flat.h"

//==============================================================================

vector<string> pathDecode::operator()(string path)
{
Lex Lx;                                // Get us a lexer...
Lex::tokdat Td;                        // ...let it generate tokens
vector<string> strv;                   // The output
strv.push_back(string());              // Pre-load the 0th element
Lx.SetFile(path);                      // Point the lexer at the input
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3} toktyp;
struct duple {int ns,ac;} next;
const int X = -1;                      // We've stepped off the path
const int R = -2;                      // Legitimate exit
duple table[7][t3+1] =
// Incident symbol               // Next
//    0      1      2      3     // state
{{{1, 1},{2, 0},{X, 2},{X, 2}},  // 0
 {{X, 2},{2, 0},{X, 2},{X, 2}},  // 1
 {{3, 3},{X, 2},{X, 2},{X, 2}},  // 2
 {{X, 2},{X, 2},{4, 0},{X, 2}},  // 3
 {{5, 4},{6, 0},{X, 2},{X, 2}},  // 2
 {{X, 2},{6, 0},{X, 2},{X, 2}},  // 3
 {{R, 5},{X, 2},{X, 2},{X, 2}}}; // 4

string Dto,Pto,Dfr,Pfr;                // Path components to be defined
for(int state=0;;) {
  Lx.GetTok(Td);                       // Get the next token...
  strv[0] = int2str(Td.c);             // Pre-emptively store column position
//  if (Td.t==Lex::Sy_EOR) continue;     // Chuck away any newlines
  if (Lx.IsError(Td)) return strv;
  switch (Td.t) {                      // Map to array index
    case Lex::Sy_col  : toktyp = t1;  break;
    case Lex::Sy_sub  : toktyp = t2;  break;
    default           : toktyp = t3;  break;
  }
  if (Lex::IsStr(Td.t))   toktyp = t0; // Reduction functions
                                       // Override the reduction functions ?
  next = table[state][toktyp];         // Make the transition
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  :               break;     // No action
    case 1  : Dto = Td.s;  break;      // Save "to" device
    case 2  : return strv;             // Fallen off syntax graph ?
    case 3  : Pto = Td.s;  break;      // Save "to" pin
    case 4  : Dfr = Td.s;  break;      // Save "from" device
    case 5  : Pfr = Td.s;  break;      // Save "from" pin
    default : return strv;             // Never here?
  }
  switch (state=next.ns) {
    case X  : return strv;             // Exit - should never be here - act code
    case R  : strv[0].clear();         // Clean return - hose column indicator
              strv.push_back(Dto);     // Have to load like this because some of
              strv.push_back(Pto);     // the strings may be empty
              strv.push_back(Dfr);
              strv.push_back(Pfr);
              return strv;
    default : break;
  }
}
}

//==============================================================================

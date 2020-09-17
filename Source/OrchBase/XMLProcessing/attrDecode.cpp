//---------------------------------------------------------------------------

#include "attrDecode.h"
#include "lex.h"
#include "macros.h"
#include "flat.h"

//==============================================================================

vector<attrDecode::a6_t> attrDecode::operator()(string s)
{
Lex Lx;
Lx.SetFile(s);                         // Point the lexer at incoming string
Lx.SetNFlag(false);                    // Tune lexer to *not* recognise numbers
Lx.SetDFlag(false);                    // Treat "." as separator
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4,t5,t6,t7,t8} toktyp;
struct duple {int ns,ac;} next;
const int X = -1;
duple table[12][t8+1] =
// Incident symbol                                                  // Next
//    0      1      2      3      4      5      6      7      8     // state
{{{X ,X},{X ,X},{X ,X},{X ,X},{X ,X},{X ,X},{1 ,0},{X ,X},{X ,X}},  // 0
 {{X ,X},{2 ,1},{3 ,0},{X ,X},{X ,X},{X ,X},{X ,X},{5 ,0},{X ,X}},  // 1
 {{X ,X},{X ,X},{3 ,0},{X ,X},{X ,X},{X ,X},{X ,X},{5 ,2},{X ,X}},  // 2
 {{X ,X},{4 ,3},{X ,X},{X ,X},{X ,X},{X ,X},{X ,X},{5 ,0},{X ,X}},  // 3
 {{X ,X},{X ,X},{X ,X},{X ,X},{X ,X},{X ,X},{X ,X},{5 ,0},{X ,X}},  // 4
 {{6 ,0},{8 ,4},{X ,X},{X ,X},{0 ,6},{X ,6},{X ,X},{X ,X},{X ,X}},  // 5
 {{X ,X},{7 ,4},{X ,X},{X ,X},{X ,X},{X ,X},{X ,X},{X ,X},{X ,X}},  // 6
 {{X ,X},{X ,X},{X ,X},{8 ,0},{6 ,0},{X ,X},{X ,X},{X ,X},{X ,X}},  // 7
 {{9 ,0},{X ,X},{X ,X},{X ,X},{0 ,0},{X ,0},{X ,X},{X ,X},{X ,X}},  // 8
 {{X ,X},{10,5},{X ,X},{11,0},{X ,X},{X ,X},{X ,X},{X ,X},{X ,X}},  // 9
 {{X ,X},{X ,X},{X ,X},{11,0},{X ,X},{X ,X},{X ,X},{X ,X},{X ,X}},  // 10
 {{X ,X},{X ,X},{X ,X},{X ,X},{0 ,0},{X ,0},{X ,X},{X ,X},{X ,X}}}; // 11

vector<attrDecode::a6_t> ans;          // Our final answer
ans.push_back(a6_t('S',s,0,0));        // Store input string in element [0]
bool problem = false;                  // We bail on the first error
unsigned n = 0;                        // Internal state - persistent in loop
static const unsigned Ldef = 0;        // Default upper and lower bounds
static const unsigned Udef = LOTS-1;
static const unsigned Xdef = LOTS;     // Error value
unsigned cnt = 0;
unsigned L=0,U=LOTS;                   // Lower and upper count bounds - init
                                       // here just to shut gcc up.
for(int state=0;;) {                   // Kick it, Winston....
  Lex::tokdat Td;
  Lx.GetTok(Td);                       // Get the next token
  switch (state) {                     // Pre-transition (entry) actions
    case 0 : L = Ldef;                 // Initialise upper/lower bounds
             U = Udef;
             cnt = 0;                  // Counter for current set of names
             break;
  }
  if (Lx.IsError(Td)) problem = true;  // No exceptional cases (EOF in table)
  switch (Td.t) {                      // Map to array index
    case Lex::Sy_lrnb : toktyp = t0;  break;
    case Lex::Sy_ddot : toktyp = t2;  break;
    case Lex::Sy_rrnb : toktyp = t3;  break;
    case Lex::Sy_cmma : toktyp = t4;  break;
    case Lex::Sy_EOF  : toktyp = t5;  break;
    case Lex::Sy_lsqb : toktyp = t6;  break;
    case Lex::Sy_rsqb : toktyp = t7;  break;
    default           : toktyp = t8;  break;
  }
  if (Lex::IsStr(Td.t))   toktyp = t1; // Reduction functions
  next = table[state][toktyp];         // Make the transition
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  :                                    break; // No action
    case X  : problem = true;                    break; // Bad syntax transition
    case 1  : L = n = str2uint(Td.s,Xdef);              // Store lower bound
              if (L!=Xdef)                       break; // All good?
              else { problem=true; L=n=Udef; }   break; // No - scream
    case 2  : U = n;                             break; // Upper = lower bound
    case 3  : U = str2uint(Td.s,Xdef);                  // Store upper bound
              if (U!=Xdef)                       break; // All good?
              else { problem=true; U=Udef; }     break; // No - scream
    case 4  : ans.push_back(a6_t('A',Td.s,L,U));        // Valid attribute name
              cnt++;                             break; // Count them
    case 5  : for (;cnt>0;cnt--)                        // Give name set syntax
                {ans[ans.size()-cnt].Ss = Td.s;} break; // Attr value syntax
    case 6  : ans.push_back(a6_t('E',"",L,U));   break; // Valid element name
    default : problem = true;                    break; // Should never happen
  }
  if (problem==true) {                 // Problem - from anywhere ?
    ans[0].L = 1;                      // Flag it
    ans[0].U = Td.c;                   // Store location in input string
    return ans;
  }
  switch (state=next.ns) {
    case X : return ans;               // Bail - for whatever reason
  }
} // for(ever

}

//==============================================================================

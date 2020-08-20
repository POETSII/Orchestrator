//------------------------------------------------------------------------------

#include "filename.h"
#include "macros.h"

const string FileName::Xc(1,':');
// ADR bodgey workaround for discrepancies in file path character. Run-time OS detection would be better.
#ifdef __BORLANDC__
const string FileName::Xs(1,'\\');
#endif
#ifdef _MSC_VER
const string FileName::Xs(1,'\\');
#endif
#ifdef __GNUC__
const string FileName::Xs(1,'/');
#endif
const string FileName::Xd(1,'.');
const string FileName::X0;

//==============================================================================

FileName::FileName(string s)
// Constructor from input string
{
Sip = s;                               // Stuff the input data away
Parse();                               // Build the output strings
}

//------------------------------------------------------------------------------

FileName::~FileName()
{
}

//------------------------------------------------------------------------------

void FileName::Dump()
{
printf("\nFileName dump.....\n");
printf("Input data                   Sip = %s\n",Sip.c_str());
printf("Drive substring               S0 = %s\n",S0.c_str());
printf("Path substring vector         S1 : \n");
WALKVECTOR(string,S1,i)
  printf("                                 ||%s||\n",(*i).c_str());
printf("Extn string                   S2 = %s\n",S2.c_str());

printf("Last element of path removed S1e = %s\n",S1e.c_str());

printf("Output string:   Complete     SC = %s\n",SC.c_str());
printf("Output string:   Drive        SD = %s\n",SD.c_str());
printf("Output string:   Path         SP = %s\n",SP.c_str());
printf("Output string:   Base         SB = %s\n",SB.c_str());
printf("Output string:   Extension    SE = %s\n",SE.c_str());
printf("Old size of S1:  oS1siz          = %d\n",oS1siz);
printf("Problem                          = %s\n",problem?"true":"false");

printf("static const string           Xc = %s\n",Xc.c_str());
printf("static const string           Xs = %s\n",Xs.c_str());
printf("static const string           Xd = %s\n",Xd.c_str());
}

//------------------------------------------------------------------------------

bool FileName::Err()
// Return problem status flag
{
return problem;
}

//------------------------------------------------------------------------------

string FileName::FNBase()
// Extract base name
{
return problem ? X0 : SB;
}

//------------------------------------------------------------------------------

void FileName::FNBase(string s)
// Set base name
{
SB = s;
Derive2();                             // And the things that depend upon it
Sip = SC;                              // Recreate the input string
Parse();                               // Update everything else
}

//------------------------------------------------------------------------------

string FileName::FNComplete()
// Extract complete filename
{
return problem ? X0 : SC;
}

//------------------------------------------------------------------------------

void FileName::FNComplete(string s)
// Set complete name
{
Sip = s;                               // New input string
Parse();                               // Update everything
}

//------------------------------------------------------------------------------

string FileName::FNDrive()
// Extract drive letter
{
return problem ? X0 : SD;
}

//------------------------------------------------------------------------------

void FileName::FNDrive(string s)
// Set drive
{
SD = s;
if (!s.empty()) SD = string(1,s[0]);   // Make sure it's one character
Derive2();                             // And the things that depend upon it
Sip = SC;                              // Recreate the input string
Parse();                               // Update everything else
}

//------------------------------------------------------------------------------

string FileName::FNExtn()
// Extract extension
{
return problem ? X0 : SE;
}

//------------------------------------------------------------------------------

void FileName::FNExtn(string s)
// Set extension
{
SE = s;
Derive2();                             // And the things that depend upon it
Sip = SC;                              // Recreate the input string
Parse();                               // Update everything else
}

//------------------------------------------------------------------------------

string FileName::FNPath(vector<string> & s)
// Extract full path. String vector goes out as reference argument, concatenated
// path (including drive) as a single string return value
{
s.clear();
if (problem) return X0;
s = S1;                                // Base already torn off
return SP;
}

//------------------------------------------------------------------------------

void FileName::FNPath(string s,int pos)
// Modify path string. If 'pos' is off either end, 's' is attached to that end.
// If it represents a valid position in the string vector, the string is
// replaced.
// THERE IS NO MECHANISM FOR INTRODUCING EXTRA LEVELS OF HIERARCHY EXCEPT AT THE
// TOP AND BOTTOM
// THERE IS NO MECHANISM FOR REMOVING LEVELS OF HIERARCHY
{
int siz = int(S1.size());

if (siz==0)  { S1.push_back(s);         goto done;} // Vector initially empty
if (pos<0)   { S1.insert(S1.begin(),s); goto done;} // Bolt onto LHS
if (pos>=siz){ S1.push_back(s);         goto done;} // Bolt onto RHS
S1[pos]=s;                             // Replace existing element
oS1siz--;                              // Decrement overriden by next line

done:
oS1siz++;                              // The path vector size goes up
Derive1();                             // Recreate stuff that depends upon it
Derive2();
Sip = SC;                              // Recreate the input string
Parse();                               // Update everything else
}

//------------------------------------------------------------------------------

string FileName::FNName()
// Simples...
{
return FNBase() + Xd + FNExtn();
}

//------------------------------------------------------------------------------

void FileName::Parse()
{
Lx.SetFile(Sip);                       // Point the lexer at it
Lx.SetMFlag(true);                     // Treat "-" as an alphanumeric
Lx.SetNFlag(false);                    // Tune lexer to *not* recognise numbers
Lx.SetDFlag(false);                    // '.' is a separator
Lx.SetCChar(EOF);                      // No continuation character
S0.clear();                            // Re-initialise the extracted strings
S1.clear();                            // Empty the path vector
S2.clear();
SE = SB = SP = SD = SC = X0;
problem = false;                       // So far, so good
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4,t5} toktyp;
struct duple {int ns,ac;} next;
duple table[6][t5+1] =
// Incident symbol                             // Next
//    0      1      2      3      4      5     // state
{{{1, 1},{2, 0},{3, 0},{X, X},{R, 0},{0, X}},  // 0
 {{X, X},{2, 0},{X, X},{5, 0},{R, 0},{0, X}},  // 1
 {{X, X},{X, X},{3, 0},{X, X},{R, 0},{0, X}},  // 2
 {{4, 2},{X, X},{X, X},{X, X},{R, 0},{0, X}},  // 3
 {{X, X},{X, X},{3, 0},{5, 0},{R, 0},{0, X}},  // 4
 {{R, 3},{X, X},{X, X},{X, X},{R, 0},{0, X}}}; // 5

for(int state=0;;) {
  Lex::tokdat Td;                      // Current lexer token
  Lx.GetTok(Td);                       // Get the next token...
//  switch (state) {                     // Pre-transition (entry) actions    
//  }
                                       // No exceptional cases (EOF in table)
  if (Lx.IsError(Td)) problem = true;
  switch (Td.t) {                      // Map to array index
    case Lex::Sy_col  : toktyp = t1;  break;
    case Lex::Sy_div  :
    case Lex::Sy_back : toktyp = t2;  break;
    case Lex::Sy_dot  : toktyp = t3;  break;
    case Lex::Sy_EOF  : toktyp = t4;  break;
    case Lex::Sy_mult : toktyp = t0;  break;  // "*" accepted as a valid string
    default           : toktyp = t5;  break;
  }
  if (Lex::IsStr(Td.t)) toktyp = t0;   // Reduction functions
  next = table[state][toktyp];         // Make the transition
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  :                         break;
    case X  : problem = true;         break;
    case 1  : S0 = Td.s;              break;
    case 2  : S1.push_back(Td.s);     break;
    case 3  : S2 = Td.s;              break;
    default :                         break;
  }
  switch (state=next.ns) {
    case X :  goto done;
    case R :  goto done;
  }
  if (problem==true) break;            // May be set elsewhere
}
if (problem==true) return;

done:

oS1siz = (unsigned int)S1.size();
if (!S1.empty()) {                     // If the path vector isn't empty...
  S1e = S1.back();                     // ...pull the basename off the end
  S1.pop_back();
}

Derive1();                             // Derive the secondary strings 'n' stuff
Derive2();
return;
}

//------------------------------------------------------------------------------

void FileName::Derive1()
// Derive extension, base and drive
{
SE = S2;                               // Extn
if (oS1siz!=0) SB = S1e;               // Base
else {
  if (!S0.empty()) SB = S0;
  else SB = S2;                        // Even if S2 = empty
}
if ((!S0.empty())&&(oS1siz!=0)) SD = string(1,S0[0]);   // Drive, else SD = ""
}

//------------------------------------------------------------------------------

void FileName::Derive2()
// Derive path and complete
{
SP.clear();                            // Build the path as a single string
WALKVECTOR(string,S1,i) SP += Xs + *i;
if (!SD.empty()) SP = SD + Xc + SP;

SC = SB;                               // Complete reconstruction
if (!SP.empty()) SC = SP + Xs + SC;
if (!SE.empty()) SC += Xd + SE;
}

//------------------------------------------------------------------------------


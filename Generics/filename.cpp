//------------------------------------------------------------------------------

#include "filename.h"
#include "macros.h"

//==============================================================================

FileName::FileName(string s)
// Constructor from input string
{
sIP = s;                               // Stuff the input data away
Parse();                               // Build the output strings
}

//------------------------------------------------------------------------------

FileName::~FileName()
{
}

//------------------------------------------------------------------------------

void FileName::Dump(FILE * fp)
{
fprintf(fp,"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"\nFileName dump.....\n");
fprintf(fp,"Input data                   sIP = %s\n",sIP.c_str());
fprintf(fp,"Drive substring               sD = %s\n",sD.c_str());
fprintf(fp,"Path substring vector         sP : \n");
WALKVECTOR(string,sP,i)
  fprintf(fp,"                                 ||%s||\n",(*i).c_str());
fprintf(fp,"Base string                   sB = %s\n",sB.c_str());
fprintf(fp,"Extn string                   sE = %s\n",sE.c_str());
fprintf(fp,"Sepn string                   sS = %s\n",sS.c_str());
fprintf(fp,"Entire substring vector     sbuf : \n");
WALKVECTOR(string,sbuf,i)
  fprintf(fp,"                                 ||%s||\n",(*i).c_str());
fprintf(fp,"Drive string address          aD = %d\n",aD);
fprintf(fp,"Path string addresses       aP[] = (%d,%d)\n",aP[0],aP[1]);
fprintf(fp,"Base string address           aB = %d\n",aB);
fprintf(fp,"Extn string address           aE = %d\n",aE);
fprintf(fp,"Output string:  FNComplete()     = %s\n",FNComplete().c_str());
fprintf(fp,"Problem                          = %s\n",problem?"true":"false");
fprintf(fp,"Dirty                            = %s\n",dirty?"true":"false");
fprintf(fp,"-------------------------------------------------------------\n\n");
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
return problem ? string() : sB;
}

//------------------------------------------------------------------------------

void FileName::FNBase(string s)
// Set base name
{
sB = s;
dirty = true;
}

//------------------------------------------------------------------------------

string FileName::FNComplete()
// Extract complete filename. If there's a problem, we return nothing.
// However, if the monkey has buggered about with the components since the
// original string was parsed, they may well have broken it.
// So a bit of paranoia: we build the output string, then we parse it all over
// again, and if it's OK, we hand it out.
{
if (problem) return string();
string ans;
if (!sD.empty())        ans = sD+":";  // Drive?
if (RAmode=='A')        ans += sS;     // Mode = abs, need an initial separator
WALKVECTOR(string,sP,i) ans += ((*i)+sS);    // Bolt the path togther
ans += sB;                                   // Base
if (!sE.empty())        ans += ("."+sE);     // Extension if any
if (dirty) {                           // If the monkey might have broken it
  FNComplete(ans);                     // ... reparse it
  ans = FNComplete();                  // Rebuild it
}
return ans;
}

//------------------------------------------------------------------------------

void FileName::FNComplete(string s)
// Set complete name - effectively reset the internal state and start again.
{
sIP = s;                               // New input string
Parse();                               // Update everything
}

//------------------------------------------------------------------------------

string FileName::FNDrive()
// Extract drive letter
{
return problem ? string() : sD;
}

//------------------------------------------------------------------------------

void FileName::FNDrive(string s)
// Set drive
{
sD = s;
dirty = true;
}

//------------------------------------------------------------------------------

string FileName::FNExtn()
// Extract extension
{
return problem ? string() : sE;
}

//------------------------------------------------------------------------------

void FileName::FNExtn(string s)
// Set extension
{
sE = s;
dirty = true;
}

//------------------------------------------------------------------------------

char FileName::FNMode()
// Extract relative/absolute mode
{
return RAmode;
}

//------------------------------------------------------------------------------

void FileName::FNMode(char nm)
// Set relative/absolute mode
{
if ((nm!='A')&&(nm!='R')) return;
RAmode = nm;
dirty = true;
}

//------------------------------------------------------------------------------

string FileName::FNName()
// Simples...
{
if (problem) return string();
return FNBase() + string(".") + FNExtn();
}

//------------------------------------------------------------------------------

vector<string> FileName::FNPath()
// Extract path string vector
{
if (problem) return vector<string>();
return sP;
}

//------------------------------------------------------------------------------

void FileName::FNPath(vector<string> s)
// Set path string vector
{
sP = s;
dirty = true;
}

//------------------------------------------------------------------------------

string FileName::FnSepn()
// Extract separator
{
return sS;
}

//------------------------------------------------------------------------------

void FileName::FnSepn(string s)
// Set separator
{
sS = s;
dirty = true;
}

//==============================================================================

// Private methods:

//------------------------------------------------------------------------------

void FileName::EOParse()
// The Parser proper shoved ALL the input strings into a single string vector
// (sbuf), and - whenever it could - worked out the addresses in the vector
// of the various components (which one was drive, base and so on).
// Here we look at the numeric addresses, and copy the various strings out
// to their target variables.
// The addresses are integer offsets into the vector<string>; -1 means not
// defined.
{
if (aB== -1) {                         // No base: it's illegal
  problem = true;                      // No point in carrying on
  return;
}
if (aD!= -1) {                         // Drive string there?
  sD = sbuf[aD];                       // Defined the string itself
  if (aB!=1) {                         // The base isn't the second (1)
    aP[0]=1;                           // Path must be 1..aB-1
    aP[1]=aB-1;
  }
} else {                               // No drive string defined
  if (aB!=0) {                         // The base isn't the first (0) thing
    aP[0]=0;                           // Path must be 0..aB-1
    aP[1]=aB-1;
  }
}
                                       // Copy the path subset out of the string
                                       // buffer into the path vector
if (aP[0]!= -1) for (int i=aP[0];i<=aP[1];i++) sP.push_back(sbuf[i]);
sB = sbuf[aB];                         // There must be a base string
if (aE!= -1) sE = sbuf[aE];            // There may be an extension string
}

//------------------------------------------------------------------------------

void FileName::Parse()
{
Lx.SetFile(sIP);                       // Point the lexer at it
Lx.SetMFlag(true);                     // Treat "-" as an alphanumeric
Lx.SetNFlag(false);                    // Tune lexer to *not* recognise numbers
Lx.SetDFlag(false);                    // '.' is a separator
Lx.SetCChar(EOF);                      // No continuation character
                                       // Re-initialise the extracted strings:
sD.clear();                            // Drive string
sP.clear();                            // Path string vector
sB.clear();                            // Base string
sE.clear();                            // Extension string
sS=string("?");                        // Separator
aD = -1;                               // Address of drive string in buffer
aB = -1;                               // Address of base string in buffer
aE = -1;                               // Address of extn string in buffer
aP[0] = aP[1] = -1;                    // Extent of path string subset in buffer
sbuf.clear();                          // Done nothing yet
RAmode = 'R';                          // Default path is relative
problem = false;                       // All is currently tickety-spong
dirty = false;                         // The monkey can't have broken it yet
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4,t5,t6} toktyp;
struct duple {int ns,ac;} next;
duple table[7][t6+1] =
// Incident symbol                                    // Next
//    0      1      2      3      4      5      6     // state
{{{1, 1},{X, X},{3,10},{X, X},{X, X},{6, 1},{X, X}},  // 0
 {{X, X},{2, 3},{3, 7},{5, 4},{R, 8},{X, X},{X, X}},  // 1
 {{4, 1},{X, X},{3,10},{X, X},{X, X},{4, 1},{X, X}},  // 2
 {{4, 1},{X, X},{X, X},{X, X},{X, X},{4, 1},{X, X}},  // 3
 {{X, X},{X, X},{3, 7},{5, 5},{R, 9},{X, X},{X, X}},  // 4
 {{R, 6},{X, X},{X, X},{X, X},{X, X},{X, X},{X, X}},  // 5
 {{X, X},{X, X},{3, 7},{X, X},{X, X},{X, X},{X, X}}}; // 6

for(int state=0;;) {
  Lex::tokdat Td;                      // Current lexer token
  Lx.GetTok(Td);                       // Get the next token...
//  switch (state) {                     // Pre-transition (entry) actions
//  }
                                       // No exceptional cases (EOF in table)
  if (Lx.IsError(Td)) problem = true;
  switch (Td.t) {                      // Map to array index
    case Lex::Sy_mult : toktyp = t0;  break;  // "*" accepted as a valid string
    case Lex::Sy_col  : toktyp = t1;  break;
    case Lex::Sy_div  :
    case Lex::Sy_back : toktyp = t2;  break;
    case Lex::Sy_dot  : toktyp = t3;  break;
    case Lex::Sy_EOF  : toktyp = t4;  break;
    case Lex::Sy_ddot : toktyp = t5;  break;
    default           : toktyp = t6;  break;
  }
  if (Lex::IsStr(Td.t)) toktyp = t0;   // Reduction functions
  next = table[state][toktyp];         // Make the transition
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  :                         break;   // Null action
    case X  : problem = true;         break;   // Fallen off syntax graph
    case 1  : sbuf.push_back(Td.s);   break;   // Save a string
    case 2  : EOParse();              break;   // Done; assemble output
    case 3  : aD = 0;                 break;   // Drive string index
    case 4  : aB = 0;                 break;   // Base string index
    case 5  : aB = sbuf.size()-1;     break;   // Base string index
    case 6  : sbuf.push_back(Td.s);            // Save a string
              aE = sbuf.size()-1;              // It's the extension
              EOParse();              break;   // Done; assemble output
    case 7  : sS = Td.s;              break;   // Found a separator ("\" or "/")
    case 8  : aB = 0;                          // Base string index
              EOParse();              break;   // Done; assemble output
    case 9  : aB = sbuf.size()-1;              // Base string index
              EOParse();              break;   // Done; assemble output
    case 10 : sS = Td.s;                       // Found a separator ("\" or "/")
              RAmode = 'A';           break;   // Put into "absolute" mode
    default :                         break;
  }
  switch (state=next.ns) {
    case X :  return;                   // Cockup exit
    case R :  return;                   // Legitimate exit
  }
  if (problem==true) return;            // May be set elsewhere
}
}

//------------------------------------------------------------------------------



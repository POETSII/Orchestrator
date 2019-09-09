//---------------------------------------------------------------------------

#include "Cli.h"
#include "macros.h"
#include "flat.h"
#include <algorithm>
using namespace std;

//==============================================================================

Cli::Cli()
{
}

//------------------------------------------------------------------------------

Cli::Cli(string instr)
// Constructor from command line
{
Orig = instr;                          // Save the input data
static const int   X = -1;             // Cosmic error table entry
static const int   R = -2;             // Cosmic return table entry
Lex Lx;
Lex::tokdat        Td;                 // Current lexer token
Lx.SetFile(instr);                     // Point the lexer at input string
Lx.SetNFlag(true);                     // Tune lexer to recognise numbers

                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4,t5,t6,t7} toktyp;
struct duple {int ns,ac;} next;
static duple table[7][t7+1] =
// Incident symbol
//   An      /      =      ,    EOF     Op   else   Cmnt
//   t0     t1     t2     t3     t4     t5     t6     t7
{{{1, 1},{3, 2},{X, X},{X, X},{R, 0},{X, X},{X, X},{R, 0}},  // 0   Next
 {{X, X},{3, 2},{X, X},{X, X},{R, 0},{X, X},{X, X},{R, 0}},  // 1   state
 {{6, 7},{3, 2},{X, X},{5, 0},{R, 0},{X, X},{X, X},{R, 0}},  // 2
 {{4, 4},{X, X},{X, X},{X, X},{R, 0},{X, X},{X, X},{R, 0}},  // 3
 {{X, X},{3, 2},{5, 0},{X, X},{R, 0},{X, X},{X, X},{R, 0}},  // 4
 {{6, 5},{3, 2},{X, X},{X, X},{R, 0},{2, 6},{X, X},{R, 0}},  // 5
 {{X, X},{3, 2},{X, X},{5, 0},{R, 0},{X, X},{X, X},{R, 0}}}; // 6

Cl_t Cl_0;                             // Empty clause holder
Pa_t Pa_0;                             // Empty parameter holder
for(int state=0;;) {                   // And walk the syntax graph...
  Lx.GetTok(Td);                       // Get the next token
                                       // No exceptional cases (EOF in table)
  if (Lx.IsError(Td)) problem.prob = true;
                                       // Map to array index:
  toktyp = t6;                         // (Else)
  if (Lex::IsStr(Td.t)) toktyp = t0;   // Reduction functions
  if (Lex::IsOp(Td.t))  toktyp = t5;
  switch (Td.t) {                      // Explicit overrides of reduction fns
    case Lex::Sy_div  : toktyp = t1;  break;
    case Lex::Sy_AS   : toktyp = t2;  break;
    case Lex::Sy_cmma : toktyp = t3;  break;
    case Lex::Sy_EOF  : 
    case Lex::Sy_EOR  : toktyp = t4;  break;
    case Lex::Sy_cmnt : toktyp = t7;  break;
    default           :               break;
  }
  next = table[state][toktyp];         // Make the transition
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  :                                                             break;
    case X  : problem.prob = true;                                        break;
    case 1  : Co = Td.s;                                                  break;
    case 2  : Cl_v.push_back(Cl_0);                                       break;
    case 4  : Cl_v.back().Cl = Td.s;                                      break;
    case 5  : Cl_v.back().Pa_v.push_back(Pa_0);
              Cl_v.back().Pa_v.back().Val = Td.s;                         break;
    case 6  : Cl_v.back().Pa_v.push_back(Pa_0);
              Cl_v.back().Pa_v.back().Op = Td.s;                          break;
    case 7  : Cl_v.back().Pa_v.back().Val = Td.s;                         break;
    default : problem.prob = true;                                        break;
  }
  if (problem.prob==true) {            // Store problem location and bail
    Lx.GetLC(problem.lin,problem.col); // Line is irrelevant here
    return;
  }
  if ((state=next.ns)==R) break;       // Legitimate exit
}
Fixup();
}

//------------------------------------------------------------------------------

Cli::~Cli()
{
}

//------------------------------------------------------------------------------

void Cli::Dump(FILE * fp)
{
fprintf(fp,"Cli dump+++++++++++++++++++++++++++++++++++++++++\n");   fflush(fp);
fprintf(fp,"Problem : ");                                            fflush(fp);
if (problem.prob) fprintf(fp,"TRUE Line %d Col %d\n",problem.lin,problem.col);
else fprintf(fp,"FALSE\n");
fprintf(fp,"Orig = ||%s||\n",Orig.c_str());
fprintf(fp,"Co = ||%s||\n",Co.c_str());                              fflush(fp);
for (unsigned c=0;c<Cl_v.size();c++) {
  fprintf(fp,"  Cl[%u] = ||%s||\n",c,Cl_v[c].Cl.c_str());            fflush(fp);
  for (unsigned p=0;p<Cl_v[c].Pa_v.size();p++)
    fprintf(fp,"    Pa_v[%u] = ||%s||%s||\n",
                p,Cl_v[c].Pa_v[p].Op.c_str(),
                  Cl_v[c].Pa_v[p].Val.c_str()); fflush(fp);
}
fprintf(fp,"Cli dump-----------------------------------------\n");   fflush(fp);
}

//------------------------------------------------------------------------------

bool Cli::Err(int & rl,int &rc)
// Return error status: yes/no, line (the answer is 1) & column
{
rl = problem.lin;
rc = problem.col;
return problem.prob;
}

//------------------------------------------------------------------------------

vector<Cli> Cli::Expand()
// Routine to take the current - possibly multi-claused - command line, and copy
// it as an ordered set (well, vector, actually) of commands, each with one
// clause.
// Recall that the entire structure is auto variables, (nothing dynamic), so
// there's a hell of a lot of behind-the-scenes copying here.
// But the objects are intended to be tiny.....
{
unsigned cl = Cl_v.size();             // Number of copies = number of clauses
vector<Cli> VCli;                      // Holder for all the answers
for (unsigned i=0;i<cl;i++) {          // One copy at a time...
  VCli.push_back(Cli(""));             // Empty class
  VCli.back().Cl_v.push_back(Cl_v[i]); // Copy the relevant clause vector
  VCli.back().Co = Co;                 // Copy the command string itself
  string Orig = Co+" /"+Cl_v[i].Cl;    // Rebuild generating string
  if (!Cl_v[i].Pa_v.empty()) Orig+="=";// Any parameters at all?
  WALKVECTOR(Pa_t,Cl_v[i].Pa_v,j)      // No, really
    Orig += ((*j).Op+(*j).Val+(j==Cl_v[i].Pa_v.end()-1?"":","));
  VCli.back().Orig = Orig;             // And load the output object
}
return VCli;                           // Shove it into the arms of the caller
}

//------------------------------------------------------------------------------

list<Cli> Cli::File(string s,bool exp)
// Take a file, and turn it into a vector of Cli objects.
// If there's a problem anywhere, processing stops and returns whatever was
// successfuly interpreted.
// exp = false : the Cli objects are as in the file
// exp = true  : the Cli objects are expanded
{
list<Cli> LCli;                        // Results container
int l,c;
FILE * fp = fopen(s.c_str(),"r");
if (fp==0) return LCli;                // Open the file if we can
static const unsigned SIZE = 512;      // Get over it
char buf[SIZE];
for(;;) {                              // One line at a time....
  char * ps = fgets(buf,SIZE-1,fp);    // Pull in the data; ends with "/n/0"
  if (ps==0) break;                    // EOF?
  Cli C(ps);                           // Build a single Cli object
  if (C.Err(l,c)) break;               // Line translate OK?
  if (!exp) LCli.push_back(C);         // No expansion required
  else {                               // Expansion required
    vector<Cli> Vex = C.Expand();
    for(unsigned i=0;i<Vex.size();i++) LCli.push_back(Vex[Vex.size()-i-1]);
  }
}
fclose(fp);
return LCli;
}

//------------------------------------------------------------------------------

void Cli::Fixup()
// An inelegant appendage to a rather elegant class, though I say so myself.
// If the user puts a wildcard ("*") in as a string literal, it gets (quite
// correctly) interpreted as op="*" val="", which is (probably) not what the
// user intended. So here we move the operator over if the string is empty......

{
WALKVECTOR(Cl_t,Cl_v,i)
  WALKVECTOR(Pa_t,(*i).Pa_v,j)
    if ((*j).Val.empty()) {
      (*j).Val=(*j).Op;
      (*j).Op.clear();
    }
}

//------------------------------------------------------------------------------

string Cli::GetC(unsigned i,string s)
{
return Get<string>(Co_v,i,s).second;
}

//------------------------------------------------------------------------------

bool Cli::StrEq(string s0, string s1,unsigned len)
// Case insensitive, trimmed string equality test. Note call-by-value; originals
// are not changed.
{
transform(s0.begin(),s0.end(),s0.begin(),::tolower);
s0.resize(len,'\0');
transform(s1.begin(),s1.end(),s1.begin(),::tolower);
s1.resize(len,'\0');
return s0==s1;
}

//------------------------------------------------------------------------------

void Cli::Trim(unsigned len)
// Turn the command and all the clause names (but NOT the parameters) into
// lower case, len characters.
// Note the weirdness: if you resize a string length 3 to 4 with padding nulls,
// you get a string where str.size() == 4, but strlen(str.c_str()) == 3.
{
                                       // Sort out the command
transform(Co.begin(),Co.end(),Co.begin(),::tolower);    // Guess
Co.resize(len,'\0');                  // Truncate to len chars
WALKVECTOR(Cl_t,Cl_v,i) {             // Do the same for the clause vector
  transform((*i).Cl.begin(),(*i).Cl.end(),(*i).Cl.begin(),::tolower);
  (*i).Cl.resize(len,'\0');
}
}

//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

void Cli::Pa_t::Dump(FILE * fp,unsigned p)
{
fprintf(fp,"    Pa_v[%u] = ||%s||%s||\n",p,Op.c_str(),Val.c_str());
fflush(fp);
}

//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

void Cli::Cl_t::Dump(FILE * fp,unsigned c)
{
fprintf(fp,"  Cl[%u] = ||%s||\n",c,Cl.c_str());
for (unsigned p=0;p<Pa_v.size();p++) Pa_v[p].Dump(fp,p);
fflush(fp);
}

//------------------------------------------------------------------------------

string Cli::Cl_t::GetO(unsigned i,string s)
// Get indexed parameter operator. If the index is OOR for whatever reason,
// return the default. It's a specialisation of the generic "Get" in flat.cpp
// (Which, in turn, is the C++98 equivalent of the C++11 vector::at)
{
return Get<Cli::Pa_t>(Pa_v,i,Cli::Pa_t(s,"")).second.Op;
// Now you're just showing off.
}

//------------------------------------------------------------------------------

string Cli::Cl_t::GetP(unsigned i,string s)
// Get indexed parameter. If the index is OOR for whatever reason,
// return the default.
{
return Get<Cli::Pa_t>(Pa_v,i,Cli::Pa_t("",s)).second.Val;
}

//==============================================================================


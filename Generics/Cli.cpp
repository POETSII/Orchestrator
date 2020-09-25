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
// Constructor from single string
{
DoIt(instr);
}

//------------------------------------------------------------------------------

Cli::Cli(int argc, char* argv[])
// Command line parameter interpreter. Program command line parameters are
// nailed together into one large string and processed like the rest.
// Note the command line parameters are separated with a ' ', so that stuff like
// ">program /a b" gets in properly
{
string cls;                            // Command line argument string
                                       // Nail them all together
for(int i=1;i<argc;i++) cls += (string(argv[i]) + string(" "));
                                       // Prepend the invoking program (dquotes)
cls = string("\"") + string(argv[0]) + string("\" ") + cls;

// The whole thing now looks like
//        "E:\Grants\this\that\tother\stuff.exe" /oink=eek /3 = 2
// and so on. The invoking command line goes in as the original command, which
// is largely unnecessary because the called program might just as well read
// argv[0] anyway.

DoIt(cls);
}

//------------------------------------------------------------------------------

Cli::~Cli()
{
}

//------------------------------------------------------------------------------

void Cli::DoIt(string instr)
// Construction from string
{
Orig = instr;                          // Save the input data
static const int   X = -1;             // Cosmic error table entry
static const int   R = -2;             // Cosmic return table entry
Lex Lx;
Lex::tokdat        Td;                 // Current lexer token
Lx.SetFile(instr);                     // Point the lexer at input string
Lx.SetNFlag(true);                     // Tune lexer to recognise numbers

                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4,t5,t6,t7,t8} toktyp;
struct duple {int ns,ac;} next;
static duple table[7][t8+1] =
// Incident symbol
//   An      /      =      ,    EOF     Op   else   Cmnt     ::
//   t0     t1     t2     t3     t4     t5     t6     t7     t8
{{{1, 1},{3, 2},{X, X},{X, X},{R, 0},{X, X},{X, X},{R, 3},{X, X}},  // 0   Next
 {{X, X},{3, 2},{X, X},{X, X},{R, 0},{X, X},{X, X},{R, 3},{X, X}},  // 1   state
 {{6, 7},{3, 2},{X, X},{5, 0},{R, 0},{X, X},{X, X},{R, 3},{X, X}},  // 2
 {{4, 4},{X, X},{X, X},{X, X},{R, 0},{X, X},{X, X},{R, 3},{X, X}},  // 3
 {{X, X},{3, 2},{5, 0},{X, X},{R, 0},{X, X},{X, X},{R, 3},{X, X}},  // 4
 {{6, 5},{3, 2},{X, X},{X, X},{R, 0},{2, 6},{X, X},{R, 3},{X, X}},  // 5
 {{X, X},{3, 2},{X, X},{5, 0},{R, 0},{X, X},{X, X},{R, 3},{2, 0}}}; // 6

Cl_t Cl_0;                             // Empty clause holder
Pa_t Pa_0;                             // Empty parameter holder
for(int state=0;;) {                   // And walk the syntax graph...
  Lx.GetTok(Td);                       // Get the next token
                                       // No exceptional cases (EOF in table)
  if (Lx.IsError(Td)) problem.lin = 1;
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
    case Lex::Sy_dcol : toktyp = t8;  break;
    default           :               break;
  }
  next = table[state][toktyp];         // Make the transition
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  :                                                             break;
    case X  : problem.lin = 1;                                            break;
    case 1  : Co = Td.s;                                                  break;
    case 2  : Cl_v.push_back(Cl_0);                                       break;
    case 3  : Cmnt = Td.s;                                                break;
    case 4  : Cl_v.back().Cl = Td.s;                                      break;
    case 5  : Cl_v.back().Pa_v.push_back(Pa_0);
              Cl_v.back().Pa_v.back().Va_v.push_back(Td.s);               break;
    case 6  : Cl_v.back().Pa_v.push_back(Pa_0);
              Cl_v.back().Pa_v.back().Op = Td.s;                          break;
    case 7  : Cl_v.back().Pa_v.back().Va_v.push_back(Td.s);               break;
    default : problem.lin = 1;                                            break;
  }
  if (problem.lin>=0) {                // Store problem location and bail
    Lx.GetLC(problem.lin,problem.col); // Line is irrelevant here
    return;
  }
  if ((state=next.ns)==R) break;       // Legitimate exit
}
Fixup();                               // Promote single operators
}

//------------------------------------------------------------------------------

void Cli::Dump(unsigned off,FILE * fp)
{
string s = string(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n%sCli ++++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sProblem : ",os);
if (problem.lin>=0)
  fprintf(fp,"%sERROR Line %d Col %d\n%s\n",
         os,problem.lin,problem.col,(string(problem.col+7,' ')+"\\v/").c_str());
else fprintf(fp,"%s.NO.\n",os);
fprintf(fp,"%sOrig = ||%s||\n",os,Orig.c_str());
fprintf(fp,"%sCmnt = ||%s||\n",os,Cmnt.c_str());
fprintf(fp,"%sCo   = ||%s|| size %lu\n",os,Co.c_str(),Co.size());
for (unsigned c=0;c<Cl_v.size();c++) {
  string sCl = Cl_v[c].Cl;
  fprintf(fp,"%s  Cl[%u] = ||%s|| size %lu\n",os,c,sCl.c_str(),sCl.size());
  for (unsigned p=0;p<Cl_v[c].Pa_v.size();p++) {
    fprintf(fp,"%s    Pa_v[%u] = ||%s||",os,p,Cl_v[c].Pa_v[p].Op.c_str());
    for (unsigned v=0;v<Cl_v[c].Pa_v[p].Va_v.size();v++) {
      fprintf(fp,"%s",Cl_v[c].Pa_v[p].Va_v[v].c_str());
      if (v!=Cl_v[c].Pa_v[p].Va_v.size()-1) fprintf(fp,"::");
    }
    fprintf(fp,"||\n");
  }
}
fprintf(fp,"%sCli ----------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void Cli::Err(int & rl,int &rc)
// Return error status: yes/no, line (the answer is 1) & column
{
rl = problem.lin;
rc = problem.col;
return;
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
  VCli.back().Cmnt = Cmnt;             // Copy any comment string
  string Orig = Co+" /"+Cl_v[i].Cl;    // Rebuild generating string
  if (!Cl_v[i].Pa_v.empty()) Orig+=" = ";// Any parameters at all?
  WALKVECTOR(Pa_t,Cl_v[i].Pa_v,j) {     // No, really
    Orig += (*j).Op;
    WALKVECTOR(string,(*j).Va_v,k) {
      Orig += (*k);
      if (k!=(*j).Va_v.end()-1) Orig += "::";
    }
    if (j!=Cl_v[i].Pa_v.end()-1) Orig += ", ";
  }
  VCli.back().Orig = Orig;             // And load the output object
  VCli.back().problem = problem;       // (.problem has the line number in it)
}
if (Cl_v.empty()) VCli.push_back(*this); // No clauses - just copy it
return VCli;                           // Shove it into the arms of the caller
}

//------------------------------------------------------------------------------

list<Cli> Cli::File(string s,bool exp)
// Take a file, and turn it into a vector of Cli objects.
// If there's a problem anywhere, processing stops and returns whatever was
// successfuly interpreted, with the dud command at the end.
// exp = false : the Cli objects are as in the file (default)
// exp = true  : the Cli objects are expanded
{
list<Cli> LCli;                        // Results container
FILE * fp = fopen(s.c_str(),"r");
if (fp==0) return LCli;                // Open the file if we can
static const unsigned SIZE = 512;      // Get over it
char buf[SIZE];
for(int L=1;;L++) {                    // One line at a time....
  char * ps = fgets(buf,SIZE-1,fp);    // Pull in the data; ends with "\n\0"
  if (ps==0) break;                    // EOF?
                                       // Lose the '\n', sometimes.
  for (unsigned i=0;i<SIZE;i++) {
      if (ps[i]=='\n') ps[i]='\0';
      if (ps[i]=='\0') break;
  }
  Cli C(ps);                           // Build a single Cli object
  if (C.problem.lin>=0) {              // Line translation not OK:
    C.problem.lin = L;                 // Overwrite error line with file line
    LCli.push_back(C);                 // Push dud command onto list
    break;                             // And bail
  }
  if (!exp) LCli.push_back(C);         // No expansion required
  else {                               // Expansion required
    vector<Cli> Vex = C.Expand();
    for(unsigned i=0;i<Vex.size();i++) LCli.push_back(Vex[i]);
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
    if ((*j).Va_v.empty()) {
      (*j).Va_v.push_back((*j).Op);
      (*j).Op.clear();
    }
}

//------------------------------------------------------------------------------
               /*
string Cli::GetC(unsigned i,string s)
//
{
return Get<string>(Co_v,i,s).second;
}
                 */
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
// lower case, "len" characters long.
// You can - if you must - Trim(0), which does what you'd expect.
{
                                       // Sort out the command
transform(Co.begin(),Co.end(),Co.begin(),::tolower);    // Guess
if (Co.size()>len) Co.resize(len);     // Truncate to len chars
WALKVECTOR(Cl_t,Cl_v,i) {              // Do the same for the clause vector
  transform((*i).Cl.begin(),(*i).Cl.end(),(*i).Cl.begin(),::tolower);
  if ((*i).Cl.size()>len)(*i).Cl.resize(len);
}
}

//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

string Cli::Pa_t::Concatenate()
// Returns concatenated copy of elements in the parameter vector.
{
string cat = "";
WALKVECTOR(string,Va_v,i)
{
if(i!=Va_v.begin()) cat+="::";
cat+=*i;
}
return cat;
}

//------------------------------------------------------------------------------

void Cli::Pa_t::Dump(FILE * fp,unsigned p)
{
fprintf(fp,"    Pa_v[%u] = ||%s||--||",p,Op.c_str());
WALKVECTOR(string,Va_v,i) fprintf(fp,"::%s::",(*i).c_str());
fprintf(fp,"\n");
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
// Get zeroth indexed parameter. If the index is OOR for whatever reason,
// return the default.
// This is just a shorthand for Cli::Cl_t::GetP(unsigned,string)[0], but it's
// so common it's worth having a seperate one
{
return GetPv(i,s)[0];
}

//------------------------------------------------------------------------------

vector<string> Cli::Cl_t::GetPv(unsigned i,string s)
// Get indexed parameter. If the index is OOR for whatever reason,
// return the default.
{
return Get<Cli::Pa_t>(Pa_v,i,Cli::Pa_t("",s)).second.Va_v;
}

//==============================================================================

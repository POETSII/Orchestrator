#ifndef __CliH__H
#define __CliH__H

#include "lex.h"
#include <vector>
#include <list>
using namespace std;

//==============================================================================
/* General purpose command line handler. It expects a single string, containing
stuff of the form

command /clause0=[op]par0,[op]par1.... /clause1=[op]par0,[op]par1....

(If it doesn't find it - syntax failure - it sets the "problem" structure and
bails. It doesn't attempt error recovery. problem.lin comes from lex.h and is
always 1 in this object.)

There is an alternative constructor for program command line arguments that
concatenates arguments 1..all into one huge string (with ' ' spacers)

The entire datastructure of the class is designed to be burgled; there is
nothing private and there are no release access functions, apart from Err().

Everything is strings; it's up to the calling code to translate strings to
numbers and operators.
*/

class Cli {
public:
                   Cli();
                   Cli(string);
                   Cli(int,char *[]);
virtual ~          Cli(void);
void               DoIt(string);
void               Dump(unsigned = 0,FILE * = stdout);
bool               Empty() { return Co.empty(); }
void               Err(int &,int &);   // Direct copy of problem struct
vector<Cli>        Expand();           // Expand the clauses -> own command
string             GetC(unsigned=0,string=string());
static list<Cli>   File(string,bool=false);  // Create a Cli vector from a file
void               Fixup();            // Iron the datastructure
static bool        StrEq(string,string,unsigned=4); // Equality test
void               Trim(unsigned=4);   // Regularise the strings

string             Orig;               // Copy of input string
string             Cmnt;               // Any terminating comment
string             Co;                 // Command
struct Pa_t {                          // Parameter holder
  Pa_t(){}
  Pa_t(string _O,string _V):Op(_O) { Va_v.push_back(_V); }
  string           Concatenate();      // Returns concatenated copy
  void             Dump(FILE * = stdout,unsigned=0);
  string           Op;                 // Parameter operator
  vector<string>   Va_v;               // Parameter name vector
};
struct Cl_t {                          // Clause holder
  void             Dump(FILE * = stdout,unsigned=0);
  string           GetO(unsigned=0,string=string());  // Indexed param operator
  string           GetP(unsigned=0,string=string());  // Zeroth compound name
  vector<string>   GetPv(unsigned=0,string=string()); // Indexed param vector
  string           Cl;                 // Clause name
  vector<Pa_t>     Pa_v;               // Bunch of parameters
};
vector<Cl_t>       Cl_v;               // Bunch of clauses in command
struct prob_t {                        // Problem structure
  prob_t():lin(-1),col(-1){}
  int              lin;                // On line (1, here)
  int              col;                // Guess
} problem;                             // The error holder itself

};

//==============================================================================

#endif

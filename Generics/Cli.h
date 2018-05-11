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

The entire datastructure of the class is designed to be burgled; there is
nothing private and there are no release access functions, apart from Err().

Everything is strings; it's up to the calling code to translate strings to
numbers and operators.
*/

class Cli {
public:
                   Cli();
                   Cli(string);
virtual ~          Cli(void);
void               Dump(FILE * = stdout);
bool               Err(int &,int &);   // Direct copy of problem struct
vector<Cli>        Expand();           // Expand the clauses -> own command
static list<Cli>   File(string,bool=false);  // Create a Cli vector from a file
void               Fixup();            // Iron the datastructure
static bool        StrEq(string,string,unsigned=4); // Equality test
void               Trim(unsigned=4);   // Regularise the strings

string             Orig;               // Copy of input string
string             Co;                 // Command
struct Pa_t {                          // Parameter holder
  Pa_t(){}
  Pa_t(string _O,string _V):Op(_O),Val(_V){}
  void             Dump(FILE * = stdout,unsigned=0);
  string           Op;                 // Parameter operator
  string           Val;                // Parameter name
};
struct Cl_t {                          // Clause holder
  void             Dump(FILE * = stdout,unsigned=0);
  string           GetO(unsigned=0,string=string());// Get indexed parameter operator
  string           GetP(unsigned=0,string=string());// Get indexed parameter
  string           Cl;                 // Clause name
  vector<Pa_t>     Pa_v;               // Bunch of parameters
};
vector<Cl_t>       Cl_v;               // Bunch of clauses in command
struct prob_t {                        // Problem structure
  prob_t():prob(false),lin(-1),col(-1){}
  bool             prob;               // There is/isn't a problem
  int              lin;                // On line (1, here)
  int              col;                // Guess
} problem;                             // The error holder itself

};

//==============================================================================

#endif

//------------------------------------------------------------------------------
// class Lex;

#ifndef __Xlex__H
#define __Xlex__H

#include <stdio.h>
#include <string>
#include <vector>
#include <list>
#include <cstdlib>
using namespace std;
#include "dfprintf.h"

//==============================================================================

class Lex
{
public:
enum Sytype {S_0 = 0, S_00, SE_XXX,           // Types of token
SE_FNA ,SE_SNF ,SE_BUF ,Sy_amp ,Sy_AND ,Sy_AS  ,Sy_AT  ,Sy_back,Sy_BSTR,Sy_cat ,
Sy_cmnt,Sy_col ,Sy_cmma,Sy_ddot,Sy_div ,Sy_dol ,Sy_dot ,Sy_dqut,Sy_EOF ,Sy_EOR ,
Sy_EQ  ,Sy_exp ,
Sy_FSTR,Sy_GE  ,Sy_GT  ,Sy_hash,Sy_HSTR,Sy_ISTR,Sy_lbrc,Sy_LE  ,Sy_line,Sy_lrnb,
Sy_lshf,Sy_lsqb,Sy_LT  ,Sy_max ,Sy_min ,Sy_mult,Sy_NE  ,Sy_OR  ,Sy_OSTR,Sy_pcnt,
Sy_plng,Sy_plus,Sy_pnd ,Sy_qst ,Sy_rbrc,Sy_rrnb,Sy_rshf,Sy_rsqb,Sy_semi,Sy_squt,
Sy_SSTR,Sy_STR ,Sy_sub ,Sy_T3  ,Sy_tlda,Sy_dcol,
Xy_sdcl,Xy_edcl,Xy_eel0,Xy_scdt,Xy_ecdt,Xy_eel1,Xy_scmt,Xy_ecmt,Sy_XXXX};
static const char * Sytype_str[Sy_XXXX+1];
static const char * Sytype_dbg[Sy_XXXX+1];

struct tokdat {
  tokdat():s(string("")),t(S_0),l(0),c(0){};
  void Dump(FILE * fp = stdout) {
    fprintf(fp,"tokdat:\n.s=%s, .p='%s', .t=%s, .l=%d, .c=%d\n",
            s.c_str(),p.c_str(),Sytype_str[t],l,c);
    fflush(fp);
  };
  string s;                            // Generating string
  string p;                            // Whitespace preamble
  Sytype t;                            // Generated token
  int    l;                            // Stream line address
  int    c;                            // Stream column address
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class History
// Ee, it's just like writing PASCAL again - everything backwards....
// History is a subclass of Lex.
// History maintains a circular buffer - mod HS - that goes from 0 to (HS-1) and
// wraps around.
{
public:
       History():nfc(0),pflag(0){}
void   Dump(vector<string> &,int=10);
void   Dump(FILE * = stdout,int=10);   // Diagnostic
long   GetSize();                      // Length of internal buffer
bool   GetTok(tokdat &);               // Pull a token from the buffer
void   push_back(int = 1);             // Push the 'next' pointer back
void   PutTok(tokdat);                 // Push a token onto end of the buffer

private:
static const int HS;                   // Buffer size (max use as of 4/4/3: 2)
tokdat td[256];                        // The buffer itself (g++ ????)
int    nfc;                            // Next Free Cell pointer
int    pflag;                          // Pushback offset (0 = do nothing)
} Hst;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Back to the Lex class:
                Lex();                 // Initialise, but still needs a file
                Lex(FILE *);           // Initialise to open file
                Lex(string);           // Initialise to string source
                Lex(char *);           // Initialise to unopen filename
virtual ~       Lex();

void            Dump(FILE * = stdout);
char            Gc();                  // Pull in single character
long            GetCtr();              // Return token counter
Sytype          GetErr(){ return err; }// Expose internal error flag
void            GetLC(int &,int &);    // Expose (line,col)
string          GetLCs();              // Pretty-print (line,col)
void            GetTok(tokdat &);      // Return next token
void            GetTokNC(tokdat &);    // Return next non-comment token
void            Init();                // Guess
bool            IsBDigit(char);        // Is the character a binary digit ?
bool            IsDDigit(char);        // Decimal ?
bool            IsEOF(tokdat &);       // EOF:
bool            IsError(tokdat &);     // Is it an error token?
bool            IsF1Digit(char);       // Floating point initial character ?
bool            IsF2Digit(char);       // Floating point non-initial character ?
bool            IsHDigit(char);        // Hex digit?
bool            IsODigit(char);        // Octal digit?
static bool     IsOp(Sytype);          // Is this a UIF operator?
bool            IsSDigit(char);        // StdLogic digit?
static bool     IsStr(Sytype);         // Is this a UIF string?
static bool     IsStrInt(Sytype);      // Is this in integer-type string?
static string   Join(string,string);
//###############
//char          Peek();                // Look ahead in character stream
char            Peek(unsigned = 1);
void            push_back(int = 1);    // Push token back
void            Reset();               // Clear internal error flag
void            SetCChar(char=(char)0);// Set continuation character
void            SetCFlag(bool);        // Treat "//.... as a comment?
void            SetCtr(long);          // Set token counter
void            SetDFlag(bool);        // Treat '.' as an alphanumeric?
void            SetFile();             // Disconnect parser
void            SetFile(FILE *);       // Associate lexer with ... a file stream
void            SetFile(char *);       // ... a file name
void            SetFile(string);       // ... a string (NOT a file)

void            SetMFlag(bool);        // Allow '-' as an alphanumeric?
void            SetNFlag(bool);        // Treat all numbers as strings?
string          SkipTo(Sytype);        // Pull in an arbitrary token string
string          SkipTo(char);          // Pull in an arbitrary character string
static unsigned Str2Uint(string);      // Generic string converter
private:
FILE   * fp;                           // Internal stream pointer
string   fname;                        // Filename (if known)
tokdat   T;                            // Current stream token + assoc data
struct   flagtype {
  bool   cflag;                        // Treat '//' as a single token
  bool   mflag;                        // Accept '-' in unquoted strings
  bool   nflag;                        // Treat 'numbers' as strings
  bool   dflag;                        // Treat '.' as an alphanumeric
  char   cont;                         // Ignore next \n character
} flags;
//#############
//char     next;                         // Lookahead character
list<char> next;
unsigned CHARBUF;
char     last;                         // Lookback character
Sytype   err;                          // Internal error flag
long     count;                        // 'Tokens handed out' counter
//bool     stringflag;                   // File or string input stream?
char     intype;                       // File name:'F' stream:'*' string:'S'
                                       // Nothing:'0'
};

//==============================================================================

#endif

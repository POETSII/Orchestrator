//------------------------------------------------------------------------------

#include "lex.h"
#include "flat.h"   // For file_exists and file_readable

//------------------------------------------------------------------------------

const char * Lex::Sytype_str[] = {
"Error: No symbol","","Generic error","Error: File not available",
"Error: Stream not found","Error: Lex buffer overflow",
"&","&&",
"=","@",
"\\","[Binary string]",
"^","//",
":",",","..",
"/","$",
".","""",
"[End of file]","\\n",
"==","**","[Floating pt string ]",
">=",">",
"#","[Hexadecimal string ]",
"[Integer string]","{",
"<=","|",
"(","<<",
"[","<",
"|\\","|/",
"*","!=",
"||","[Octal string]",
"%","!",
"+","£",
"?","}",
")",">>",
"]",";",
"'","[Std logic vector   ]",
"[String]","-",
"?:","~",
"::",
"<?","?>",
"/>","<![CDATA[",
"]]>","</",
"<!--","-->"};

const char * Lex::Sytype_dbg[] = {
"S_0    [No symbol          ]" ,"S_00   [Blank              ]",
"SE_XXX [Generic error      ]" ,"SE_FNA [File not available ]",
"SE_SNF [Stream not found   ]" ,"SE_BUF [Lex buffer overflow]",
"Sy_amp [&                  ]" ,"Sy_AND [&&                 ]",
"Sy_AS  [=                  ]" ,"Sy_AT  [@                  ]",
"Sy_back[\\                  ]","Sy_BSTR[Binary string      ]",
"Sy_cat [^                  ]" ,"Sy_cmnt[//                 ]",
"Sy_col [:                  ]" ,"Sy_cmma[,                  ]",
"Sy_ddot[..                 ]" ,
"Sy_div [/                  ]" ,"Sy_dol [$                  ]",
"Sy_dot [.                  ]" ,"Sy_dqut[""                 ]",
"Sy_EOF [End of file        ]" ,"Sy_EOR [End of record      ]",
"Sy_EQ  [==                 ]" ,"Sy_exp [**                 ]",
"Sy_FSTR[Floating pt string ]",
"Sy_GE  [>=                 ]" ,"Sy_GT  [>                  ]",
"Sy_hash[#                  ]" ,"Sy_HSTR[Hexadecimal string ]",
"Sy_ISTR[Integer string     ]" ,"Sy_lbrc[{                  ]",
"Sy_LE  [<=                 ]" ,"Sy_line[|                  ]",
"Sy_lrnb[(                  ]" ,"Sy_lshf[<<                 ]",
"Sy_lsqb[[                  ]" ,"Sy_LT  [<                  ]",
"Sy_max [|\\                 ]","Sy_min [|/                 ]",
"Sy_mult[*                  ]" ,"Sy_NE  [!=                 ]",
"Sy_OR  [||                 ]" ,"Sy_OSTR[Octal string       ]",
"Sy_pcnt[%                  ]" ,"Sy_plng[!                  ]",
"Sy_plus[+                  ]" ,"Sy_pnd [£                  ]",
"Sy_qst [?                  ]" ,"Sy_rbrc[}                  ]",
"Sy_rrnb[)                  ]" ,"Sy_rshf[>>                 ]",
"Sy_rsqb[]                  ]" ,"Sy_semi[;                  ]",
"Sy_squt['                  ]" ,"Sy_SSTR[Std logic vector   ]",
"Sy_STR [String             ]" ,"Sy_sub [-                  ]",
"Sy_T3  [?:                 ]" ,"Sy_tlda[~                  ]",
"Sy_dcol[::                 ]" ,
"Xy_sdcl[<?                 ]" ,"Xy_edcl[?>                 ]",
"Xy_eel0[/>                 ]" ,"Xy_scdt[<![CDATA[          ]",
"Xy_ecdt[]]>                ]" ,"Xy_eel1[</                 ]",
"Xy_scmt[<!--               ]" ,"Xy_ecmt[-->                ]"};

//------------------------------------------------------------------------------
// Input channel assignment operates on a leave-it-as-you-found-it basis.
// The lexer can read from a...
// ...file : you give it the filename, lex opens, reads and closes the file
// ...file stream : you open the file and give it the stream, lex leaves the
// stream open
// ...string : you give it the string
// Initially the input channel is unassigned, and the lexer will return SNF to
// any interrogation.
//------------------------------------------------------------------------------

Lex::Lex()
// Generic constructor. The lexer can't be used until it has an input stream
// associated with it by SetStr.
{
Init();                                // Initialise everything
}

//------------------------------------------------------------------------------

Lex::Lex(FILE * ffp)
// Create the thing and attach the file which is assumed to be open for reading.
{
Init();                                // Initialise everything
fp = ffp;                              // Assign the input stream
intype = '*';                          // Input channel is a file stream
}

//------------------------------------------------------------------------------

Lex::Lex(string name)
// Create the thing, and attach it to a string
{
Init();                                // Initialise everything
fname = name;
intype = 'S';
}

//------------------------------------------------------------------------------

Lex::Lex(char * name)
// Create the thing, open the file for read and attach.
{
Init();                                // Initialise everything
fname = name;                          // Store file name
fp = fopen(name,"r");                  // Assign the input stream
if (fp==0) intype = '0';               // Didn't work?
intype = 'F';                          // Input channel is a file name
}

//------------------------------------------------------------------------------

Lex::~Lex()
{
switch (intype) {
  case 'F' : fclose(fp); fp=0; return; // It was a file; our stream pointer
  case '*' : return;                   // It was a stream pointer; leave alone
  case 'S' : return;                   // It was a string; no stream pointer
  case '0' : return;                   // We never did anything anyway
  default  : return;                   // And again
}
//if (fp != 0) fclose(fp);
}

//------------------------------------------------------------------------------

void Lex::Dump(FILE * fp)
{
fprintf(fp,"Character buffer:\n||");
WALKLIST(char,next,i)fprintf(fp,"%c",*i);
fprintf(fp,"||\n");
}

//------------------------------------------------------------------------------

char Lex::Gc()
// This is the low-level (core) input-getting routine. It is the ONLY place the
// input is actually accessed; thus we distinguish here whether we are reading
// from a string or a file.
// Pull the next character from the input stream (actually, return the lookahead
// character and pull the next lookahead character off the input stream).
{
char c;     /*                           // 'Current' character
if (stringflag) {                      // We're reading from a string
  if (fname.size()==0) return EOF;     // Special case - empty string
  if (T.c>=int(fname.size())) return EOF;  // End of string
                                       // Initialise the lookahead if necessary
  if (next == (char)0) next = fname[0];
  c = next;                            // Lookahead -> current
  if (++T.c<(int)fname.size()) next = fname[T.c];
  else next = char(0);                 // Read new lookahead, update col address
}
else                                   // We're reading from a file
{
                                       // Initialise the lookahead if necessary
  if (next == (char)0) next = (char)fgetc(fp);
  c = next;                            // Lookahead -> current
  next = (char)fgetc(fp);              // Read new lookahead
  T.c++;                               // Update column address
  if (last=='\n') {                    // If it's a newline, update line address
    T.c = 1;                           // and reset column address
    T.l++;
  }
}
*/

switch (intype) {
  case '0' : return EOF;
  case 'F' :
  case '*' : for(;next.size()<CHARBUF;) next.push_back((char)fgetc(fp));
             c = next.front();
             next.pop_front();
             if (c==EOF) return c;
             T.c++;
             if (last=='\n') {
               T.c = 1;
               T.l++;
             }
             break;
  case 'S' : if (fname.size()==0) return EOF;
             if (T.c>=int(fname.size())) return EOF;
             for(int io=1;
                 ((next.size()<CHARBUF)&&(T.c+next.size()<fname.size()));
                 io++)
               next.push_back(fname[T.c+next.size()]);
             T.c++;
             c = next.front();
             next.pop_front();
             break;
  default  : return EOF;
}

/*
if (stringflag) {
  if (fname.size()==0) return EOF;
  if (T.c>=int(fname.size())) return EOF;
  for(int io=1;((next.size()<CHARBUF)&&(T.c+next.size()<fname.size()));io++)
    next.push_back(fname[T.c+next.size()]);
  T.c++;
  c = next.front();
  next.pop_front();
} else {
  for(;next.size()<CHARBUF;) next.push_back((char)fgetc(fp));
  c = next.front();
  next.pop_front();
  T.c++;
  if (last=='\n') {
    T.c = 1;
    T.l++;
  }
} */

last = c;
return c;                            // And finally return a character
}

//------------------------------------------------------------------------------

long Lex::GetCtr()
{
return count;
}

//------------------------------------------------------------------------------

void Lex::GetLC(int & l,int & c)
// Routine to hand out the current row (line) and column
{
l = T.l;
c = T.c;
}

//------------------------------------------------------------------------------

string Lex::GetLCs()
// Hand out the current line/col as a string
{
char buf[20];                          // Why 20? Stack free, me paranoid
sprintf(buf,"(%4u,%3u)",unsigned(T.l),unsigned(T.c));
return string(buf);
}

//------------------------------------------------------------------------------

void Lex::GetTok(tokdat & Td)
// The routine that does all the work. It hands out a sequence of tokens:
// The token itself is in "token" - well, it would be, really - even if it's
// only a single character. "token" can also contain the entire comment, if
// necessary, or an entire string literal.
// "sy" is the enumerated type telling you what type it is.
// (lin,col) give the coordinates in the file of the first character of the
// token.
{
                                       // Deal with the errors first:
if (intype=='0') err = SE_SNF;
if (err!=S_0) {
  Td.s = Sytype_str[err];
  Td.t = err;
  Td.l = 0;
  Td.c = 0;
//  Td.Dump();
  return;
}

if (Hst.GetTok(Td)) {                  // Retrieving from the history subsystem?
  count++;
//  TRACE(2) fprintf(ofp,"\nLex::Td.s=%s,\n  Td.t=%s, Td.l=%d, Td.c=%d, count=%d\n",
//                   Td.s.c_str(),Sytype_str[Td.t],Td.l,Td.c,count);
//  Td.Dump();
  return;
}

char * end;
char c;
                                       // OK, at least we have an input stream.
while (isspace((unsigned char)(c=Gc())))if (c=='\n') break;
T.s.clear();
if (c==flags.cont) for (;;) {          // Multiple blank continuations ?
  while ((c=Gc()) != '\n'){if (c==EOF) goto OUT;}            // Skip to EOR/EOF
  while ((c=Gc()) == ' ');             // Skip leading whitespace
  if (c!=flags.cont) break;
}
OUT:
switch (c) {
  case '!'  : switch (Peek()) {
                case '=' : T.s=Sytype_str[T.t=Sy_NE  ]; Gc(); break;  // !=
                default  : T.s=Sytype_str[T.t=Sy_plng];       break;  // !
              } break;
  case '"'  : T.t = Sy_dqut;            // Start of a quoted string
              if(Peek()=='\n') break;
              while (((c=Gc())!='"')&&(Peek()!='\n')) T.s.push_back(c);
              if((c!='"')&&(Peek()=='\n')) T.s.push_back(c);
              break;
  case '£'  : T.s=Sytype_str[T.t=Sy_pnd ]; break;
  case '$'  : while (IsHDigit(Peek())) T.s.push_back(Gc());
              T.t=Sy_HSTR;
              if (T.s.size()==0) T.s=Sytype_str[T.t=Sy_dol ];
              else T.s = "$" + T.s;
              break;
  case '%'  : while (IsBDigit(Peek())) T.s.push_back(Gc());
              T.t=Sy_BSTR;
              if (T.s.size()==0) T.s=Sytype_str[T.t=Sy_pcnt];
              else T.s = "%" + T.s;
              break;
  case '^'  : T.s=Sytype_str[T.t=Sy_cat ]; break;
  case '&'  : while (IsODigit(Peek())) T.s.push_back(Gc());
              T.t=Sy_OSTR;
              if (T.s.size()==0) {
                switch (Peek()) {
                  case '&' : T.s=Sytype_str[T.t=Sy_AND ]; Gc(); break;  // &&
                  default  : T.s=Sytype_str[T.t=Sy_amp ];       break;  // &
                }
              }
              else T.s = "$" + T.s;
              break;
  case '*'  : switch (Peek()) {
                case '*' : T.s=Sytype_str[T.t=Sy_exp ]; Gc(); break;  // **
                default  : T.s=Sytype_str[T.t=Sy_mult];       break;  // *
              } break;
  case '('  : T.s=Sytype_str[T.t=Sy_lrnb]; break;
  case ')'  : T.s=Sytype_str[T.t=Sy_rrnb]; break;
  case '-'  : switch (Peek()) {
                case '-' : switch (Peek(2)) {
                             case '>' : T.s=Sytype_str[T.t=Xy_ecmt ]; // -->
                                        Gc(); Gc();
                                        goto Sy_subout;
                             default  : ;
                           } break;
                default  : ;
              }
              T.s=Sytype_str[T.t=Sy_sub ];                            // -
  Sy_subout:  break;
  case '+'  : T.s=Sytype_str[T.t=Sy_plus]; break;
  case '='  : switch (Peek()) {
                case '=' : T.s=Sytype_str[T.t=Sy_EQ  ]; Gc(); break;  // ==
                default  : T.s=Sytype_str[T.t=Sy_AS  ];       break;  // =
              } break;
  case '{'  : T.s=Sytype_str[T.t=Sy_lbrc]; break;
  case '}'  : T.s=Sytype_str[T.t=Sy_rbrc]; break;
  case '['  : T.s=Sytype_str[T.t=Sy_lsqb]; break;
  case ']'  : switch (Peek()) {
                case ']' : switch (Peek(2)) {
                             case '>' : T.s=Sytype_str[T.t=Xy_ecdt];
                                        Gc(); Gc();
                                        goto Sy_rsqbout;
                             default  : ;
                           } break;
                default  : ;
              }
              T.s=Sytype_str[T.t=Sy_rsqb];                            // ]
  Sy_rsqbout: break;
  case ':'  : switch (Peek()) {
                case ':' : T.s=Sytype_str[T.t=Sy_dcol]; Gc(); break;  // ::
                default  : T.s=Sytype_str[T.t=Sy_col ];       break;  // :
              } break;
  case '@'  : while (IsSDigit(Peek())) T.s.push_back(Gc());
              T.t=Sy_SSTR;
              if (T.s.size()==0) T.s=Sytype_str[T.t=Sy_AT  ];
              else T.s = "@" + T.s;
              break;
  case '~'  : T.s=Sytype_str[T.t=Sy_tlda]; break;
  case ';'  : T.s=Sytype_str[T.t=Sy_semi]; break;
  case '\'' : T.s=Sytype_str[T.t=Sy_squt]; break;
  case '#'  : T.s=Sytype_str[T.t=Sy_hash]; break;
  case '<'  : switch (Peek()) {
                case '=' : T.s=Sytype_str[T.t=Sy_LE  ]; Gc(); goto Sy_LTout;// <=
                case '<' : T.s=Sytype_str[T.t=Sy_lshf]; Gc(); goto Sy_LTout;// <<
                case '?' : T.s=Sytype_str[T.t=Xy_sdcl]; Gc(); goto Sy_LTout;// <?
                case '/' : T.s=Sytype_str[T.t=Xy_eel1]; Gc(); goto Sy_LTout;// </
                case '!' : switch (Peek(2)) {
                             case '-' : switch (Peek(3)) {
                                          case '-' : T.s=Sytype_str[T.t=Xy_scmt];
                                                     Gc(); Gc(); Gc();
                                                     goto Sy_LTout;
                                          default  : ;
                                        } break;
                             case '[' : switch (Peek(3)) {
                                          case 'C' : switch (Peek(4)) {
                                                       case 'D' : switch (Peek(5)) {
                                                                    case 'A' : switch (Peek(6)) {
                                                                                 case 'T' : switch (Peek(7)) {
                                                                                              case 'A' : switch (Peek(8)) {
                                                                                                           case '[' : T.s=Sytype_str[T.t=Xy_scdt];
                                                                                                                      Gc(); Gc(); Gc();Gc(); Gc(); Gc();Gc(); Gc();
                                                                                                                      goto Sy_LTout;
                                                                                                           default  : ;
                                                                                                         } break;
                                                                                              default  : ;
                                                                                            } break;
                                                                                 default  : ;
                                                                               } break;
                                                                    default  : ;
                                                                  } break;
                                                       default  : ;
                                                     } break;
                                          default  : ;
                                        } break;
                             default  : ;
                           } break;
                default  : ;
              }
              T.s=Sytype_str[T.t=Sy_LT  ];
  Sy_LTout:   break;
  case '>'  : switch (Peek()) {
                case '=' : T.s=Sytype_str[T.t=Sy_GE  ]; Gc(); break;  // >=
                case '>' : T.s=Sytype_str[T.t=Sy_rshf]; Gc(); break;  // ==
                default  : T.s=Sytype_str[T.t=Sy_GT  ];       break;  // =
              } break;
  case '?'  : switch (Peek()) {
                case '>' : T.s=Sytype_str[T.t=Xy_edcl]; Gc(); break;  // ?>
                default  : T.s=Sytype_str[T.t=Sy_qst ];       break;  // ?
              } break;
  case ','  : T.s=Sytype_str[T.t=Sy_cmma]; break;
  case '.'  : switch (Peek()) {
                case '.' : T.s=Sytype_str[T.t=Sy_ddot]; Gc(); break;  // ..
                default  : T.s=Sytype_str[T.t=Sy_dot ];       break;  // .
              } break;
  case '/'  : switch (Peek()) {
       // C-style inline comment (//) may be handled as a single lexical token
                case '/'  : Gc();                                     // //
                            if (flags.cflag)
                              while ((Peek()!='\n')&&(Peek()!=EOF))
                                T.s.push_back(Gc());
                            else T.s = "//";
                            T.t=Sy_cmnt;
                            break;
                case '>'  : T.s=Sytype_str[T.t=Xy_eel0]; Gc(); break; // />
                default   : T.s=Sytype_str[T.t=Sy_div];        break; // /
              } break;
  case '|'  : switch (Peek()) {
                case '|'  : T.s=Sytype_str[T.t=Sy_OR  ]; Gc(); break; // ||
                case '/'  : T.s=Sytype_str[T.t=Sy_min ]; Gc(); break; // |/
                case '\\' : T.s=Sytype_str[T.t=Sy_max ]; Gc(); break; // |
                default   : T.s=Sytype_str[T.t=Sy_line];       break;
              } break;
  case '\\' : T.s=Sytype_str[T.t=Sy_back]; break;
  case '\n' : T.s=Sytype_str[T.t=Sy_EOR ]; break;
  case EOF  : T.s=Sytype_str[T.t=Sy_EOF ]; break;
  default   : if (IsF1Digit(c)) {      // Flt pt decimal (FPD) start character
                T.t=Sy_ISTR;           // Assume it's going to form an integer
                T.s.push_back(c);      // Store it
                                       // While the *next* one is a FPD char...
                while (IsF2Digit(Peek())) {
                                       // Store it (but DON'T read it yet)
                  T.s.push_back(Peek());
                                       // Is this a valid FPD number? (Not every
                                       // sequence of valid FPD *characters*
                                       // make a valid FPD *number*: -2.34.5)
                  strtod(T.s.c_str(),&end);
                                       // No, so ditch the last character
                                       // See notes below
                  if ((*end!=0)&&(*end!='e')&&(*end!='E')) {
                    T.s.erase(T.s.end()-1);
                    break;             // And go, 'cos it *was* valid last time
                  }
                                       // Yes, so ditch the character (we've
                                       // already Peeked it) and use the side-
                                       // effect to see if it's FP or integer.
                                       // (The '-' is for things like 1e-2: note
                                       // this will make 1000e-1 FP, but then..?
                  else
                    if ((Gc() == '.')||(last == '-')||(*end=='e')||(*end=='E'))
                      T.t=Sy_FSTR;
                }
                break;                 // End of while(): next char is not FPD
              }
              T.t=Sy_STR;              // Unquoted arbitary alphanumeric
              T.s.push_back(c);
              while (isalnum((unsigned char)(Peek()))||
                      (Peek()=='_')                  ||
                      ((Peek()=='.')&& flags.dflag)  ||
                      ((Peek()=='-')&& flags.mflag) ) T.s.push_back(Gc());
              break;
}

Td = T;
Hst.PutTok(Td);                        // Load the history buffer
count++;                               // Update token counter

// The floating point stuff is a collosal bodge. It was written around 2000
// under Borland, and worked fine. BUT the u$oft VS10 compiler interprets
// floating point numnbers differently to Borland, and strtod() has different
// behaviour under the two compilers: Borland is happy that 1.23e is a FPD,
// u$oft not.
// To be fair, the standard is ambiguous on the matter - it depends how you read
// it. The cool way to solve this problem is to define our own strtod(). There's
// a first crack at a state transition matrix in my notebooks (1/10/2015), but
// then looking at the test data output (notebook) if I just override the
// detection of 'e' as an illegal FPD character - which is what the code above
// does - we're good to go again.
// I can't help but feel I'm just storing up trouble here......
}

//------------------------------------------------------------------------------

void Lex::GetTokNC(tokdat & Td)
// A wrapper for "get tokens" that ditches PCODE comments before they get out.
{
do GetTok(Td); while (T.t==Lex::Sy_cmnt);
}

//------------------------------------------------------------------------------

void Lex::Init()
{
fp = 0;                                // Internal stream pointer
fname.clear();                         // Filename
T.l = 1;                               // Line and column coordinates
T.c = 0;
//############
//next = (char)0;                        // Lookahead character
next.clear();
CHARBUF = 10;
//for(unsigned i=0;i<CHARBUF;i++)next.push_back((char)0);
last = (char)0;                        // Lookback character
err = S_0;                             // No errors (yet)
flags.dflag = true;                    // '.' is an alphanumeric
flags.mflag = false;                   // No '-' in unquoted strings
flags.nflag = true;                    // Interpret strings of digits as numbers
flags.cont = '\\';                     // Continuation character
flags.cflag = true;                    // Treat // as a C comment
count = 0L;                            // Tokens handed out so far
//stringflag = false;                    // Are we reading from file or string?
intype = '0';                          // No input channel assigned
}

//------------------------------------------------------------------------------

bool Lex::IsBDigit(char c)
// Is it a binary digit?
{
switch (c) {
  case '0' : case '1' : return true;
  default  : return false;
}
}

//------------------------------------------------------------------------------

bool Lex::IsDDigit(char c)
// Is it a decinal digit?
{
switch (c) {
  case '0' : case '1' : case '2' : case '3' : case '4' : case '5' : case '6' :
  case '7' : case '8' : case '9' : return true;
  default  : return false;
}
}

//------------------------------------------------------------------------------

bool Lex::IsEOF(tokdat & Td)
{
return (Td.t == Sy_EOF);
}

//------------------------------------------------------------------------------

bool Lex::IsError(tokdat & Td)
// Routine to establish if the token is a lexer/parser error token
{
switch (Td.t) {
  case SE_FNA : return true;
  case SE_SNF : return true;
  case SE_BUF : return true;
  default     : return false;
}

}

//------------------------------------------------------------------------------

bool Lex::IsF1Digit(char c)
// We need to distinguish between an allowable *start* character and the others.
// The guts of the numeric recogniser in the lexer is centered round strtod().
// Yes, I know, write it yourself, and one day, perhaps....
// But anyway, the C standard allows a floating point number to *start* with 'e'
// thus "e-1" is interpreted as "0". Why, Lord? The quickest (dirtyest) way to
// fix this is to make an exception of strings *starting* with 'e' - hence these
// two routines.
{
if (!flags.nflag) return false;
switch (c) {
  case '0' : case '1' : case '2' : case '3' : case '4' : case '5' : case '6' :
  case '7' : case '8' : case '9' : case '.' : case '+' : case '-' : return true;
  default  : return false;
}
}

//------------------------------------------------------------------------------

bool Lex::IsF2Digit(char c)
// Is it a non-initial floating point character?
{
switch (c) {
  case '0' : case '1' : case '2' : case '3' : case '4' : case '5' : case '6' :
  case '7' : case '8' : case '9' : case '.' : case '+' : case '-' : case 'e' :
  case 'E' : return true;
  default  : return false;
}
}

//------------------------------------------------------------------------------

bool Lex::IsHDigit(char c)
// Is it a hexadecimal digit?
{
switch (c) {
  case '0' : case '1' : case '2' : case '3' : case '4' : case '5' : case '6' :
  case '7' : case '8' : case '9' : case 'a' : case 'A' : case 'b' : case 'B' :
  case 'c' : case 'C' : case 'd' : case 'D' : case 'e' : case 'E' : case 'f' :
  case 'F' : return true;
  default  : return false;
}
}

//------------------------------------------------------------------------------

bool Lex::IsODigit(char c)
// Is it an octal digit?
{
switch (c) {
  case '0' : case '1' : case '2' : case '3' : case '4' : case '5' : case '6' :
  case '7' : return true;
  default  : return false;
}
}

//------------------------------------------------------------------------------

bool Lex::IsOp(Lex::Sytype t)
// Reduces the operator types
{
switch (t) {
  case Lex::Sy_amp  :
  case Lex::Sy_AND  :
  case Lex::Sy_AT   :
  case Lex::Sy_cat  :
  case Lex::Sy_cmma :
  case Lex::Sy_div  :
  case Lex::Sy_dol  :
  case Lex::Sy_EQ   :
  case Lex::Sy_exp  :
  case Lex::Sy_GE   :
  case Lex::Sy_GT   :
  case Lex::Sy_hash :
  case Lex::Sy_LE   :
  case Lex::Sy_LT   :
  case Lex::Sy_line :
  case Lex::Sy_lshf :
  case Lex::Sy_max  :
  case Lex::Sy_min  :
  case Lex::Sy_mult :
  case Lex::Sy_NE   :
  case Lex::Sy_OR   :
  case Lex::Sy_pcnt :
  case Lex::Sy_plng :
  case Lex::Sy_plus :
  case Lex::Sy_pnd  :
  case Lex::Sy_qst  :
  case Lex::Sy_rshf :
  case Lex::Sy_squt :
  case Lex::Sy_sub  :
  case Lex::Sy_tlda :
  case Lex::Sy_dcol : return true;
  default           : return false;
}

}

//------------------------------------------------------------------------------

bool Lex::IsSDigit(char c)
// Is it a STL digit?
{
switch (c) {
  case 'U' : case 'u' : case 'X' : case 'x' : case '0' : case '1' : case 'Z' :
  case 'z' : case 'W' : case 'w' : case 'L' : case 'l' : case 'H' : case 'h' :
  case 'D' : case 'd' : return true;
  default  : return false;
}
}

//------------------------------------------------------------------------------

bool Lex::IsStr(Lex::Sytype t)
// Reduces the string types to a single one
{
switch (t) {
  case Lex::Sy_dqut :
  case Lex::Sy_STR  :
  case Lex::Sy_HSTR :
  case Lex::Sy_ISTR :
  case Lex::Sy_SSTR :
  case Lex::Sy_BSTR :
  case Lex::Sy_OSTR :
  case Lex::Sy_FSTR : return true;
  default           : return false;
}

}

//------------------------------------------------------------------------------

bool Lex::IsStrInt(Lex::Sytype t)
// Is the string of integer type?
{
switch (t) {
  case Lex::Sy_HSTR :
  case Lex::Sy_ISTR :
  case Lex::Sy_BSTR :
  case Lex::Sy_OSTR : return true;
  default           : return false;
}

}

//------------------------------------------------------------------------------

string Lex::Join(string s0,string s1)
// To re-unite the string representation of a number with its leading sign
{
if (s0!="-") return s1;                // It isn't a minus sign
return s0+s1;                          // It is a minus sign
}

//------------------------------------------------------------------------------

char Lex::Peek(unsigned p)
{
//Dump();
//WALKLIST(char,next,i) printf("|%c|",*i);
//printf("\np = %u\n",p);
if (p==0) return last;
if (p>=CHARBUF) return char(0);
if (next.empty())return EOF;
list<char>::iterator it = next.begin();
for(unsigned i=0;i<p-1;i++) it++;
//printf("\n*it = %c\n",*it);
return *it;
}

//------------------------------------------------------------------------------
/*
char Lex::Peek()
// Access lookahead character
{
return next;
}
  */
//------------------------------------------------------------------------------

void Lex::push_back(int x)
// Set the pushback flag. This causes the token generator to repeat itself next
// time it's kicked, rather than spit out a new token.
{
//TRACE(2) fprintf(ofp,"Lex::push_back(%d)\n",x);
                                       // History buffer is fixed size
                                       // (yeah, yeah, I know...)
if (abs(x) > Hst.GetSize()) err = SE_BUF;
Hst.push_back(x);                      // Move the 'here' pointer in the buffer
count -= (long)x;                      // Keep track of things
}

//------------------------------------------------------------------------------

void Lex::Reset()
// Routine to reset the internal error flag
{
err = S_0;
}

//------------------------------------------------------------------------------

void Lex::SetCChar(char c)
// Routine to set the special character that makes the lexer skip the rest of
// the line plus the next '\n'. In other words, the continuation character.
{
flags.cont = c;
}

//------------------------------------------------------------------------------

void Lex::SetCFlag(bool c)
// Toggles the behaviour when confronted by '//'.
// TRUE  : Treat as a C comment (token type T.t = Sy_cmnt), and pull in the rest
// of the line up to EOR/EOF and plonk it in T.s
// FALSE : Treat it as a C comment token (T.t = Sy_cmnt still), but put '//'
// into T.s and leave the rest of the line for the next level up
{
flags.cflag = c;
}

//------------------------------------------------------------------------------

void Lex::SetCtr(long l)
{
count = l;
}

//------------------------------------------------------------------------------

void Lex::SetDFlag(bool f)
// Routine to write dflag. This changes the lexer behaviour:
// TRUE  : '.' is an alphanumeric (so ABC.XYZ is a single lexical token)
// FALSE : it's a separate token (so ABC.XYZ is ABC . XYZ)
{
flags.dflag = f;
}

//------------------------------------------------------------------------------

void Lex::SetFile()
// Disconnect the input stream. I can't just call one of the other SetFile()
// routines with a null argument, 'cos the compiler can't disambiguate them...
{
//if (fp!=0) fclose(fp);
if (intype=='F') fclose(fp);
Init();                                // This will reset intype
}

//------------------------------------------------------------------------------

void Lex::SetFile(FILE * ffp)
// Kill any existing stream association with the lexer and attach this one.
// fp == 0 is acceptable.
{
SetFile();
fp = ffp;
intype = '*';
}

//------------------------------------------------------------------------------

void Lex::SetFile(char * str)
// Kill any existing file association with the lexer and attach this file.
// The empty string is an acceptable way of disconnecting everything.
{
SetFile();
fname = string(str);
if(fname.empty()){
  fp = 0;
  err = SE_FNA;
}
else {
  if (file_exists(str)&&file_readable(str)) fp = fopen(fname.c_str(),"r");
  else fp = 0;
  err = fp ? S_0 : SE_FNA;
  intype = 'F';
}

}

//------------------------------------------------------------------------------

void Lex::SetFile(string str)
// Kill any existing file association with the lexer and attach this string.
// The empty string is a valid input in this context.
{
SetFile();
fname = str;
//stringflag = true;
intype = 'S';
}

//------------------------------------------------------------------------------

void Lex::SetMFlag(bool f)
// Routine to write mflag. This is a bodge to change the lexer behaviour: with
// the flag false, it behaves as any sane lexer would. When set to true, it
// treats '-' as an alphabetic character, not an operator. This is so that we
// can parse ICODE, which allows this kind of thing in PROGRAM blocks.
{
flags.mflag = f;
}

//------------------------------------------------------------------------------

void Lex::SetNFlag(bool f)
// Routine to write nflag. This changes the behaviour of the lexer when
// confronted with numbers.
// With .nflag = TRUE,
// 1.0 will be recognised as
//  symbol token 1:     floating point number one-point-zero
// 1.0.0.9 will be recognised as
//  symbol token 1:     floating point number one-point-zero
//  symbol token 2:     character '.'
//  symbol token 3:     floating point number zero-point-nine
// With .nflag = FALSE,
// 1.0 will be recognised as
//  symbol token 1:     string "1.0"
// 1.0.0.9 will be recognised as
//  symbol token 1:     string "1.0.0.9"
{
flags.nflag = f;
}

//------------------------------------------------------------------------------

string Lex::SkipTo(Lex::Sytype sy)
// Routine to pull in tokens up to and including the first instance of 'sy'.
// It's a proper version of SkipTo(char).
// The history buffer still works as it should.
// The idea is that the token stream is drained until the termination token is
// found, whereupon control is handed back to whatever parser called this.
// As a by-product, the skipped tokens are reassembled into the skipped string
// and returned in case the string itself needs to be passed to yet another
// sub-parser. This is fiddlier than it sounds, because we can't just copy the
// characters themselves into the buffer, because we can't see them (nor should
// we be able to, because yer average token may consist of an arbitrary number
// of characters). So we re-assemble the skipped string by assembling the string
// equivalent of the token stream. Fine, but now we don't know what the token
// separator character was, and at this point I kind of lost the will to live
// and poked ' ' in as and when it seemed reasonable.
// Could do better in another life.
{
string buf;
string s0 = string(" ");
for (;;) {                             // Loop until termination token
  tokdat Td;
  GetTok(Td);                          // Next token
//  if (Td.s != "\\n") {
//    if (Td.t==Sy_dqut) buf += '"' + Td.s + '"';
//    else buf += ((IsStr(Td.t)?s0:string())+Td.s);
//  }
//  else buf += '\n';

  switch (Td.t) {
                   // EOR must be inserted explicitly
    case Sy_EOR  : buf += '\n';                                       break;
                   // Double quoted strings need the quotes re-introduced
    case Sy_dqut : buf += '"' + Td.s + '"';                           break;
                   // Comments need to be reconstructed
    case Sy_cmnt : //buf += Sytype_str[Sy_cmnt] + Td.s;                 break;
                   buf += Td.s;                                       break;
                   // If it's a string type, the delimiter *might* have been a
                   // ' ', so poke one in
    default      : buf += ((IsStr(Td.t)?s0:string())+Td.s);           break;
}
  if (Td.t==sy) return buf;            // Legitimate exit
  if (IsError(Td)) return buf;         // Error exit
}

}

//------------------------------------------------------------------------------

string Lex::SkipTo(char ce)
// Routine to pull in characters up to and including 'ce' (or EOF), and poke
// them into a string, which is returned.
// NOTE THIS SHORT-CIRCUITS THE HISTORY BUFFER, WHICH WORKS AT THE TOKEN LEVEL
{
string buf;
char c;
while (((c=Gc())!=ce)&&(c!=EOF)) buf += c;
return buf;
}

//------------------------------------------------------------------------------

unsigned Lex::Str2Uint(string s)
// Takes a generic UIF integer string (i.e. ISTR, STR, OST, BSTR or HSTR) and
// turns it into an unsigned integer.
// (VHDL STD_LOGIC gets turned into 0, and serve it right.)
// If the conversion can't be done - for example "$123k", the routine just
// returns 0.
{
unsigned ans = 0;
if (s[0]=='$') {
  for (unsigned int i=1;i<s.size();i++) {
    unsigned int j = 0;
//printf("s[%d-1] = [%c]\n",i,s[i-1]);
    switch (s[i]) {
      case '0' : j = 0; break;
      case '1' : j = 1; break;
      case '2' : j = 2; break;
      case '3' : j = 3; break;
      case '4' : j = 4; break;
      case '5' : j = 5; break;
      case '6' : j = 6; break;
      case '7' : j = 7; break;
      case '8' : j = 8; break;
      case '9' : j = 9; break;
      case 'a' : case 'A' : j = 10; break;
      case 'b' : case 'B' : j = 11; break;
      case 'c' : case 'C' : j = 12; break;
      case 'd' : case 'D' : j = 13; break;
      case 'e' : case 'E' : j = 14; break;
      case 'f' : case 'F' : j = 15; break;
      default  : return 0;
    }
    ans = ans << 4;
    ans = ans + j;
  }
  return ans;
}

if (s[0]=='&') {
  for (unsigned int i=1;i<s.size();i++) {
    unsigned int j = 0;
    switch (s[i]) {
      case '0' : j = 0; break;
      case '1' : j = 1; break;
      case '2' : j = 2; break;
      case '3' : j = 3; break;
      case '4' : j = 4; break;
      case '5' : j = 5; break;
      case '6' : j = 6; break;
      case '7' : j = 7; break;
      default  : return 0;
    }
    ans = ans << 3;
    ans = ans + j;
  }
  return ans;
}

if (s[0]=='%') {
  for (unsigned int i=1;i<s.size();i++) {
    unsigned int j = 0;
    switch (s[i]) {
      case '0' : j = 0; break;
      case '1' : j = 1; break;
      default  : return 0;
    }
    ans = ans << 1;
    ans = ans + j;
  }
  return ans;
}

int n = sscanf(s.c_str(),"%u",&ans);
return n==1 ? ans : 0;
}

//==============================================================================

const int Lex::History::HS = 256;

//==============================================================================

void Lex::History::Dump(vector<string> & vstr,int depth)
// Dump the history list to a string vector
// This version gets the pretty-print, so it can be given to the monkey
{
depth = abs(depth);
depth = min(depth,HS);
vstr.clear();
vstr.push_back(string("(Line,Col)\n"));
string s;
for(int i=1;i!=depth+1;i++) {
  int j = nfc-i>=0 ? nfc-i : nfc-i+HS;
  s.clear();
  dprintf(s,"(%4d,%3d) %s",td[j].l,td[j].c,td[j].s.c_str());
  if(i==1)dprintf(s,"      <- Token (%s) out of sequence",Sytype_dbg[td[j].t]);
  dprintf(s,"\n");
  vstr.push_back(s);
}
}

//------------------------------------------------------------------------------

void Lex::History::Dump(FILE * fp,int depth)
// Dump the history list
// This version gets the full deal, including the internals
{
depth = abs(depth);
depth = min(depth,HS);
fprintf(fp,"\n(Line,Col)\n");
for(int i=1;i!=depth+1;i++) {
  int j = nfc-i>=0 ? nfc-i : nfc-i+HS;
  //td[j].Dump(fp);
  fprintf(fp,"(%4d,%3d) %s",td[j].l,td[j].c,td[j].s.c_str());
  if(i==1)fprintf(fp,"      <- Token (%s) out of sequence",Sytype_dbg[td[j].t]);
  fprintf(fp,"\n");
}
fprintf(fp,"Next free token (nfc)   = %d\n",nfc);
fprintf(fp,"Pushback offset (pflag) = %d\n",pflag);
}

//------------------------------------------------------------------------------

long Lex::History::GetSize()
// Return the length of the internal ring buffer
{
return (long)HS;
}

//------------------------------------------------------------------------------

bool Lex::History::GetTok(tokdat & Td)
// Routine to hand a token out from the history substructure, if appropriate
{
if (pflag==0) return false;            // None to be had

int a = nfc-pflag;
do a+=HS; while (a<HS);                // Avoid underrun at all costs....
int b = a % HS;
//int b = (a+HS) % HS;
Td = td[b];
pflag--;

//Td = td[(nfc-(pflag--))%HS];           // Find it, hand it over, decrement ptr
return true;
}

//------------------------------------------------------------------------------

void Lex::History::push_back(int x)
// Move the 'token wanted' offset counter
{
pflag += x;
// pflag %= HS; I don't think I need to modulo this?
}

//------------------------------------------------------------------------------

void Lex::History::PutTok(tokdat Td)
// Shove the token onto the history list
{
td[nfc++] = Td;
nfc %= HS;
}

//------------------------------------------------------------------------------

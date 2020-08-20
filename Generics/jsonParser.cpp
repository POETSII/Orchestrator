//---------------------------------------------------------------------------
/* A brutally simple streaming XML parser. You instantiate the class, and then
the way in is via Parse(FILE * fp). *It is assumed* that the XML is in a file
that has been opened and attached to fp.
The class is intended to be derived off (is that English?). The streaming
interface gives you routines that are called when various srtuctures are
encountered. The first argument to each of them is a void *, which can be used
to pass out context-dependent information at will. (It's a way in to burgle
any datastructures you choose to reveal.)
As with any parser, you need the syntax state transition diagrams in front of
you if it is to make any sense - see documentation (22/12/17).
In overview, the way in is via Parse(); this calls Attr() and Element() (which
may itself call Attr() and Json()) as necessary).
The interface base class functions may be trivially replaced by callbacks if you
prefer.

*/
//==============================================================================

#include "jsonParser.h"
#include "macros.h"

//==============================================================================

jsonParser::jsonParser()
{
root        = new node();
root->jtype = '0';
par         = root;
}

//------------------------------------------------------------------------------

jsonParser::~jsonParser()
//
{
delete root;
}

//------------------------------------------------------------------------------

jsonParser::node * jsonParser::AddNode()
{
node * n = new node();
par->node_v.push_back(n); 
n->par = par;
return n;
}

//------------------------------------------------------------------------------
       /*
bool xmlParser::CDATA(const void * p,const unsigned & type,const string & s)
{
printf("\n==============================================\n");
printf("xmlParser::CDATA() BASE CLASS INSTANCE\n");
printf("Data structure tag : 0x%#08p\n",p);
printf("CDATA type         : %u\n",type);
printf("CDATA string       : %s\n",s.c_str());
printf("\n==============================================\n");
return true;
}

//------------------------------------------------------------------------------

bool xmlParser::Comments(const void * p,const string & s)
// Comment string encountered.
// s : The entire comment string
{
printf("\n==============================================\n");
printf("xmlParser::Comments() BASE CLASS INSTANCE\n");
printf("Data structure tag     : 0x%#08p\n",p);
printf("comment string         : %s\n",s.c_str());
printf("\n==============================================\n");
return true;
}

//------------------------------------------------------------------------------

bool xmlParser::EndDocument(const void * p)
// End-Of-Document found; processing halts.
{
printf("\n==============================================\n");
printf("xmlParser::EndDocument() BASE CLASS INSTANCE\n");
printf("Data structure tag     : 0x%#08p\n",p);
printf("\n==============================================\n");
return true;
}

//------------------------------------------------------------------------------

bool xmlParser::EndElement(const void * p,const string & s)
// End-Of-Element encountered.
// s : Element name, *whether or not* the closure was anonymous
// i.e.(.../> or </stuff>)

{
printf("\n==============================================\n");
printf("xmlParser::EndElement() BASE CLASS INSTANCE\n");
printf("Data structure tag     : 0x%#08p\n",p);
printf("Element name           : %s\n",s.c_str());
printf("\n==============================================\n");
return true;
}
     */
//------------------------------------------------------------------------------

bool jsonParser::Error(const void * p,const unsigned & r,const unsigned & c,
                      const string & s)
// Syntax error in the XML detected.
// r,c : Row and column in the source file of the last character of the token
// that triggered the error
// s : The last lexical token encountered. (The error *may* lie before this in
// the source.)
{
printf("\n==============================================\n");
printf("jsonParser::Error() BASE CLASS INSTANCE\n");
printf("Data structure tag     : 0x%#08p\n",p);
printf("Row, col               : %u,%u\n",r,c);
printf("(Last) symbol token    : %s\n",s.c_str());
printf("\n==============================================\n");
err_v.push_back(err_t(r,c,s));
return true;
}

//------------------------------------------------------------------------------

void jsonParser::ExposeErrs()
// Tell the monkey how many times it went wrong
{
printf("%u parse errors found\n",err_v.size());
WALKVECTOR(err_t,err_v,i)
  printf("(%u,%u) : %s\n",(*i).row,(*i).col,(*i).sym.c_str());
}

//------------------------------------------------------------------------------
       /*
bool xmlParser::JSON(const void * p,const string & s,
                     const vector<pair<string,string> > & vps)
// s : The JSON string itself
// vps: A vector of <name,value> pairs.
{
printf("\n==============================================\n");
printf("xmlParser::JSON() BASE CLASS INSTANCE\n");
printf("Data structure tag     : 0x%#08p\n",p);
printf("Origonal string        : %s\n",s.c_str());
printf("JSON elements\n");
if (vps.empty()) printf("   .. there are none\n");
for(unsigned i=0;i<vps.size();i++)
  printf("%s : %s\n",vps[i].first.c_str(),vps[i].second.c_str());
printf("\n==============================================\n");
return true;
}

//------------------------------------------------------------------------------

bool xmlParser::SetPtr(const void * p)
{
phome = const_cast<void *>(p);
return true;
}

//------------------------------------------------------------------------------

bool xmlParser::StartDocument(const void * p,const string & s,
                              const vector<pair<string,string> > & vps)
// Start of document
// s : Document name
// vps : Vector of <name,value> attributes
{
printf("\n==============================================\n");
printf("xmlParser::StartDocument() BASE CLASS INSTANCE\n");
printf("Data structure tag     : 0x%#08p\n",p);
printf("Document name          : %s\n",s.c_str());
printf("Attributes\n");
if (vps.empty()) printf("   .. there are none\n");
for(unsigned i=0;i<vps.size();i++)
  printf("%s = %s\n",vps[i].first.c_str(),vps[i].second.c_str());
printf("\n==============================================\n");
return true;
}

//------------------------------------------------------------------------------

bool xmlParser::StartElement(const void * p,const string & s,
                             const vector<pair<string,string> > & vps)
// The start of an element has been encountered. The is no indication here
// whether or not sub-elements exist.
// s : Element name
// vps : Vector of attribute <name,value> pairs
{
printf("\n==============================================\n");
printf("xmlParser::StartElement() BASE CLASS INSTANCE\n");
printf("Data structure tag     : 0x%#08p\n",p);
printf("Element name           : %s\n",s.c_str());
printf("Attributes\n");
if (vps.empty()) printf("   .. there are none\n");
for(unsigned i=0;i<vps.size();i++)
  printf("%s = %s\n",vps[i].first.c_str(),vps[i].second.c_str());
printf("\n==============================================\n");
return true;
}

//==============================================================================

void xmlParser::Attr()
{
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4,t5} toktyp;
struct duple {int ns,ac;} next;
duple table[5][t5+1] =
// Incident symbol                             // Next
//    0      1      2      3      4      5     // state
{{{1, 1},{E, 0},{E, 0},{0, 2},{E, 0},{E, 0}},  // 0
 {{E, 0},{2, 0},{E, 0},{1, 2},{R, 3},{R, 3}},  // 1
 {{R, 4},{E, 0},{R, 4},{2, 2},{E, 0},{3, 0}},  // 2
 {{4, 4},{E, 0},{4, 4},{3, 2},{E, 0},{R, 0}},  // 3
 {{E, 0},{E, 0},{E, 0},{4, 2},{E, 0},{R, 0}}}; // 4

for(int state=0;;) {
  Lx.GetTok(Td);                       // Get the next token...
  if (Td.t==Lex::Sy_EOR) continue;     // Chuck away any newlines
  if (Lx.IsError(Td)) problem = true;
  switch (Td.t) {                      // Map to array index
    case Lex::Sy_STR  : toktyp = t0;  break;
    case Lex::Sy_AS   : toktyp = t1;  break;
    case Lex::Sy_dqut : toktyp = t2;  break;
    case Lex::Xy_scmt : toktyp = t3;  break;
    case Lex::Sy_squt : toktyp = t5;  break;
    default           : toktyp = t4;  break;
  }
//  if (Lex::IsStr(Td.t))   toktyp = t0; // Reduction functions
                                       // Override the reduction functions
  next = table[state][toktyp];         // Make the transition
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  : break;
    case 1  : Lx.push_back();
              attr.push_back(pair<string,string>(Calpha().c_str(),string()));
              break;
    case 2  : comments = Lx.SkipTo(Lx.Xy_ecmt);
              Comments(phome,comments);
              comments.clear();
              break;
    case 3  : Lx.push_back();
              break;
    case 4  : attr.back().second = Td.s;
              break;
    default : break;
  }
  switch (state=next.ns) {
    case X :  goto out;                // Exit
    case R :  goto out;                // Return
    case E :  problem = true;          // Error
              break;
  }
  if (problem==true) {                 // May be set elsewhere
    Error(phome,Td.l,Td.c,Td.s);
    goto out;
  }
}
out: return;
}

//------------------------------------------------------------------------------
         
xmlParser::calpha_t xmlParser::Calpha()
{
                                       // And away we go...
enum loctok  {t0=0,t1,t2} toktyp;
struct duple {int ns,ac;} next;
duple table[5][t2+1] =
// Incident symbol        // Next
//    0      1      2     // state
{{{1, 1},{2, 0},{R, 2}},  // 0
 {{R, 2},{2, 0},{R, 2}},  // 1
 {{R, 3},{R, 2},{R, 2}}}; // 2

string strR,strS;
for(int state=0;;) {
  Lx.GetTok(Td);                       // Get the next token...
  if (Td.t==Lex::Sy_EOR) continue;     // Chuck away any newlines
  if (Lx.IsError(Td)) problem = true;
  switch (Td.t) {                      // Map to array index
    case Lex::Sy_STR  : toktyp = t0;  break;
    case Lex::Sy_col  : toktyp = t1;  break;
    default           : toktyp = t2;  break;
  }
//  if (Lex::IsStr(Td.t))   toktyp = t0; // Reduction functions
                                       // Override the reduction functions
  next = table[state][toktyp];         // Make the transition
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  :                   break;
    case 1  : strR = Td.s;      break;
    case 2  : Lx.push_back();   break;
    case 3  : strS = Td.s;      break;
    default :                   break;
  }
  switch (state=next.ns) {
    case X :  goto out;                // Exit
    case R :  goto out;                // Return
    case E :  problem = true;          // Error
              break;
  }
  if (problem==true) {                 // May be set elsewhere
    Error(phome,Td.l,Td.c,Td.s);
    goto out;
  }
}
out: return calpha_t(strR,strS);
}
         */
//------------------------------------------------------------------------------

void jsonParser::Dump(FILE * fp)
{
fprintf(fp,"jsonParser+++++++++++++++++++++++++++++++++++++++++\n");
root->Dump(fp);
/*
fprintf(fp,"Element name stack\n");
if (elename.empty()) fprintf(fp,"   .. is empty\n");
WALKVECTOR(calpha_t,elename,i) fprintf(fp,"   .. %s\n",(*i).c_str());
fprintf(fp,"Attributes\n");
if (attr.empty()) fprintf(fp,"   .. there are none\n");
for(unsigned i=0;i<attr.size();i++)
  fprintf(fp,"   .. %s = %s\n",attr[i].first.c_str(),attr[i].second.c_str());
fprintf(fp,"JSON\n");
if (json.empty()) fprintf(fp,"   .. there are none\n");
for(unsigned i=0;i<json.size();i++)
  fprintf(fp,"   ..%s : %s\n",json[i].first.c_str(),json[i].second.c_str());
fprintf(fp,"JSON string : %s\n",jsonstr.c_str());
fprintf(fp,"xmlname     : %s\n",xmlname.c_str());
fprintf(fp,"cdata       : %s\n",cdata.c_str());
fprintf(fp,"comments    : %s\n",comments.c_str());
fprintf(fp,"problem     : %s\n",problem ? "TRUE" : "FALSE");
fprintf(fp,"phome       : 0x%#08p\n",phome);
*/
fprintf(fp,"jsonParser-----------------------------------------\n");
}

//------------------------------------------------------------------------------
           /*
void xmlParser::Element()
{
//printf("\nIN xmlParser::Element()\n\n");
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4,t5,t6,t7,t8} toktyp;
struct duple {int ns,ac;} next;

duple table[9][t8+1] =
// Incident symbol                                                  // Next
//    0      1      2      3      4      5      6      7      8     // state
{{{1, 0},{E, 0},{E, 0},{E, 0},{E, 0},{E, 0},{0, 1},{E, 0},{E, 0}},  // 0
 {{E, 0},{2, 2},{E, 0},{E, 0},{E, 0},{E, 0},{1, 1},{E, 0},{E, 0}},  // 1
 {{E, 0},{3, 4},{4, 3},{R, 5},{E, 0},{E, 0},{2, 1},{E, 0},{E, 0}},  // 2
 {{E, 0},{3, 4},{4, 3},{R, 5},{E, 0},{E, 0},{3, 1},{E, 0},{E, 0}},  // 3
 {{5, 7},{6, 8},{E, 0},{E, 0},{4, 6},{7, 0},{4, 1},{E, 0},{E, 0}},  // 4
 {{5, 7},{E, 0},{E, 0},{E, 0},{5, 9},{7, 0},{5, 1},{X,11},{E, 0}},  // 5
 {{E, 0},{E, 0},{E, 0},{E, 0},{E, 0},{7, 0},{6, 1},{E, 0},{E, 0}},  // 6
 {{E, 0},{8,12},{E, 0},{E, 0},{E, 0},{E, 0},{7, 1},{E, 0},{E, 0}},  // 7
 {{E, 0},{E, 0},{R, 0},{E, 0},{E, 0},{E, 0},{8, 1},{E, 0},{E, 0}}}; // 8

string exitroot;
for(int state=0;;) {
  Lx.GetTok(Td);                       // Get the next token...
  if (Td.t==Lex::Sy_EOR) continue;     // Chuck away any newlines
  if (Lx.IsError(Td)) problem = true;
  switch (Td.t) {                      // Map to array index
    case Lex::Sy_LT   : toktyp = t0;  break;
    case Lex::Sy_dqut : toktyp = t1;  break;
    case Lex::Sy_STR  : toktyp = t1;  break;
    case Lex::Sy_GT   : toktyp = t2;  break;
    case Lex::Xy_eel0 : toktyp = t3;  break;
    case Lex::Xy_scdt : toktyp = t4;  break;
    case Lex::Xy_eel1 : toktyp = t5;  break;
    case Lex::Xy_scmt : toktyp = t6;  break;
    case Lex::Sy_EOF  : toktyp = t7;  break;
//    case Lex::Sy_col  : toktyp = t9;  break;
    default           : toktyp = t8;  break;
  }
//  if (Lex::IsStr(Td.t))   toktyp = t0; // Reduction functions
                                       // Override the reduction functions
  next = table[state][toktyp];         // Make the transition
//printf("next,action = %d,%d\n",next.ns,next.ac);
  switch (next.ac) {                   // Post-transition (exit) actions
    case  0 : break;
    case  1 : comments = Lx.SkipTo(Lex::Xy_ecmt);
              Comments(phome,comments);
              comments.clear();
              break;
//    case  2 : elename.push_back(Td.s);
    case  2 : Lx.push_back();
              elename.push_back(Calpha());
              break;
    case  3 : problem = !StartElement(phome,elename.back().c_str(),attr);
              attr.clear();
              break;
    case  4 : Lx.push_back();
              Attr();
              break;
    case  5 : problem = !StartElement(phome,elename.back().c_str(),attr);
              if (problem) break;
              attr.clear();
              problem = !EndElement(phome,elename.back().c_str());
              elename.pop_back();
              break;
    case  6 : cdata = Lx.SkipTo(Lex::Xy_ecdt);
              cdata.erase(cdata.size()-3,cdata.size());   // Remove the "]]>"
              problem = !CDATA(phome,unsigned(1),cdata);
              cdata.clear();
              break;
    case  7 : Lx.push_back();
              Element();
              break;
    case  8 :                          // Rebuild the JSON string
              jsonstr = '"' + Td.s + '"' + Lx.SkipTo(Lex::Xy_eel1);
                                       // Chop Xy_eel1 off the end
              jsonstr.erase(jsonstr.rfind(Lex::Sytype_str[Lex::Xy_eel1]));
              Lx.push_back();          // SkipTo ate Xy_eel1: so shove it back
              { int l_ = Td.l;         // Get the line and column offsets right
                int c_ = 0;
                problem = Json(c_);
                Td.l = l_;
                Td.c += c_;
              }
              problem = !JSON(phome,jsonstr,json);
              jsonstr.clear();
              json.clear();
              break;
    case  9 : cdata = Lx.SkipTo(Lex::Xy_ecdt);
              problem = !CDATA(phome,unsigned(2),cdata);
              cdata.clear();
              break;
    case 10 : Lx.push_back();
              break;
    case 11 : problem = !EndElement(phome,elename.back().c_str());
              if (problem) break;
              problem = !EndDocument(phome);
              break;
    case 12 : Lx.push_back();
              if (elename.back()!=Calpha()) problem=true;
              problem = !EndElement(phome,elename.back().c_str());
              elename.pop_back();
              break;

    default : break;
  }
  switch (state=next.ns) {
    case X :  goto out;                // Exit
    case R :  goto out;                // Return
    case E :  problem = true;          // Error
              break;
  }
  if (problem==true) {                 // May be set elsewhere
    Error(phome,Td.l,Td.c,Td.s);
    problem = false;                   // Reported it; no point doing it again
    goto out;
  }
}
out:
//printf("\nOUT xmlParser::Element()\n\n");
return;
}

//------------------------------------------------------------------------------

bool xmlParser::Json(int & rc)
// A sort of optional sub-parse......
// This is called to parse a string that may or may not be JSON.
// It has its own, local lexer (Lx2) which operates on the class-global string
// "jsonstr". IF the string can be corectly parsed as JSON, the "json" class-
// global structure is loaded.
// THIS IS A NAIVE STRUCTURE THAT ASSUMES ALL JASON is of the form
// "xxx" : 999
// It won't handle anything else; groups and hierarchy are not supported.
// We DO NOT call Error from here, because the local lexer coordinates refer
// to the string jsonstring, not the parent file. If an error is detected,
// the function returns TRUE. If everything is OK, FALSE.
{
//printf("\nIN xmlParser::Json()\n\n");
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4,t5} toktyp;
struct duple {int ns,ac;} next;
duple table[4][t5+1] =
// Incident symbol                             // Next
//    0      1      2      3      4      5     // state
{{{1, 1},{E, 0},{1, 1},{E, 0},{1, 2},{E, 0}},  // 0
 {{E, 0},{2, 0},{E, 0},{E, 0},{2, 2},{E, 0}},  // 1
 {{E, 0},{E, 0},{3, 3},{E, 0},{3, 2},{E, 0}},  // 2
 {{E, 0},{E, 0},{E, 0},{0, 0},{E, 0},{R, 4}}}; // 3

Lex Lx2;                               // Need a local lexer
Lex::tokdat Td2;
Lx2.SetFile(jsonstr);                  // Point the lexer at the JSON string
for(int state=0;;) {
  Lx2.GetTok(Td2);                     // Get the next token...
  rc = Td2.c;
//Td.Dump();
//Dump();
  if (Td2.t==Lex::Sy_EOR) continue;    // Chuck away any newlines
  if (Lx2.IsError(Td2)) problem = true;
  switch (Td2.t) {                     // Map to array index
    case Lex::Sy_dqut : toktyp = t0;  break;
    case Lex::Sy_col  : toktyp = t1;  break;
    case Lex::Sy_cmma : toktyp = t3;  break;
    case Lex::Xy_scmt : toktyp = t4;  break;
    default           : toktyp = t5;  break;
  }
  if (Lex::IsStr(Td2.t))   toktyp = t2;// Reduction functions
                                       // Override the reduction functions
  next = table[state][toktyp];         // Make the transition
//printf("next,action = %d,%d\n",next.ns,next.ac);
  switch (next.ac) {                   // Post-transition (exit) actions
    case  0 : break;
    case  1 : json.push_back(pair<string,string>(Td2.s,string()));
              break;
    case  2 : comments = Lx2.SkipTo(Lex::Xy_ecmt);
              Comments(phome,comments);
              comments.clear();
              break;
    case  3 : json.back().second = Td2.s;
              break;
    case  4 : Lx2.push_back();
              break;
    default : break;
  }
  switch (state=next.ns) {
    case X :  return false;            // Exit
    case R :  return false;            // Return
    case E :  problem = true;          // Error
              goto out;
  }
 if (problem==true) {                  // May be set elsewhere
//    Error(phome,Td2.l,Td2.c,Td2.s); // DON'T call error here 'cos column is wrong
    goto out;
  }
}
out :
json.clear();                          // Whatever, it's broken.
//printf("\nOUT xmlParser::Json()\n - error exit: \n\n");
return true;
}
      */
//------------------------------------------------------------------------------

void jsonParser::Parse(FILE * fp)
//
{
phome = 0;                             // External data structure tag
Lx.SetFile(fp);                        // Point the lexer at it
//Lx.SetNFlag(false);               // Tune lexer to *not* recognise numbers
Value();

}

//------------------------------------------------------------------------------

void jsonParser::Value()
{
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10} toktyp;
struct duple {int ns,ac;} next;
static duple table[7][t10+1] =
// Incident symbol                                                                // Next
//    0      1      2      3      4      5      6      7      8      9     10     // state
{{{1, 8},{5, 4},{4, 3},{4, 2},{4, 1},{E, 0},{E, 0},{E, 0},{E, 0},{X, 0},{E, 0}},  // 0
 {{E, 0},{E, 0},{2, 9},{E, 0},{E, 0},{E, 0},{E, 0},{E, 0},{4, 5},{E, 0},{E, 0}},  // 1
 {{E, 0},{E, 0},{E, 0},{E, 0},{E, 0},{3, 7},{E, 0},{E, 0},{E, 0},{E, 0},{E, 0}},  // 2
 {{E, 0},{E, 0},{E, 0},{E, 0},{E, 0},{E, 0},{E, 0},{E, 0},{4, 5},{E, 0},{1, 0}},  // 3
 {{R,10},{R,10},{R,10},{R,10},{R,10},{R,10},{R,10},{R,10},{R,10},{X, 0},{R,10}},  // 4
 {{6, 6},{6, 6},{6, 6},{6, 6},{6, 6},{6, 6},{6, 6},{4, 5},{6, 6},{E, 0},{6, 6}},  // 5
 {{E, 0},{E, 0},{E, 0},{E, 0},{E, 0},{E, 0},{E, 0},{4, 5},{E, 0},{E, 0},{6, 7}}}; // 6

for(int state=0;;) {
  Lx.GetTok(Td);                       // Get the next token...
  if (Td.t==Lex::Sy_EOR) continue;     // Chuck away any newlines
  if (Lx.IsError(Td)) problem = true;
  switch (Td.t) {                      // ...and map to array index
    case Lex::Sy_lbrc : toktyp = t0;  break;
    case Lex::Sy_lsqb : toktyp = t1;  break;
    case Lex::Sy_FSTR : toktyp = t3;  break;
    case Lex::Sy_col  : toktyp = t5;  break;
    case Lex::Sy_rsqb : toktyp = t7;  break;
    case Lex::Sy_rbrc : toktyp = t8;  break;
    case Lex::Sy_EOF  : toktyp = t9;  break;
    case Lex::Sy_cmma : toktyp = t10; break;
    default           : toktyp = t6;  break;
  }
  if (Lex::IsStr(Td.t))    toktyp = t4;// Reduction functions
  if (Lex::IsStrInt(Td.t)) toktyp = t3;
  if (Td.t==Lex::Sy_dqut) toktyp  = t2;// Override reduction function
  next = table[state][toktyp];         // Make the transition
  node * n;
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  : break;
    case 1  : n = AddNode();
              n->jtype = 'L';
              n->jdata = Td.s;
              break;
    case 2  : n = AddNode();
              n->jtype = 'N';
              n->jdata = Td.s;
              break;
    case 3  : n = AddNode();
              n->jtype = 'S';
              n->jdata = Td.s;
              break;
    case 4  : n = AddNode();
              n->jtype = 'A';
              par = n;
              break;
    case 5  : par = par->par;
              break;
    case 6  : Lx.push_back();
              Value();
              break;
    case 7  : Value();
              break;
    case 8  : n = AddNode();
              n->jtype = 'T';
              par = n;
              break;
    case 9  : par->jkey = Td.s;
              break;
    case 10 : Lx.push_back();
              break;
    default : break;
  }
  switch (state=next.ns) {
    case X :  goto out;                // Exit
    case R :  goto out;                // Return
    case E :  problem = true;          // Error
              break;
  }
 if (problem==true) {                  // May be set elsewhere
    Error(phome,Td.l,Td.c,Td.s);
    goto out;
  }
}
out: return;
}

//------------------------------------------------------------------------------
/*
jsonParser::Value()
{
}
  */
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void jsonParser::node::Dump(FILE * fp,int off)
{
string B(off,' ');                     // Create leading space indent
                                       // Print this node
fprintf(fp,"%skey %s type %c data %s par 0X%08p this 0X%08p\n",
      B.c_str(),jkey.c_str(),jtype,jdata.c_str(),par,this);
                                       // Pretty-print the children
WALKVECTOR(node *,node_v,i)(*i)->Dump(fp,off+2);
}

//------------------------------------------------------------------------------

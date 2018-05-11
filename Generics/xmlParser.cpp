//---------------------------------------------------------------------------

#include "xmlParser.h"
#include "macros.h"

//==============================================================================

xmlParser::xmlParser(FILE * fp)
//
{
phome = 0;                             // External data structure tag
Lx.SetFile(fp);                        // Point the lexer at it
Lx.SetNFlag(false);                    // Tune lexer to *not* recognise numbers
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4,t5} toktyp;
struct duple {int ns,ac;} next;
duple table[5][t5+1] =
// Incident symbol                             // Next
//    0      1      2      3      4      5     // state
{{{X, 1},{1, 0},{E, 0},{E, 0},{0, 2},{E, 0}},  // 0
 {{E, 0},{E, 0},{2, 3},{E, 0},{1, 2},{E, 0}},  // 1
 {{E, 0},{E, 0},{3, 4},{4, 0},{2, 2},{E, 0}},  // 2
 {{E, 0},{E, 0},{3, 4},{4, 0},{3, 2},{E, 0}},  // 3
 {{X, 1},{E, 0},{E, 0},{E, 0},{4, 2},{E, 0}}}; // 4

for(int state=0;;) {
  Lx.GetTok(Td);                       // Get the next token...
  if (Td.t==Lex::Sy_EOR) continue;     // Chuck away any newlines
  if (Lx.IsError(Td)) problem = true;
  switch (Td.t) {                      // Map to array index
    case Lex::Sy_LT   : toktyp = t0;  break;
    case Lex::Xy_sdcl : toktyp = t1;  break;
    case Lex::Xy_edcl : toktyp = t3;  break;
    case Lex::Xy_scmt : toktyp = t4;  break;
    default           : toktyp = t5;  break;
  }
  if (Lex::IsStr(Td.t))   toktyp = t2; // Reduction functions
                                       // Override the reduction functions ?
  next = table[state][toktyp];         // Make the transition
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  : break;
    case 1  : Lx.push_back();
              problem = !StartDocument(phome,xmlname,attr);
              if (problem) break;
              Element();
              attr.clear();
              break;
    case 2  : comments = Lx.SkipTo(Lx.Xy_ecmt);
              Comments(phome,comments);
              comments.clear();
              break;
    case 3  : xmlname=Td.s;
              break;
    case 4  : Lx.push_back();
              Attr();
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

xmlParser::~xmlParser()
//
{

}

//------------------------------------------------------------------------------

bool xmlParser::CDATA(const void * p,const unsigned & type,const string & s)
{
printf("\n==============================================\n");
printf("xmlParser::CDATA() SHOULD BE OVERLOADED\n");
printf("Data structure tag : 0x%#08p\n",p);
printf("CDATA type         : %u\n",type);
printf("CDATA string       : %s\n",s.c_str());
printf("\n==============================================\n");
return true;
}

//------------------------------------------------------------------------------

bool xmlParser::Comments(const void * p,const string & s)
{
printf("\n==============================================\n");
printf("xmlParser::Comments() SHOULD BE OVERLOADED\n");
printf("Data structure tag     : 0x%#08p\n",p);
printf("comment string         : %s\n",s.c_str());
printf("\n==============================================\n");
return true;
}

//------------------------------------------------------------------------------

bool xmlParser::EndDocument(const void * p)
{
printf("\n==============================================\n");
printf("xmlParser::EndDocument() SHOULD BE OVERLOADED\n");
printf("Data structure tag     : 0x%#08p\n",p);
printf("\n==============================================\n");
return true;
}

//------------------------------------------------------------------------------

bool xmlParser::EndElement(const void * p,const string & s)
{
printf("\n==============================================\n");
printf("xmlParser::EndElement() SHOULD BE OVERLOADED\n");
printf("Data structure tag     : 0x%#08p\n",p);
printf("Element name           : %s\n",s.c_str());
printf("\n==============================================\n");
return true;
}

//------------------------------------------------------------------------------

bool xmlParser::Error(const void * p,const unsigned & r,const unsigned & c,
                      const string & s)
{
printf("\n==============================================\n");
printf("xmlParser::Error() SHOULD BE OVERLOADED\n");
printf("Data structure tag     : 0x%#08p\n",p);
printf("Row, col               : %u,%u\n",r,c);
printf("(Last) symbol token    : %s\n",s.c_str());
printf("\n==============================================\n");
return true;
}

//------------------------------------------------------------------------------

bool xmlParser::JSON(const void * p,const string & s,
                     const vector<pair<string,string> > & vps)
{
printf("\n==============================================\n");
printf("xmlParser::JSON() SHOULD BE OVERLOADED\n");
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
{
printf("\n==============================================\n");
printf("xmlParser::StartDocument() SHOULD BE OVERLOADED\n");
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
{
printf("\n==============================================\n");
printf("xmlParser::StartElement() SHOULD BE OVERLOADED\n");
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
 {{E, 0},{E, 0},{R, 4},{2, 2},{E, 0},{3, 0}},  // 2
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
    case 1  : attr.push_back(pair<string,string>(Td.s,string()));
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

void xmlParser::Dump()
{
FILE * fp = stdout;
fprintf(fp,"xmlParser+++++++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"Element name stack\n");
if (elename.empty()) fprintf(fp,"   .. is empty\n");
WALKVECTOR(string,elename,i) fprintf(fp,"   .. %s\n",(*i).c_str());
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
fprintf(fp,"xmlParser-----------------------------------------\n");
}

//------------------------------------------------------------------------------

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
    case  2 : elename.push_back(Td.s);
              break;
    case  3 : problem = !StartElement(phome,elename.back(),attr);
              attr.clear();
              break;
    case  4 : Lx.push_back();
              Attr();
              break;
    case  5 : problem = !StartElement(phome,elename.back(),attr);
              if (problem) break;
              attr.clear();
              problem = !EndElement(phome,elename.back());
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
    case  8 : jsonstr = Td.s + Lx.SkipTo(Lex::Xy_eel1);
              Td.Dump();
              { int l_ = Td.l;      // Get the line and column offsets right
                int c_;
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
    case 11 : problem = !EndElement(phome,elename.back());
              if (problem) break;
              problem = !EndDocument(phome);
              break;
    case 12 : if (elename.back()!=Td.s) problem=true;
              problem = !EndElement(phome,elename.back());
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
// It won't handle anything else
// We DO NOT call Error from here, because the local lexer coordinates refer
// to the string jsonstring, not the parent file.
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
Lx2.SetFile(jsonstr);                  // Point the lexer at the JSON string
for(int state=0;;) {
  Lx2.GetTok(Td);                      // Get the next token...
  rc = Td.c;
//Td.Dump();
//Dump();
  if (Td.t==Lex::Sy_EOR) continue;     // Chuck away any newlines
  if (Lx2.IsError(Td)) problem = true;
  switch (Td.t) {                      // Map to array index
    case Lex::Sy_dqut : toktyp = t0;  break;
    case Lex::Sy_col  : toktyp = t1;  break;
    case Lex::Sy_cmma : toktyp = t3;  break;
    case Lex::Xy_scmt : toktyp = t4;  break;
    default           : toktyp = t5;  break;
  }
  if (Lex::IsStr(Td.t))   toktyp = t2; // Reduction functions
                                       // Override the reduction functions
  next = table[state][toktyp];         // Make the transition
//printf("next,action = %d,%d\n",next.ns,next.ac);
  switch (next.ac) {                   // Post-transition (exit) actions
    case  0 : break;
    case  1 : json.push_back(pair<string,string>(Td.s,string()));
              break;
    case  2 : comments = Lx.SkipTo(Lex::Xy_ecmt);
              Comments(phome,comments);
              comments.clear();
              break;
    case  3 : json.back().second = Td.s;
              break;
    case  4 : Lx.push_back();
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
//    Error(phome,Td.l,Td.c,Td.s); // DON'T call error here 'cos column is wrong
    goto out;
  }
}
out :
json.clear();                          // Whatever, it's broken.
//printf("\nOUT xmlParser::Json()\n - error exit: \n\n");
return true;
}

//------------------------------------------------------------------------------


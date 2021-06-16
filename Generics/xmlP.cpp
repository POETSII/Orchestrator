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

#include "xmlP.h"
#include "macros.h"
#include <stdint.h>

//==============================================================================

xmlP::xmlP()
{
problem = 0;                           // Clear global error flag
// Values of "problem":
emap[0 ] = "No worries";
emap[1 ] = "Input stream returned error token";
emap[2 ] = "Unexpected token encountered";
emap[3 ] = "StartElement() returns an error";
emap[4 ] = "EndElement() returns an error";
emap[5 ] = "CDATA() returns an error";
emap[6 ] = "Json() returns an error";
emap[7 ] = "JSON() returns an error";
emap[8 ] = "EndDocument() returns an error";
emap[9 ] = "Mismatched tag strings";
emap[10] = "StartDocument() returns an error";
fo = stdout;                           // Default text output stream
dflag = 0;                             // Debug flag -> no output
phome = (void *)this;                  // "Let me in" pointer
Lx.SetCFlag(false);                    // Disable C-style comments in SkipTo
}

//------------------------------------------------------------------------------

xmlP::~xmlP()
//
{

}

//------------------------------------------------------------------------------

bool xmlP::CDATA(const void * p,const unsigned & type,const string & s)
// Found a chunk of CDATA. The entire string is handed out
{
if (dflag==0) return true;
fprintf(fo,"\n==============================================\n");
fprintf(fo,"xmlP::CDATA() BASE CLASS INSTANCE\n");
fprintf(fo,"Data structure tag : %" PTR_FMT "\n",OSFixes::getAddrAsUint(p));
fprintf(fo,"CDATA type         : %u\n",type);
fprintf(fo,"CDATA string       : %s\n",s.c_str());
fprintf(fo,"\n==============================================\n");
fflush(fo);
return true;
}

//------------------------------------------------------------------------------

bool xmlP::Comments(const void * p,const string & s)
// Comment string encountered.
// s : The entire comment string
{
if (dflag==0) return true;
fprintf(fo,"\n==============================================\n");
fprintf(fo,"xmlP::Comments() BASE CLASS INSTANCE\n");
fprintf(fo,"Data structure tag     : %" PTR_FMT "\n",
        OSFixes::getAddrAsUint(p));
fprintf(fo,"comment string         : %s\n",s.c_str());
fprintf(fo,"\n==============================================\n");
fflush(fo);
return true;
}

//------------------------------------------------------------------------------

bool xmlP::EndDocument(const void * p)
// End-Of-Document found; processing halts.
{
if (dflag==0) return true;
fprintf(fo,"\n==============================================\n");
fprintf(fo,"xmlP::EndDocument() BASE CLASS INSTANCE\n");
fprintf(fo,"Data structure tag     : %" PTR_FMT "\n",
        OSFixes::getAddrAsUint(p));
fprintf(fo,"\n==============================================\n");
fflush(fo);
return true;
}

//------------------------------------------------------------------------------

bool xmlP::EndElement(const void * p,const string & s)
// End-Of-Element encountered.
// s : Element name, *whether or not* the closure was anonymous
// i.e.(.../> or </stuff>)
{
if (dflag==0) return true;
fprintf(fo,"\n==============================================\n");
fprintf(fo,"xmlP::EndElement() BASE CLASS INSTANCE\n");
fprintf(fo,"Data structure tag     : %" PTR_FMT "\n",
        OSFixes::getAddrAsUint(p));
fprintf(fo,"Element name           : %s\n",s.c_str());
fprintf(fo,"\n==============================================\n");
fflush(fo);
return true;
}

//------------------------------------------------------------------------------

bool xmlP::Error(const void * p,const unsigned & e,const unsigned & r,
                      const unsigned & c,const string & s)
// Syntax error in the XML detected.
// e : error code
// r,c : Row and column in the source file of the last character of the token
// that triggered the error
// s : The last lexical token encountered. (The error *may* lie before this in
// the source.)
// Then we dive into the lexer history buffer just for the hell of it.
{
fprintf(fo,"\n==========================================================\n");
fprintf(fo,"XML syntax error\n");
//fprintf(fo,"xmlP::Error() BASE CLASS INSTANCE\n");
//fprintf(fo,"Data structure tag     : 0x%08p\n",p);
fprintf(fo,"Error code             : (%u) %s\n",e,emap[e].c_str());
fprintf(fo,"Line, col              : %u,%u\n",r,c);
fprintf(fo,"(Last) symbol token    : %s\n",s.c_str());
fprintf(fo,"Lexer token history....\n");
vector<string> vstr;
Lx.Hst.Dump(vstr,10);
WALKVECTOR(string,vstr,i)fprintf(fo,"%s",(*i).c_str());
fprintf(fo,"\n==========================================================\n");
err_v.push_back(err_t(e,r,c,s));
fflush(fo);
return true;
}

//------------------------------------------------------------------------------

unsigned xmlP::ErrCnt()
// Tell the monkey how many times it went wrong, and show it.
{
unsigned ecnt = err_v.size();
//fprintf(fo,"\n%u parse errors found\n\n",ecnt);
//WALKVECTOR(err_t,err_v,i) (*i).Dump(fo);
fflush(fo);
return ecnt;
}

//------------------------------------------------------------------------------

bool xmlP::JSON(const void * p,const string & s,
                     const vector<pair<string,string> > & vps)
// s : The JSON string itself
// vps: A vector of <name,value> pairs.
{
if (dflag==0) return true;
fprintf(fo,"\n==============================================\n");
fprintf(fo,"xmlP::JSON() BASE CLASS INSTANCE\n");
fprintf(fo,"Data structure tag     : %" PTR_FMT "\n",
        OSFixes::getAddrAsUint(p));
fprintf(fo,"Original string        : %s\n",s.c_str());
fprintf(fo,"JSON elements\n");
if (vps.empty()) fprintf(fo,"   .. there are none\n");
for(unsigned i=0;i<vps.size();i++)
  fprintf(fo,"%s : %s\n",vps[i].first.c_str(),vps[i].second.c_str());
fprintf(fo,"\n==============================================\n");
fflush(fo);
return true;
}

//------------------------------------------------------------------------------

void xmlP::Parse(FILE * fp)
//
{
phome = 0;                             // External data structure tag
Lx.SetFile(fp);                        // Point the lexer at it
Lx.SetNFlag(false);                    // Tune lexer to *not* recognise numbers
Lx.SetCFlag(false);                    // To ignore C-comments
//Lx.SetMFlag(true);                     // To interpret '-' as an alphanumeric
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4,t5,t6} toktyp;
struct duple {int ns,ac;} next;
duple table[5][t6+1] =
// Incident symbol                                    // Next
//    0      1      2      3      4      5      6     // state
{{{X, 1},{1, 0},{E, 0},{E, 0},{0, 2},{E, 0},{X, 0}},  // 0
 {{E, 0},{E, 0},{2, 3},{E, 0},{1, 2},{E, 0},{E, 0}},  // 1
 {{E, 0},{E, 0},{3, 4},{4, 0},{2, 2},{E, 0},{E, 0}},  // 2
 {{E, 0},{E, 0},{3, 4},{4, 0},{3, 2},{E, 0},{E, 0}},  // 3
 {{X, 1},{E, 0},{E, 0},{E, 0},{4, 2},{E, 0},{E, 0}}}; // 4

for(int state=0;;) {
  Lx.GetTok(Td);                       // Get the next token...
//  Td.Dump();
  if (Td.t==Lex::Sy_EOR) continue;     // Chuck away any newlines
  if (Lx.IsError(Td)) problem = 1;
  switch (Td.t) {                      // Map to array index
    case Lex::Sy_LT   : toktyp = t0;  break;
    case Lex::Xy_sdcl : toktyp = t1;  break;
    case Lex::Xy_edcl : toktyp = t3;  break;
    case Lex::Xy_scmt : toktyp = t4;  break;
    case Lex::Sy_EOF  : toktyp = t6;  break;
    default           : toktyp = t5;  break;
  }
  if (Lex::IsStr(Td.t))   toktyp = t2; // Reduction functions
                                       // Override the reduction functions ?
  next = table[state][toktyp];         // Make the transition
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  : break;
    case 1  : Lx.push_back();
              if(!StartDocument(phome,xmlname,attr)) problem = 10;
              if (problem!=0) break;
              attr.clear();
              Element();
              attr.clear();
              if(!EndDocument(phome)) problem = 8;
              break;
    case 2  : comments = GetString(Lex::Xy_ecmt);
              Comments(phome,comments);
              comments.clear();
              break;
    case 3  : Lx.push_back();
              xmlname = Calpha().str();
              break;
    case 4  : Lx.push_back();
              Attr();
              break;
    default : break;
  }
  switch (state=next.ns) {
    case X :  goto out;                // Exit
    case R :  goto out;                // Return
    case E :  problem = 2;             // Error
              break;
  }
 if (problem!=0) {                     // May be set elsewhere
    Error(phome,problem,Td.l,Td.c,Td.s);
    goto out;
  }
}
out: return;
}

//------------------------------------------------------------------------------

bool xmlP::SetPtr(const void * p)
{
phome = const_cast<void *>(p);
return true;
}

//------------------------------------------------------------------------------

bool xmlP::StartDocument(const void * p,const string & s,
                              const vector<pair<string,string> > & vps)
// Start of document
// s : Document name
// vps : Vector of <name,value> attributes
{
if (dflag==0) return true;
fprintf(fo,"\n==============================================\n");
fprintf(fo,"xmlP::StartDocument() BASE CLASS INSTANCE\n");
fprintf(fo,"Data structure tag     : %" PTR_FMT "\n",
        OSFixes::getAddrAsUint(p));
fprintf(fo,"Document name          : %s\n",s.c_str());
fprintf(fo,"Attributes\n");
if (vps.empty()) fprintf(fo,"   .. there are none\n");
for(unsigned i=0;i<vps.size();i++)
  fprintf(fo,"%s = %s\n",vps[i].first.c_str(),vps[i].second.c_str());
fprintf(fo,"\n==============================================\n");
fflush(fo);
return true;
}

//------------------------------------------------------------------------------

bool xmlP::StartElement(const void * p,const string & s,
                             const vector<pair<string,string> > & vps)
// The start of an element has been encountered. The is no indication here
// whether or not sub-elements exist.
// s : Element name
// vps : Vector of attribute <name,value> pairs
{
if (dflag==0) return true;
fprintf(fo,"\n==============================================\n");
fprintf(fo,"xmlP::StartElement() BASE CLASS INSTANCE\n");
fprintf(fo,"Data structure tag     : %" PTR_FMT "\n",
        OSFixes::getAddrAsUint(p));
fprintf(fo,"Element name           : %s\n",s.c_str());
fprintf(fo,"Attributes\n");
if (vps.empty()) fprintf(fo,"   .. there are none\n");
for(unsigned i=0;i<vps.size();i++)
  fprintf(fo,"%s = %s\n",vps[i].first.c_str(),vps[i].second.c_str());
fprintf(fo,"\n==============================================\n");
fflush(fo);
return true;
}

//==============================================================================
// End of public interface
//==============================================================================

void xmlP::Attr()
// Handle attributes: key,data pair
{
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4,t5,t6} toktyp;
struct duple {int ns,ac;} next;
duple table[5][t6+1] =
// Incident symbol                                    // Next
//    0      1      2      3      4      5      6     // state
{{{1, 1},{E, 0},{E, 0},{0, 2},{E, 0},{E, 0},{1, 1}},  // 0
 {{R, 3},{2, 0},{R, 3},{1, 2},{R, 3},{R, 3},{R, 3}},  // 1
 {{R, 4},{E, 0},{R, 4},{2, 2},{E, 0},{3, 0},{E, 0}},  // 2
 {{4, 4},{E, 0},{4, 4},{3, 2},{E, 0},{R, 0},{E, 0}},  // 3
 {{E, 0},{E, 0},{E, 0},{4, 2},{E, 0},{R, 0},{E, 0}}}; // 4

for(int state=0;;) {
  Lx.GetTok(Td);                       // Get the next token...
  if (Td.t==Lex::Sy_EOR) continue;     // Chuck away any newlines
  if (Lx.IsError(Td)) problem = 1;
  switch (Td.t) {                      // Map to array index
    case Lex::Sy_STR  : toktyp = t0;  break;
    case Lex::Sy_AS   : toktyp = t1;  break;
    case Lex::Sy_dqut : toktyp = t2;  break;
    case Lex::Xy_scmt : toktyp = t3;  break;
    case Lex::Sy_squt : toktyp = t5;  break;
    case Lex::Sy_col  : toktyp = t6;  break;
    default           : toktyp = t4;  break;
  }
//                                     // Reduction functions
                                       // Override the reduction functions ?
  next = table[state][toktyp];         // Make the transition
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  : break;
    case 1  : Lx.push_back();
              attr.push_back(pair<string,string>(Calpha().str(),string()));
              break;
    case 2  : comments = GetString(Lex::Xy_ecmt);
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
    case E :  problem = 2;             // Error
              break;
  }
  if (problem!=0) {                    // May be set elsewhere
    Error(phome,problem,Td.l,Td.c,Td.s);
    goto out;
  }
}
out: return;
}

//------------------------------------------------------------------------------

xmlP::calpha_t xmlP::Calpha()
// Complex XML names (root:stem) have their very own STG.
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
  if (Lx.IsError(Td)) problem = 1;
  switch (Td.t) {                      // Map to array index
    case Lex::Sy_STR  : toktyp = t0;  break;
    case Lex::Sy_col  : toktyp = t1;  break;
    default           : toktyp = t2;  break;
  }
//                                     // Reduction functions
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
    case E :  problem = 2;             // Error
              break;
  }
  if (problem!=0) {                    // May be set elsewhere
    Error(phome,problem,Td.l,Td.c,Td.s);
    goto out;
  }
}
out: return calpha_t(strR,strS);
}

//------------------------------------------------------------------------------

void xmlP::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n%sxmlP++++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sElement name stack\n",os);
if (elename.empty()) fprintf(fp,"%s   .. is empty\n",os);
WALKVECTOR(calpha_t,elename,i) fprintf(fp,"%s   .. %s\n",os,(*i).str().c_str());
fprintf(fp,"%sAttributes\n",os);
if (attr.empty()) fprintf(fp,"%s   .. there are none\n",os);
for(unsigned i=0;i<attr.size();i++)
  fprintf(fp,"%s   .. %s = %s\n",os,attr[i].first.c_str(),attr[i].second.c_str());
fprintf(fp,"%sJSON\n",os);
if (json.empty()) fprintf(fp,"%s   .. there are none\n",os);
for(unsigned i=0;i<json.size();i++)
  fprintf(fp,"%s   ..%s : %s\n",os,json[i].first.c_str(),json[i].second.c_str());
fprintf(fp,"%sJSON string : %s\n",os,jsonstr.c_str());
fprintf(fp,"%sxmlname     : %s\n",os,xmlname.c_str());
fprintf(fp,"%scdata       : %s\n",os,cdata.c_str());
fprintf(fp,"%scomments    : %s\n",os,comments.c_str());
fprintf(fp,"%sproblem     : %u\n",os,problem);
fprintf(fp,"%sphome       : %" PTR_FMT "\n",os,
        OSFixes::getAddrAsUint(phome));
fprintf(fp,"%sxmlP----------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void xmlP::Element()
// Top level STG to handle an XML element; note Element() is recursive
{
//fprintf(fo,"\nIN xmlP::Element()\n\n");
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
  Lx.GetTok(Td);
//  Td.Dump();                     // Get the next token...
  if (Td.t==Lex::Sy_EOR) continue;     // Chuck away any newlines
  if (Lx.IsError(Td)) problem = 1;
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
                                       // Reduction functions
                                       // Override the reduction functions
  next = table[state][toktyp];         // Make the transition
//fprintf(fo,"next,action = %d,%d\n",next.ns,next.ac);
  switch (next.ac) {                   // Post-transition (exit) actions
    case  0 : break;
    case  1 : comments = GetString(Lex::Xy_ecmt);
              Comments(phome,comments);
              comments.clear();
              break;
    case  2 : Lx.push_back();
              elename.push_back(Calpha());
 // Special cases..... Why, Lord?
// This will match "*:documentation":
              if (elename.back().S ==string("documentation")) {
                string s=GetString(Lex::Xy_eel1);
                GetString(Lex::Sy_GT);
                fprintf(fo,"%s\n",s.c_str());
                Lx.push_back();
                elename.pop_back();
                next.ns=4;
// Oh, for fucks sake. I'm not doing this.
              }
              break;
    case  3 : if(!StartElement(phome,elename.back().str(),attr)) problem = 3;
              attr.clear();
              break;
    case  4 : Lx.push_back();
              Attr();
              break;
    case  5 : if(!StartElement(phome,elename.back().str(),attr)) problem = 3;
              if (problem!=0) break;
              attr.clear();
              if(!EndElement(phome,elename.back().str())) problem = 4;
              elename.pop_back();
              break;
    case  6 : cdata = GetString(Lex::Xy_ecdt);
              if (!CDATA(phome,unsigned(1),cdata)) problem = 5;
              cdata.clear();
              break;
    case  7 : Lx.push_back();
              Element();
              break;
    case  8 : jsonstr = GetString(Lex::Xy_eel1);
              jsonstr = '"' + Td.s + '"' + jsonstr;  // Seriously?
              Lx.push_back();          // SkipTo ate Xy_eel1: so shove it back
              { int l_ = Td.l;         // Get the line and column offsets right
                int c_ = 0;
                if(Json(c_)) problem = 6;
                Td.l = l_;
                Td.c += c_;
                if (problem!=0) break;
              }
              if (!JSON(phome,jsonstr,json)) problem = 7;
              jsonstr.clear();
              json.clear();
              break;
    case  9 : cdata = GetString(Lex::Xy_ecdt);
              if (!CDATA(phome,unsigned(2),cdata)) problem = 5;
              cdata.clear();
              break;
    case 10 : Lx.push_back();
              break;
    case 11 : if (!EndElement(phome,elename.back().str())) problem = 4;
              if (problem!=0) break;
              if (!EndDocument(phome)) problem = 8;
              break;
    case 12 : Lx.push_back();
      //     {   calpha_t c = Calpha();
       //       printf("elename.back=%s calpha=%s\n",
        //            elename.back().str().c_str(),c.str().c_str());
         //     if (elename.back()!=c) problem = 9; }
              if (elename.back()!=Calpha()) problem = 9;
              if (problem!=0) break;
              if(!EndElement(phome,elename.back().str())) problem = 4;
              elename.pop_back();
              break;
    default : break;
  }
  switch (state=next.ns) {
    case X :  goto out;                // Exit
    case R :  goto out;                // Return
    case E :  problem = 2;             // Error
              break;
  }
  if (problem!=0) {                    // May be set elsewhere
    Error(phome,problem,Td.l,Td.c,Td.s);
    problem = 0;                       // Reported it; no point doing it again
    goto out;
  }
}
out:
//fprintf(fo,"\nOUT xmlP::Element()\n\n");
return;
}

//------------------------------------------------------------------------------

string xmlP::GetString(Lex::Sytype tx)
// Yank everything out of the input stream up to the token tx.
// The string equivalent of this gets nailed onto the end, so we have to pull
// it off explicitly
// A problem: the standard is confused about what is supposed to happen if the
// token string can't be found (rfind()) except it does say that no exceptions
// will be thrown. What actually happens (in the version supplied with Borland,
// anyway) is that rfind returns -1, and erase() throws an exception. Hey-ho.
{
string s = Lx.SkipTo(tx);              // Get everything
unsigned u = s.rfind(Lex::Sytype_str[tx]);
if (u==0xffffffff) return s;
s.erase(u);                            // Hoik the token substring off the end
return s;
}

//------------------------------------------------------------------------------

bool xmlP::Json(int & rc)
// A sort of optional sub-parse......
// This is called to parse a string that may or may not be JSON.
// It has its own, local lexer (Lx2) which operates on the class-global string
// "jsonstr". IF the string can be correctly parsed as JSON, the "json" class-
// global structure is loaded.
// THIS IS A NAIVE STRUCTURE THAT ASSUMES ALL JASON is of the form
// "xxx" : 999
// It won't handle anything else; groups and hierarchy are not supported.
// We DO NOT call Error from here, because the local lexer coordinates refer
// to the string jsonstring, not the parent file. If an error is detected,
// the function returns TRUE. If everything is OK, FALSE.
{
//fprintf(fo,"\nIN xmlP::Json()\n\n");
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4,t5,t6} toktyp;
struct duple {int ns,ac;} next;
duple table[4][t6+1] =
// Incident symbol                                   // Next
//    0      1      2      3      4      5     6     // state
{{{1, 1},{E, 0},{1, 1},{E, 0},{1, 2},{E, 0},{E,0}},  // 0
 {{E, 0},{2, 0},{E, 0},{E, 0},{2, 2},{E, 0},{E,0}},  // 1
 {{E, 0},{E, 0},{3, 3},{E, 0},{3, 2},{E, 0},{2,5}},  // 2
 {{E, 0},{E, 0},{E, 0},{0, 0},{E, 0},{R, 4},{E,0}}}; // 3

Lex Lx2;                               // Need a local lexer
Lx2.SetMFlag(true);
Lex::tokdat Td2;
Lx2.SetFile(jsonstr);                  // Point the lexer at the JSON string
for(int state=0;;) {
  Lx2.GetTok(Td2);                     // Get the next token...
  rc = Td2.c;
  if (Td2.t==Lex::Sy_EOR) continue;    // Chuck away any newlines
  if (Lx2.IsError(Td2)) problem = 1;
  switch (Td2.t) {                     // Map to array index
    case Lex::Sy_dqut : toktyp = t0;  break;
    case Lex::Sy_col  : toktyp = t1;  break;
    case Lex::Xy_scmt : toktyp = t4;  break;
    default           : toktyp = t5;  break;
  }
  if (Lex::IsStr(Td2.t))   toktyp = t2;// Reduction functions
  if (Lex::IsOp(Td2.t))    toktyp = t6;
  if (Td2.t==Lex::Sy_cmma) toktyp = t3;// Override the reduction functions
  next = table[state][toktyp];         // Make the transition
//fprintf(fo,"next,action = %d,%d\n",next.ns,next.ac);
  switch (next.ac) {                   // Post-transition (exit) actions
    case  0 : break;
    case  1 : json.push_back(pair<string,string>(Td2.s,string()));
              break;
    case  2 : comments = GetString(Lex::Xy_ecmt);
              Comments(phome,comments);
              comments.clear();
              break;
    case  3 : json.back().second += Td2.s;
              break;
    case  4 : Lx2.push_back();
              break;
    case  5 : json.back().second = Td2.s;
              break;
    default : break;
  }
  switch (state=next.ns) {
    case X :  return false;            // Exit
    case R :  return false;            // Return
    case E :  problem = 2;             // Error
              goto out;
  }
 if (problem!=0) {                     // May be set elsewhere
//    Error(phome,Td2.l,Td2.c,Td2.s); // DON'T call error here 'cos column is wrong
    goto out;
  }
}
out :
json.clear();                          // Whatever, it's broken.
//fprintf(fo,"\nOUT xmlP::Json()\n - error exit: \n\n");
return true;
}

//------------------------------------------------------------------------------



//---------------------------------------------------------------------------

#include "ini.h"
#include "macros.h"

//==============================================================================

bool S2::operator == (S2 & RHS)
{
return (_s1==RHS._s1)&&(_s2==RHS._s2);
}

//==============================================================================

bool S4::operator == (S4 & RHS)
{
return (_s1==RHS._s1)&&(_s2==RHS._s2)&&(_s3==RHS._s3)&&(_s4==RHS._s4);
}

//==============================================================================

INI::INI(char * file,
         void (* Ccb)(void *),
         void (* Ecb)(void *,int),
         void (* Rcb)(void *),
         void (* Scb)(void *)):UIF()
{
SetCCB(Ccb);
SetECB(Ecb);
SetRCB(Rcb);
SetSCB(Scb);
}

//------------------------------------------------------------------------------
             /*
INI::INI(void (* Ccb)(void *),
         void (* Ecb)(void *),
         void (* Rcb)(void *),
         void (* Scb)(void *)):UIF(Ccb,Ecb,Rcb,Scb)
{
}
               */
//------------------------------------------------------------------------------

INI::INI(int argc, char ** argv):UIF(argc,argv)
{
}

//------------------------------------------------------------------------------

INI::INI():UIF()
{
}

//------------------------------------------------------------------------------

INI::~INI()
{

}

//------------------------------------------------------------------------------

bool INI::Decode(void * p,Lex::Sytype & op, vector<string> & ss)
// This routine assumes it has been called by the command callback handler.
// As such, the first argument will be the section node, the last addition to
// which will be the command that triggered this call.
{
UIF::Node * pN = static_cast<UIF::Node *>(p);
if (pN==0) return false;               // Sanity checks...
if (pN->Type()!=No_sect) return false;
if (pN->leaf.back()->Type()!=No_cmnd) return false;
string sinput = pN->leaf.back()->str;  // At last, the string!
                                       // So parse the command....
return Decode(sinput,op,ss);
}

//------------------------------------------------------------------------------

bool INI::Decode(string sinput,Lex::Sytype & op, vector<string> & ss)
// This routine simply decodes whatever is in the first argument.
{
bool problem = false;
Lex Lx;
Lex::tokdat Td;
Lx.SetFile(sinput);                    // Point the lexer at it
Lx.SetNFlag(false);                    // Tune lexer to *not* recognise numbers
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4} toktyp;
struct duple {int ns,ac;} next;
duple table[3][t4+1] =
// Incident symbol                      // Next
//    0      1      2      3      4     // state
{{{1, 0},{X, X},{X, X},{X, X},{X, X}},  // 0
 {{X, X},{2, 1},{2, 2},{X, X},{X, X}},  // 1
 {{X, X},{X, X},{2, 3},{R, 0},{X, X}}}; // 2

op = Lex::S_00;
for(int state=0;;) {
  Lx.GetTok(Td);                       // Get the next token...
//switch (state) {                     // Pre-transition (entry) actions
//  case 0 : cOp = Lex::S_00; break;   // Null current argument sign
//}
                                       // No exceptional cases (EOF in table)
  if (Lx.IsError(Td)) problem = true;
  toktyp = t4;                         // 'Else' token
  if (IsStr(Td.t))        toktyp = t2; // Reduction functions
  if (IsOp(Td.t))         toktyp = t1;
  if (Td.t==Lex::Sy_LT)   toktyp = t0; // Override the reduction functions
  if (Td.t==Lex::Sy_GT)   toktyp = t3;
  next = table[state][toktyp];         // Make the transition
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  :                                                             break;
    case X  : problem = true;                                             break;
    case 1  : // Got an operator
              op = Td.t;                                                  break;
    case 2  : // Command starts with a string
              Lx.push_back();                                             break;
    case 3  : // Store the string
              ss.push_back(Td.s);                                         break;
    default :                                                             break;
  }
  switch (state=next.ns) {
    case X :  return false;
    case R :  return true;
  }
  if (problem==true) return false;
}
//return false;                          // Never here
}

//------------------------------------------------------------------------------

vector<UIF::Node *> INI::FindCommands(UIF::Node * pS)
// Routine to return a vector of command nodes found in the section pS
{
vector<Node *> vN;
WALKVECTOR(Node *,pS->leaf,i) {
  if ((*i)->Type()==UIF::No_cmnd) vN.push_back(*i);
}
return vN;
}

//------------------------------------------------------------------------------

UIF::Node * INI::FindSection(S2 & sect)
// Return the node address of the named section
{
if (UIF_root==0) return 0;
WALKVECTOR(Node *,UIF_root->leaf,i)
  if (INI::Match(GetSectName(*i),sect)) return *i;
return 0;
}

//------------------------------------------------------------------------------

vector<S2> INI::GetSections()
// Return a vector of section name(arg) pairs
{
vector<S2> ans;
if (UIF_root==0) return ans;
WALKVECTOR(Node *,UIF_root->leaf,i) {
  Node * n1 = FindNode(*i,No_name);
  if (n1==0) { ans.push_back(S2()); continue; }
  Node * n2 = FindNode(n1,No_name);
  if (n2==0) { ans.push_back(S2()); continue; }
  Node * n3 = FindNode(n2,No_name);
  if (n3==0)  ans.push_back(S2(n2->str));
  else ans.push_back(S2(n2->str,n3->str));
}
return ans;
}

//------------------------------------------------------------------------------

vector<S2> INI::GetValues(S2 & sect,S4 & vari)
{
vector<S2> ans;
Node * n = FindSection(sect);
if (n==0) return ans;
WALKVECTOR(Node *,n->leaf,i) {
  if ((*i)->typ!=No_recd) continue;
  S2 l = GetRecdLabl(*i);
  S2 v = GetRecdVari(*i);
  S4 x = S4(l,v);
  if (vari==x) return GetRecdValu(*i);
}
return ans;
}

//------------------------------------------------------------------------------

vector<S2> INI::GetValues(string & sect,string & vari)
// Compiler bug? Why do I have to generate the temporaries explicitly?
{
S2 tmp2 = S2(sect);
S4 tmp4 = S4(vari,3);
return GetValues(tmp2,tmp4);
}

//------------------------------------------------------------------------------

vector<S2> INI::GetValues(char * & sect,char * & vari)
{
string s1 = string(sect);
string s2 = string(vari);
return GetValues(s1,s2);
}

//------------------------------------------------------------------------------

vector<S4> INI::GetVariables(S2 & sect)
// Return a vector of the variable signatures in the named section
{
vector<S4> ans;
S4 tmp4;
Node * n = FindSection(sect);
if (n==0) return ans;
WALKVECTOR(Node *,n->leaf,i) {
  if ((*i)->typ!=No_recd) continue;    // Not a record node
  S2 tmp2;
  S4 tmp4;
  tmp4.Clear();
  tmp2 = GetRecdLabl(*i);
  tmp4.s1()=tmp2.s1();
  tmp4.s2()=tmp2.s2();
  tmp2 = GetRecdVari(*i);
  tmp4.s3()=tmp2.s1();
  tmp4.s4()=tmp2.s2();
  ans.push_back(tmp4);
}

return ans;

}

//------------------------------------------------------------------------------

void INI::Mask(vector<S2> & target,S2 m,uchar op)
{
startagain:
WALKVECTOR(S2,target,i) {
  bool f = INI::Match(*i,m);
//printf("op=%x, INI::INI_NOMATCH=%x\n",op,INI::INI_NOMATCH);
  if (op==INI::INI_NOMATCH) f = !f;
  if (f) {
    target.erase(i);
    goto startagain;
  }
}
}

//------------------------------------------------------------------------------

void INI::Mask(vector<S4> & target,S4 m,uchar op)
// Remove elements from 'target' that match 'm'.
// "" is a significant string; "*" is a wildcard
{
startagain:
WALKVECTOR(S4,target,i) {
  bool f = INI::Match(*i,m);
  if (op==INI::INI_NOMATCH) f = !f;
  if (f) {
    target.erase(i);
    goto startagain;
  }
}
}

//------------------------------------------------------------------------------

bool INI::Match(S2 t,S2 m)
{
string w = string("*");
if ((t.s1()!=m.s1())&&(m.s1()!=w)) return false;
if ((t.s2()!=m.s2())&&(m.s2()!=w)) return false;
return true;
}

//------------------------------------------------------------------------------

bool INI::Match(S4 t,S4 m)
{
string w = string("*");
if ((t.s1()!=m.s1())&&(m.s1()!=w)) return false;
if ((t.s2()!=m.s2())&&(m.s2()!=w)) return false;
if ((t.s3()!=m.s3())&&(m.s3()!=w)) return false;
if ((t.s4()!=m.s4())&&(m.s4()!=w)) return false;
return true;
}

//------------------------------------------------------------------------------

bool INI::SectionExists(S2 sect)
// Establish whether or not the named section exists
{
if (UIF_root==0) return false;
if (FindSection(sect)==0) return false;
return true;
}

//------------------------------------------------------------------------------

bool INI::SectionExists(string s1,string s2)
{
S2 tmp = S2(s1,s2);
return SectionExists(tmp);
}

//------------------------------------------------------------------------------

bool INI::SectionExists(string sect)
// Establish whether or not the named section exists
{
return SectionExists(S2(sect));
}

//------------------------------------------------------------------------------

bool INI::VariableExists(S2 & sect,S4 & vari)
{
Node * n = FindSection(sect);
if (n==0) return false;
WALKVECTOR(Node *,n->leaf,i) {
  if ((*i)->typ!=No_recd) continue;
  S2 l = GetRecdLabl(*i);
  S2 v = GetRecdVari(*i);
  S4 x = S4(l,v);
  if (vari==x) return true;
}
return false;
}

//------------------------------------------------------------------------------

bool INI::VariableExists(string & sect,string & vari)
{
S2 tmp2 = S2(sect);
S4 tmp4 = S4(vari,3);
return VariableExists(tmp2,tmp4);
}

//------------------------------------------------------------------------------

bool INI::VariableExists(char * & sect,char * & vari)
{
string s1 = string(sect);
string s2 = string(vari);
return VariableExists(s1,s2);
}

//------------------------------------------------------------------------------

S2 INI::GetSectName(Node * p)
// Get the section name of the section node p
{
S2 ans;
p = FindNode(p,No_name);               // Find the name root
if (p==0) return ans;                  // The node with no name...
p = FindNode(p,No_name);               // First name
if (p==0) return ans;
ans.s1() = p->str;
p = FindNode(p,No_name);               // First argument
if (p==0) return ans;
ans.s2() = p->str;
return ans;
}

//------------------------------------------------------------------------------

S2 INI::GetRecdLabl(Node * p)
// Get the label of the record node p
{
S2 ans;
p = FindNode(p,No_body);
if (p==0) return ans;
p = FindNode(p,No_labl);
if (p==0) return ans;
p=FindNode(p,No_name);
if (p==0) return ans;
ans.s1() = p->str;
p = FindNode(p,No_name);
if (p==0) return ans;
ans.s2() = p->str;
return ans;
}

//------------------------------------------------------------------------------

vector<S2> INI::GetRecdValu(Node * p)
{
vector<S2> ans;
S2 tmp;
p = FindNode(p,No_body);
if (p==0) return ans;
p = FindNode(p,No_valu);
if (p==0) return ans;
vector<Node *> nv = FindNodes(p,No_name);
if (nv.size()==0) return ans;
WALKVECTOR(Node *,nv,i) {
  tmp.s1() = (*i)->str;
  p = FindNode(*i,No_name);
  if (p!=0) tmp.s2() = p->str;
  ans.push_back(tmp);
}
return ans;
}

//------------------------------------------------------------------------------

S2 INI::GetRecdVari(Node * p)
// Get the label of the record node p
{
S2 ans;
p = FindNode(p,No_body);
if (p==0) return ans;
p = FindNode(p,No_vari);
if (p==0) return ans;
p=FindNode(p,No_name);
if (p==0) return ans;
ans.s1() = p->str;
p = FindNode(p,No_name);
if (p==0) return ans;
ans.s2() = p->str;
return ans;
}

//------------------------------------------------------------------------------




















/*
void INI::Save(string savefile)
{
}

//------------------------------------------------------------------------------

bool INI::section_exists(const string& title)
// Does the named section exist?
{
return (FindSection(title)==0) ? false : true;
}

//------------------------------------------------------------------------------

string INI::filename()
// Return the source file name
{
return fname;
}

//------------------------------------------------------------------------------

vector<string> INI::section_names(void)
// Return a vector containig all the section names
{
vector<string> ans;
WALKVECTOR(Node *,UIF_root->leaf,i)
  WALKVECTOR(Node *,(*i)->leaf,j)
    ans.push_back((*j)->leaf[0]->str);
return ans;
}

//------------------------------------------------------------------------------

bool INI::variable_exists(const string& section,const string variable)
// Does the variable "variable" exists in the section "section"?
{
                                       // Find the section
UIF::Node * pSect = FindSection(section);
if (pSect==0) return false;            // If that's not there...

UIF::Node * pRecd = FindRecordVari(pSect,variable);
if (pRecd==0) return false;            // Nope

return true;
}

//------------------------------------------------------------------------------

vector<string> INI::variable_names(const string& section)
// Hand out the variable names in a given section
{
vector<string> ans;
UIF::Node * pSect = FindSection(section);
if (pSect==0) return ans;

WALKVECTOR(Node *,pSect->leaf,i) {   // Walk the records
  printf("i in variable_names is %x\n",(*i));
  UIF::Node * pBody = FindNode(*i,No_body);  // Find the body
  if (pBody!=0) {
    UIF::Node * pVari = FindNode(pBody,No_vari); // Find the variable
    if (pVari!=0) WALKVECTOR(Node *,pVari->leaf,j) ans.push_back((*j)->str);
  }
}
return ans;
}

//------------------------------------------------------------------------------

vector<string> INI::variable_values(const string& section,const string variable)
// Hand out the variable values for a given (section,variable)
{
vector<string> ans;
UIF::Node * pSect = FindSection(section);
if (pSect==0) return ans;
UIF::Node * pRecd = FindRecordVari(pSect,variable);
if (pRecd==0) return ans;
UIF::Node * pBody = FindNode(pRecd,No_body);
if (pBody==0) return ans;
UIF::Node * pValu = FindNode(pBody,No_valu);
if (pValu==0) return ans;
WALKVECTOR(Node *,pValu->leaf,i) ans.push_back((*i)->str);
return ans;
}

//------------------------------------------------------------------------------

UIF::Node * INI::FindSection(string name)
// Find the Node address from the name
{
if (UIF_root==0) return 0;             // Empty object ?
WALKVECTOR(Node *,UIF_root->leaf,i)    // Walk the sections
  WALKVECTOR(Node *,(*i)->leaf,j)      // Walk the section names
    if ((*j)->leaf[0]->str==name) return *i;

return 0;                              // Not there, then ?
}

//------------------------------------------------------------------------------

UIF::Node * INI::FindRecordVari(UIF::Node * pSect,string name)
// Given the section node address, find the named variable record
{
WALKVECTOR(Node *,pSect->leaf,i) {   // Walk the records
  printf("i in FindRecordVari is %x\n",(*i));
  UIF::Node * pBody = FindNode(*i,No_body);  // Find the body
  if (pBody!=0) {
    UIF::Node * pVari = FindNode(pBody,No_vari); // Find the variable
    if (pVari!=0) {
      UIF::Node * pName = FindName(pVari,0); // Find the first name
      if (pName->str==name) return *i;
    }
  }
}
return 0;
}

//------------------------------------------------------------------------------

UIF::Node * INI::FindName(UIF::Node * pVari,int j)
// Given the record variable header node address, find the "j"th name
// (if it exists)
{
WALKVECTOR(Node *,pVari->leaf,i)  {
  printf("i,j in FindName is %x,%d\n",(*i),j); // Walk the leaves
  if (j-- ==0) return (*i);
}
return 0;
}

//------------------------------------------------------------------------------

bool INI::section_exists(Tcpssr)
// Does the section name(arg) exist?
{
return true;
}

//------------------------------------------------------------------------------

INI::Tvpss INI::section_names_2(void)
// Return the section name(arg) list
{

}

//------------------------------------------------------------------------------

INI::Tvs INI::section_names(string name)
// Sections can have the same name but different arguments; given a name, this
// returns the list of section name arguments
{
Tvs ans;
vector<Node *> vn = find_section_names(name);
WALKVECTOR(Node *,vn,i) {
  Node * a = FindNode(n,No_name);
  if (a!=0) ans.push_back(s->str);
}
return ans;
}

//------------------------------------------------------------------------------

bool INI::variable_exists(Tcpssr s,Tcpssr v)
// Does the variable name(arg) exist in the section name(arg)?
{
}

//------------------------------------------------------------------------------

INI::Tvpss INI::variable_names(Tcpssr s)
// Return a vector of variable name(arg) from the section name(arg)
{




}

//------------------------------------------------------------------------------

INI::Tvpss INI::variable_values(Tcpssr s,Tcpssr v)
// Return a vector of value name(arg) for the variable name(arg) in the section
// name(arg)
{
}

//------------------------------------------------------------------------------

bool INI::label_exists(Tcpssr s,Tcpssr l)
// Does the label name(arg) exist in the section name(arg)?
{
}

//------------------------------------------------------------------------------

INI::Tvpss INI::label_names(Tcpssr s)
// Return a list of name(arg) labels in the section name(arg)
{
}

//------------------------------------------------------------------------------

INI::Tvpss INI::label_values(Tcpssr s,Tcpssr l)
// Return a list of name(arg) values for the label name(arg) in the section
// name(arg)
{
}

//------------------------------------------------------------------------------

INI::Tvpss INI::label2variable(Tcpssr s,Tcpssr l)
// Return the vector of name(arg) variables corresponding to the name(arg)
// label in the name(arg) section
{
}

//------------------------------------------------------------------------------

INI::Tvpss INI::variable2label(Tcpssr s,Tcpssr v)
// Return the vector of name(arg) labels corresponding to the name(arg)
// variable in the name(arg) section
{
}

//------------------------------------------------------------------------------

INI::Tvpss INI::values(Tcpssr s,Tcpssr l,Tcpssr v)
// Return the vector of name(arg) values of the name(arg) variable with
// the name(arg) label in the name(arg) section
{
}

//------------------------------------------------------------------------------

vector<UIF::Node *> INI::find_section_names(string name)
// Sections can have the same name but different arguments; given a name, this
// returns the list of section nodes with that name
{
vector<Node *>ans;
vector<Node *> sects = FindNodes(UIF_root,No_sect);
WALKVECTOR(Node *,sects,i) {
  Node * n = FindNode(*i,No_name);
  if (n==0) continue;
  n = FindNode(n,No_name);
  if (n==0) continue;
  if (n->str==name) ans.push_back(*i);
}
return ans;
}

//------------------------------------------------------------------------------

*/

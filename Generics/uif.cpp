//---------------------------------------------------------------------------

#include "uif.h"
#include "macros.h"
#include "flat.h"
#include "dfprintf.h"

//==============================================================================

//class Node;
const string UIF::s_ = string("");
//vector<UIF::Node *> UIF::Node::NodeVector;

//==============================================================================

UIF::UIF()
{
UIF_root = 0;
Init();
}

//------------------------------------------------------------------------------

UIF::UIF(int argc, char ** argv)
// Constructor from command line arguments
{
UIF_root = 0;
Init();
                                       // Turn everything into one string
for (int i=1;i<argc;i++) fname += (argv[i]+string(" "));

Lx.SetFile(fname);                     // Point the lexer at it
Lx.SetNFlag(false);                    // Tune lexer to *not* recognise numbers
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4,t5,t6} toktyp;
struct duple {int ns,ac;} next;
duple table[6][t6+1] =
// Incident symbol                                    // Next
//    0      1      2      3      4      5      6     // state
{{{0, 4},{2, 2},{1, 0},{3, 0},{3, 0},{R, 0},{X, X}},  // 0
 {{0, 1},{X, X},{X, X},{X, X},{X, X},{X, X},{X, X}},  // 1
 {{0, 4},{X, X},{X, X},{X, X},{X, X},{X, X},{X, X}},  // 2
 {{0, 4},{5, 2},{4, 0},{0, 5},{0, 5},{R, 5},{X, X}},  // 3
 {{0, 1},{X, X},{X, X},{X, X},{X, X},{X, X},{X, X}},  // 4
 {{0, 4},{X, X},{X, X},{X, X},{X, X},{X, X},{X, X}}}; // 5

Lex::Sytype cOp = Lex::S_0;            // Keep the compiler happy
Node * pBody;
Node * pVari;
Node * pName;
Node * pLabl;
string sName;
int cnt = 0;
                                       // Store command line as comment
pSect->Add(pRecd = pNH->new_Node(0,No_recd,string(argv[0])));
pRecd->Add(pNH->new_Node(0,No_body));
pSect->Add(pRecd = pNH->new_Node(0,No_recd,fname));
pRecd->Add(pNH->new_Node(0,No_body));


for(int state=0;;) {
  Lx.GetTok(Td);                       // Get the next token...
  switch (state) {                     // Pre-transition (entry) actions
    case 0 : cOp = Lex::S_00; break;   // Null current argument sign
  }
                                       // No exceptional cases (EOF in table)
  if (Lx.IsError(Td)) problem = true;
  switch (Td.t) {                      // Map to array index
    case Lex::Sy_div  : toktyp = t2;  break;
    case Lex::Sy_cmma : toktyp = t3;  break;
    case Lex::Sy_AS   : toktyp = t4;  break;
    case Lex::Sy_EOF  : toktyp = t5;  break;
    default           : toktyp = t6;  break;
  }
  if (Lex::IsStr(Td.t))   toktyp = t0; // Reduction functions
  if (Lex::IsOp(Td.t))    toktyp = t1;
  if (Td.t==Lex::Sy_div)  toktyp = t2; // Override the reduction functions
  if (Td.t==Lex::Sy_cmma) toktyp = t3;
  next = table[state][toktyp];         // Make the transition
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  :                                                             break;
    case X  : problem = true;                                             break;
    case 1  : // New named section
              UIF_root->Add(pSect = pNH->new_Node(0,No_sect,s_,Lex::Sy_div));
              pSect->Add(pName = pNH->new_Node(Td.c,No_name));
              pName->Add(pNH->new_Node(Td.c,No_name,sName=Td.s,Td.t));
              cnt = 0;                                                    break;
    case 2  : // Store single monadic operator
              cOp = Td.t;                                                 break;
    case 4  : // Create a new record; note conditional on variable name
    case 5  : {
              string s = (next.ac==4) ? Td.s : s_;
              pSect->Add(pRecd = pNH->new_Node(Td.l,No_recd));
              pRecd->Add(pBody = pNH->new_Node(Td.c,No_body));
              pBody->Add(pVari = pNH->new_Node(Td.c,No_vari));
              pVari->Add(pNH->new_Node(Td.c,No_name,s,cOp));
              pBody->Add(pLabl = pNH->new_Node(Td.c,No_labl));
              pLabl->Add(pName = pNH->new_Node(Td.c,No_name,sName,Td.t));
              pName->Add(pNH->new_Node(Td.c,No_name,int2str(cnt),Td.t));
              cnt++;                                                      break;
              }
    default :                                                             break;
  }
  switch (state=next.ns) {
    case X :  return;
    case R :  return;
  }
  if (problem==true) break;            // May be set elsewhere
}
return;
}

//------------------------------------------------------------------------------

UIF::~UIF()
// Recursive destruction of node tree
{
Destroy(this,UIF_root);
delete pNH;                            // Kill the node-local heap
}

//------------------------------------------------------------------------------

void UIF::Add(string name)
// Adds the contents of a file to the already existing datastructure
{
problem = false;                       // So far, so good
Lx.SetFile((char *)name.c_str());      // Point the lexer at the input file
                                       // BORLAND BUG: we need the cast???
if (Lx.GetErr()!=Lex::S_0) {           // Problem - bomb
  errcnt++;
  return;
}
//Lx.SetNFlag(false);                    // Tune lexer to *not* recognise numbers
Lx.SetCChar('\\');                     // Set lexer continuation character
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12} toktyp;
struct duple {int ns,ac;} next;
duple table[10][t12+1] =
// Incident symbol                                                                              // Next
//    0      1      2      3      4      5      6      7      8      9     10     11     12     // state
{{{1, 3},{1, 3},{2, 0},{4, 0},{6, 6},{7,10},{X, X},{0, 7},{0, 0},{0, 8},{1, 3},{X, X},{R, 0}},  // 0
 {{X, X},{X, X},{2, 9},{4, 0},{6, 6},{X, X},{X, X},{X, X},{0, 0},{0, 8},{X, X},{X, X},{R, 0}},  // 1
 {{3, 4},{3, 4},{X, X},{4, 0},{6, 6},{X, X},{X, X},{X, X},{0, 0},{0, 8},{3, 4},{X, X},{R, 0}},  // 2
 {{X, X},{X, X},{X, X},{4, 0},{6, 6},{X, X},{X, X},{X, X},{0, 0},{0, 8},{X, X},{X, X},{R, 0}},  // 3
 {{5, 5},{5, 5},{X, X},{X, X},{X, X},{X, X},{X, X},{X, X},{0, 0},{0, 8},{5, 5},{X, X},{R, 0}},  // 4
 {{X, X},{X, X},{X, X},{X, X},{6, 6},{X, X},{X, X},{X, X},{0, 0},{0, 8},{X, X},{X, X},{R, 0}},  // 5
 {{X, X},{X, X},{X, X},{X, X},{X, X},{X, X},{X, X},{X, X},{0, 0},{0, 8},{X, X},{X, X},{R, 0}},  // 6
 {{8, 1},{8, 1},{X, X},{X, X},{X, X},{X, X},{9, 2},{X, X},{X, X},{X, X},{8, 1},{X, X},{X, X}},  // 7
 {{X, X},{X, X},{X, X},{X, X},{X, X},{X, X},{9, 0},{X, X},{X, X},{X, X},{X, X},{X, X},{X, X}},  // 8
 {{X, X},{X, X},{X, X},{X, X},{X, X},{X, X},{X, X},{X, X},{0, 0},{0, 8},{X, X},{X, X},{R, 0}}}; // 9

Node * pName = 0;
Node * pBody = 0;                      // Local declares - keep compiler happy
Node * pLabl = 0;
Node * pVari = 0;
Node * pValu = 0;
Node * tmp;

for(int state=0;;) {                   // And around and around we go ...
  Lx.GetTok(Td);                       // Get the next token...
  if (Lx.IsError(Td)) break;           // Lexer reports a problem?
  switch (state) {                     // Pre-transition (entry) actions
    case 0 : PruneRec();               // Delete unnecessary nodes
             RCB();
             pSect->Add(pRecd = pNH->new_Node(Td.l,No_recd));
             pRecd->Add(pBody = pNH->new_Node(Td.c,No_body));
             pBody->Add(pLabl = pNH->new_Node(Td.c,No_labl));
             pBody->Add(pVari = pNH->new_Node(Td.c,No_vari));
             pBody->Add(pValu = pNH->new_Node(Td.c,No_valu));
             break;
    case 9 : SCB(true);
             break;
  }
                                       // No exceptional cases (EOF in table)
  switch (Td.t) {                      // Map to array index
    case Lex::Sy_col  : toktyp = t2;  break;
    case Lex::Sy_AS   : toktyp = t3;  break;
    case Lex::Sy_lbrc : toktyp = t4;  break;
    case Lex::Sy_lsqb : toktyp = t5;  break;
    case Lex::Sy_rsqb : toktyp = t6;  break;
    case Lex::Sy_LT   : toktyp = t7;  break;
    case Lex::Sy_EOR  : toktyp = t8;  break;
    case Lex::Sy_cmnt :
    case Lex::Sy_semi : toktyp = t9;  break;
    case Lex::Sy_lrnb : toktyp = t10; break;
    case Lex::Sy_EOF  : toktyp = t12; break;
    default           : toktyp = t11; break;
  }
  if (Lex::IsStr(Td.t)) toktyp = t0;   // Reduction functions
  if (Lex::IsOp(Td.t))  toktyp = t1;
  if (Td.t==Lex::Sy_LT) toktyp = t7;   // Override the reduction functions
  next = table[state][toktyp];         // Make the transition
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  :                                                             break;
    case X  : problem = true;                                             break;
    case 1  : // Create a new named section header
              Lx.push_back();
              UIF_root->Add(pSect = pNH->new_Node(Td.l,No_sect,s_));
              pSect->Add(pName = pNH->new_Node(Td.c,No_name));
              pQal(pName);                                                break;
    case 2  : // Create a blank section header. Need the name to hold a comment
              UIF_root->Add(pSect = pNH->new_Node(Td.l,No_sect));
              pSect->Add(pName = pNH->new_Node(Td.c,No_name));            break;
    case 3  : // Pull in the first qualified name field - assume it's the
              // *variable* field by default
              Lx.push_back();
              pQal(pVari);                                                break;
    case 4  : // Pull in the second qualified name field - here we *know* this
              // is the variable field
              Lx.push_back();
              pQal(pVari);                                                break;
    case 5  : // Pull in the third qualified name field - here we *know* this
              // is the value field
              Lx.push_back();
              pQal(pValu);                                                break;
    case 6  : // Start of attribute list
              pAtl(pRecd);                                                break;
    case 7  : // Found a command; remove unnecessary record node
    //          Lx.push_back();
              pSect->Sub(this);
              pSect->Add(pRecd = pNH->new_Node(Td.l,No_cmnd));
              CmdProc(pRecd);
              CCB();                                                      break;
    case 8  : // The Great Comment Bodge. What can I say? Sorry.
              if (pRecd==0) CmtProc(pSect,pName);
              else CmtProc(pRecd,pBody); // GCC chokes if break follows else
                                                                          break;
    case 9  : // *Now* we know which field is which; swap variable and value
              tmp = pVari; pVari = pLabl; pLabl = tmp;
              pVari->Type(No_vari);
              pLabl->Type(No_labl);                                       break;
    case 10 : // Start a new section; remove unnecessary record node
              PruneRec(); SCB(false); pSect->Sub(this); pRecd = 0;        break;
    default :                                                             break;
  }
  if ((state=next.ns) == R) {          // Legit exit - we're done
    PruneRec();
    SCB(false);                        // Report the last section
    Lx.SetFile();                      // Disconnect lexer
    return;
  }
  if ((state==X)||(problem==true)) {   // Error handler triggered
    problem = false;                   // Clear flag
    Lx.SkipTo('\n');                   // Junk rest of offending record
    state = 0;                         // Reset state
    ECB();                             // Error callback - and away we go again
  }
  if (stop) {                          // Legitimate exit
    Lx.SetFile();                      // Disconnect lexer
    return;
  }
}
ECB();                                 // Punch out the errors
Lx.SetFile();                          // Disconnect lexer
return;
}

//------------------------------------------------------------------------------

void UIF::Addx(char * name)
// I can't overload Add(string), because BORLAND doesn't seem to be able to
// correctly resolve a call to Add(0)...
{
if (name!=0) Add(string(name));
}

//------------------------------------------------------------------------------

void UIF::Args()
// Clear preprocessor argument map
{
argMap.clear();
}

//------------------------------------------------------------------------------

void UIF::Args(string s_f)
// Preprocessor argument map defined by a file. Somewhat recursively, we need
// to parse this *inside* this, the parser. The problem is that we can't
// (reasonably) see JNJ from here, so the parsing is somewhat clunky and manual.
{
                                       // Dud file just ignored (for now)
if (!file_readable(s_f.c_str())) return;
UIF * pUIF = new UIF;                  // Parser for argument file
pUIF->Add(s_f);
Node * root = pUIF->Root();
unsigned nsects = root->leaf.size();   // Count the sections
Node * psect = 0;                      // Section pointer
unsigned nrecds = 0;                   // Records in the section
if (nsects < 2) goto out;              // Only the blank one present ?
psect = root->leaf[1];                 // Grab the first named section
nrecds = psect->leaf.size();  // Count the records
if (nrecds < 2) goto out;              // No records, just the section name
                                       // Loop through argument list
for (unsigned recd=1;recd<nrecds;recd++) {
  Node * precd = psect->leaf[recd];
  if (precd->leaf.empty()) continue;   // Record has no body
  Node * pbody = precd->leaf[0];       // Body node (probably)
  if (pbody->typ != No_body) continue; // Not a body
  if (pbody->leaf.empty()) continue;   // Body has no fields
  Node * pvari = 0;                    // Now (try to) find {variable,value}
  Node * pvalu = 0;
  for (unsigned i=0;i<pbody->leaf.size();i++) {
    if ((pvari==0)&&(pbody->leaf[i]->typ==No_vari)) pvari = pbody->leaf[i];
    if ((pvalu==0)&&(pbody->leaf[i]->typ==No_valu)) pvalu = pbody->leaf[i];
  }
  if ((pvari==0)||(pvalu==0)) continue;// Not a sensible pair
  if (pvari->leaf.empty()) continue;   // No variable data
  if (pvalu->leaf.empty()) continue;   // No value data
                                       // LHS properly labelled?
  if (pvari->leaf[0]->qop!=Lex::Sy_hash) continue;
                                       // RHS properly labelled?
//  if (pvalu->leaf[0]->qop!=Lex::S_00) continue;
//*************************
// BUG: if the LHS is in double quotes, the opertaor is dqt, not null.
//*************************
                                       // At last! Do it
  Args(pvari->leaf[0]->str,pvalu->leaf[0]->str);
       // And this, children, is why I wrote JNJ.
}
out: if (pUIF!=0) delete pUIF;         // Chuck away the transient parser
}

//------------------------------------------------------------------------------

void UIF::Args(string s_a,string s_d)
// Add an entry to the preprocessor argument map
{
argMap[s_a] = s_d;
}

//------------------------------------------------------------------------------

void UIF::CCB()
// Command callback
{
if (cb.com_cb!=0)cb.com_cb(cb.com_pt,(void *)this,(void *)pRecd);
}

//------------------------------------------------------------------------------

void UIF::Collapse(vector<Node *> * ptmp)
// The expression parser builds - and hands in - a daisy-chain of tokens.
// After each *token* is added, this routine is called on the daisy-chain, to
// see if it can be collapsed in any way by one of the six transforms embodied
// in t123() or t456(). (There is no sinister reason why these are separate;
// it's just that 1,2 and 3 are related and so are 4,5 and 6. The loop here is
// because some transforms enable others, so for each addition of a token (i.e.
// each call of this routine) we try all the transforms again and again until
// nothing happens any more.
{
/*
int count = 0;
fprintf(ofp,"++Pre-collapse++\n");
WALKVECTOR(Node *,(*ptmp),i){
  fprintf(ofp,"Node %d ...\n",count++);
  (*i)->Dump();
  fprintf(ofp,"... end of\n");
}

fprintf(ofp,"Pre-collapse +++\n");
WALKVECTOR(Node *,(*ptmp),i) (*i)->Dump();
fprintf(ofp,"\n"); */
do {} while (t123(ptmp)||t456(ptmp));
/*
fprintf(ofp,"Post-collapse ---\n");
WALKVECTOR(Node *,(*ptmp),i) (*i)->Dump();
fprintf(ofp,"\n");

count = 0;
WALKVECTOR(Node *,(*ptmp),i){
  fprintf(ofp,"Node %d ...\n",count++);
  (*i)->Dump();
  fprintf(ofp,"... end of\n");
}
fprintf(ofp,"--Post-collapse--\n"); */
}

//------------------------------------------------------------------------------

void UIF::CmdProc(Node * pCmnd)
// Command handler
{
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4} toktyp;
struct duple {int ns,ac;} next;
duple table[4][t4+1] =
// Incident symbol                      // Next
//    0      1      2      3      4     // state
{{{1, 1},{2, 2},{X, X},{R, 0},{X, X}},  // 0
 {{X, X},{2, 2},{X, X},{X, X},{X, X}},  // 1
 {{3, 3},{3, 3},{3, 3},{R, 0},{X, X}},  // 2
 {{X, X},{X, X},{X, X},{R, 0},{X, X}}}; // 3

for(int state=0;;) {
  Lx.GetTok(Td);                       // Get the next token...
//  switch (state) {                     // Pre-transition (entry) actions
//  }
  if (Td.t == Lex::Sy_EOF) return;     // Exceptional case
  if (Lx.IsError(Td)) problem = true;
  switch (Td.t) {                      // Map to array index
    case Lex::Sy_lrnb : toktyp = t2;  break;
    case Lex::Sy_GT   : toktyp = t3;  break;
    default           : toktyp = t4;  break;
  }
  if (Lex::IsStr(Td.t)) toktyp = t1;   // Reduction functions
  if (Lex::IsOp(Td.t))  toktyp = t0;
  if (Td.t==Lex::Sy_GT) toktyp = t3;   // Override the reduction functions
  next = table[state][toktyp];         // Make the transition
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  :                                                             break;
    case X  : problem = true;                                             break;
    case 1  : // Command starts with an operator
              pCmnd->qop = Td.t;                                          break;
    case 2  : // Command name
              pCmnd->str = Td.s;                                          break;
    case 3  : // Command proper is a qualified list
              Lx.push_back();
              pQal(pCmnd);                                                break;
    default :                                                             break;
  }
  switch (state=next.ns) {
    case X :  return;
    case R :  return;
  }
  if (problem==true) break;           // May be set elsewhere
}

//string buf = Lx.SkipTo(Lex::Sy_GT);    // Pull in command guts
//Lx.SkipTo('\n');                       // Junk the EOR
//return buf;                            // Return the command
return;
}

//------------------------------------------------------------------------------

void UIF::CmtProc(Node * pS, Node * pP)
// Comment processor. This is a bodge, because I forgot to design in anywhere
// to store the comment column address. As it is, if it's a SECTION comment,
// it's shoved into the position field in the section name node, and if it's a
// record, it goes into the record body node. Sorry. Me prat.
// In fact, it's an even bigger bodge:
// If we're here because the comment was recognised from a ';', the comment
// string will be pulled in as the rest of the line.
// If we're here because we got a comment *token* - i.e. Sy_cmnt, the comment
// string will have already been pulled in by the lexer, and it's in the token
{
if (Td.t == Lex::Sy_cmnt) {
  pS->str = Td.s;
                                       // Adjust pointer to hold comment start
  pP->pos = Td.c - (int)(pS->str).size();
  Lx.SkipTo('\n');                     // Chuck away superfluous EOR
} else {
  pS->str = Lx.SkipTo('\n');
  pP->pos = Td.c;
}
return;
}

//------------------------------------------------------------------------------

void UIF::DeBody(UIF * pUIF,Node * p)
// Called on a record node; we chop off the entire subtree, leaving the record
// node behind. This is so that the 'current record' pointer doesn't get
// confused in the UIF body. The subtree will contain most of the memory
// resources of the record, so it's virtually a 'delete' on the record anyway,
// plus we don't have to bugger about deleteing the parent leaf pointer.
{
if (p==0) return;                      // Probably a problem......
if (p->Type()!=No_recd) return;        // Probably a problem......
if (p->leaf.size()!=1) return;         // Probably a problem......
if (p->leaf[0]->Type()!=No_body) return; // Oh, for heavens sake
Destroy(pUIF,p->leaf[0]);              // Hose the subtree
p->leaf.clear();                       // Tidy up
}

//------------------------------------------------------------------------------

void UIF::DefECB(void * pThis,void * p,int id)
// Default error callback
// pThis is the object address, which is n/u in this here default handler
// Never called with id=0, which is just as well, 'cos Node::Dump() is
// expecting a string argument.....
{
FILE * chan = stdout;
fprintf(chan,"\n+------------------------+\n");
fprintf(chan,"UIF default error handler: id %d\n",id);
switch (id) {
  case 0  : static_cast<UIF::Node *>(p)->Dump(chan);         break;
  case 1  : static_cast<UIF *>(p)->Lx.Hst.Dump(chan);        break;
  default : fprintf(chan,"Unrecognised error identifier\n"); break;
}
fprintf(chan,"\n+------------------------+\n");
}

//------------------------------------------------------------------------------

void UIF::DeNull(Node * p)
// Called by the expression handler; walk the tree rooted on p, and lose any
// null children
{
vector<Node *> vN = p->leaf;
p->leaf.clear();
WALKVECTOR(Node *,vN,i) if ((*i)!=0) p->leaf.push_back(*i);
WALKVECTOR(Node *,p->leaf,i) DeNull(*i);
}

//------------------------------------------------------------------------------

void UIF::Destroy(UIF * pUIF,Node * p)
// Trash one node and its children
{
if (p==0) return;
WALKVECTOR(Node *,p->leaf,i)Destroy(pUIF,*i);
pUIF->pNH->delete_Node(p);
}

//------------------------------------------------------------------------------

void UIF::Dump(string dumpfile)
// Diagnostic pretty print
{
static FILE * df;
df = ofp;
if (!dumpfile.empty()) df = fopen(dumpfile.c_str(),"w");
fprintf(df,"\n-----------------------------------------\n");
fprintf(df,"UIF object dump (%s)\n",fname.c_str());
fprintf(df,"problem = %c\n",problem ? 'T' : 'F');
fprintf(df,"stop    = %c\n",stop    ? 'T' : 'F');
Td.Dump(df);
fprintf(df,"UIF preprocessor string map - %lu entries:\n", argMap.size());
WALKMAP(string,string,argMap,i) {
  fprintf(df,"%10s -> %10s\n",(*i).first.c_str(),(*i).second.c_str());
}
if (UIF_root!=0)UIF_root->Dump(df);
fprintf(df,"\n-----------------------------------------\n\n");
if (!dumpfile.empty()) fclose(df);
}

//------------------------------------------------------------------------------

void UIF::ECB()
// Error callback
{
errcnt++;
if (cb.err_cb!=0)cb.err_cb(cb.err_pt,(void *)this,1);
}

//------------------------------------------------------------------------------

int UIF::ErrCnt()
// Returns the number of times ECB has been invoked
{
return errcnt;
}

//------------------------------------------------------------------------------

UIF::Node * UIF::Expr()
// Hands out a (properly) formed expression tree
{
enum loctok  {t0=0,t1,t2,t3,t4,t5,t6} toktyp;
struct duple {int ns,ac;} next;
duple table[1][t6+1] =
// Incident symbol
//    0      1      2      3      4      5      6
{{{0, 2},{0, 2},{0, 2},{0, 2},{R, 1},{R, 1},{X, X}}};  // 0  Next state

vector<Node *> tmp;
for(int state=0;;) {
  Notype No_x;
  Lex::tokdat Td;
  Lx.GetTok(Td);
                                       // No pre-transition actions
  if (Td.t==Lex::Sy_EOF) break;
  if (Lx.IsError(Td)) problem = true;
  switch (Td.t) {
    case Lex::Sy_lrnb : toktyp = t2;  break;
    case Lex::Sy_rrnb : toktyp = t3;  break;
    case Lex::Sy_semi : toktyp = t4;  break;
    case Lex::Sy_rbrc : toktyp = t5;  break;
    default           : toktyp = t6;  break;
  }
  if (Lex::IsStr(Td.t))  toktyp = t0;  // Reduction functions
  if (Td.t==Lex::Sy_col) toktyp = t1;  // ':' - in this context - is an operator
  if (Lex::IsOp(Td.t))   toktyp = t1;
                                       // Map Symbol types onto expression types
  switch (toktyp) {                    // This is necessary because Collapse()
    case t0 : No_x = No_e_ex;  break;  // can change these types
    case t1 : No_x = No_e_op;  break;
    case t2 : No_x = No_e_LB;  break;
    case t3 : No_x = No_e_RB;  break;
    case t4 : No_x = No_XXXX;  break;
    case t5 : No_x = No_XXXX;  break;
    case t6 : No_x = No_XXXX;  break;
    default : No_x = No_XXXX;  break;  // Keeping GCC happy
  }
  next = table[state][toktyp];
  switch (next.ac) {
    case 0 :                                                              break;
    case X : problem = true;                                              break;
    case 1 : Lx.push_back();                                              break;
// Time for a rant: I *want* to overload the new() operator to finesse a local
// storage manager for the enode class, but whatever I do, I can't make the
// overloaded new() pick up any constructor apart from the default. And I don't
// want the default, I want the overload in the line below.
    case 2 : tmp.push_back(pNH->new_Node(Td.c,No_x,Td.s,Td.t));
             Collapse(&tmp);                                              break;
    default:                                                              break;
  }
  switch (state=next.ns) {
    case X :
    case R : WALKVECTOR(Node *,tmp,i)DeNull(*i);
             if (!tmp.empty())Tertiaries(tmp[0]);
             if (tmp.size()==1) return tmp[0];
             problem = true;
             if (tmp.empty()) return 0;
             return tmp[0];
  }
  if (problem==true) break;
}
                                       // Never here
return tmp[0];
}

//------------------------------------------------------------------------------

UIF::Node * UIF::FindNode(Node * p,Notype t)
// Given a node address, find the address of the first child of type t
{
WALKVECTOR(Node *,p->leaf,i) if ((*i)->Type()==t) return (*i);
return 0;
}

//------------------------------------------------------------------------------

vector<UIF::Node *> UIF::FindNodes(Node * p,Notype t)
// Given a node address, find the address of all the children of type t
{
vector<Node *> ans;
WALKVECTOR(Node *,p->leaf,i) if ((*i)->Type()==t) ans.push_back(*i);
return ans;
}

//------------------------------------------------------------------------------

void UIF::Init()
{
pNH       = new NodeHeap();            // Internal memory manager
problem   = false;                     // No errors yet
stop      = false;                     // 'Stop' flag
Destroy(this,UIF_root);                // Kill any old datastructure
UIF_root  = pNH->new_Node();           // Initialise root
                                       // Create blank section
UIF_root->Add(pSect = pNH->new_Node(0,No_sect));
pRecd     = 0;                         // "Current record" isn't
                                       // Initialise callback functions
cb.com_cb = 0;                         // Command....
cb.com_pt = 0;
cb.err_cb = DefECB;                    // Errors....
cb.err_pt = 0;
cb.rec_cb = 0;                         // Record....
cb.rec_pt = 0;
cb.sec_cb = 0;                         // Section....
cb.sec_pt = 0;
ofp       = stdout;                    // Output stream goes to screen
errcnt    = 0;                         // No errors yet
}

//------------------------------------------------------------------------------

void UIF::pAtl(Node * in)
// Given a node, this loads any attribute list nodes
{
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4,t5,t6,t7} toktyp;
struct duple {int ns,ac;} next;
duple table[5][t7+1] =
// Incident symbol                                           // Next
//    0      1      2      3      4      5      6      7     // state
{{{1, 1},{1, 1},{R, 0},{2, 0},{X, X},{X, X},{1, 1},{X, X}},  // 0
 {{X, X},{X, X},{R, 0},{2, 0},{0, 0},{4, 4},{X, X},{X, X}},  // 1
 {{3, 2},{3, 2},{R, 0},{X, X},{0, 0},{4, 3},{3, 2},{X, X}},  // 2
 {{X, X},{X, X},{R, 0},{X, X},{0, 0},{4, 3},{X, X},{X, X}},  // 3
 {{X, X},{X, X},{R, 0},{X, X},{0, 0},{X, X},{X, X},{X, X}}}; // 4

Node * pLabl = 0;                      // Keep the compiler happy
Node * pVari = 0;
Node * pExpr = 0;
Node * pAttr;
Node * tmp;

for(int state=0;;) {
  Lx.GetTok(Td);                       // Get the next token...
  switch (state) {                     // Pre-transition (entry) actions
    case 0 : in->Add(pAttr = pNH->new_Node(in->pos,No_attr));
             pAttr->Add(pLabl = pNH->new_Node(Td.c,No_labl));
             pAttr->Add(pVari = pNH->new_Node(Td.c,No_vari));
             pAttr->Add(pExpr = pNH->new_Node(Td.c,No_expr));
             break;
  }
  if (Td.t == Lex::Sy_EOF) return;     // Exceptional case
  if (Lx.IsError(Td)) problem = true;
  switch (Td.t) {                      // Map to array index
    case Lex::Sy_rbrc : toktyp = t2;  break;
    case Lex::Sy_col  : toktyp = t3;  break;
    case Lex::Sy_semi : toktyp = t4;  break;
    case Lex::Sy_AS   : toktyp = t5;  break;
    case Lex::Sy_lrnb : toktyp = t6;  break;
    default           : toktyp = t7;  break;
  }
  if (Lex::IsStr(Td.t)) toktyp = t0;   // Reduction functions
  if (Lex::IsOp(Td.t))  toktyp = t1;
  next = table[state][toktyp];         // Make the transition
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  :                                                             break;
    case X  : problem = true;                                             break;
    case 1  : // Load a single qualified name as the label
              Lx.push_back();
              pQal(pLabl);                                                break;
    case 2  : // Load a single qualified name as the variable
              Lx.push_back();
              pQal(pVari);                                                break;
    case 3  : // Load an expression
              pExpr->Add(Expr());                                         break;
    case 4  : // Load an expression
              pExpr->Add(Expr());
              // The first string is the variable, not the label
              tmp = pVari; pVari = pLabl; pLabl = tmp;
              pVari->Type(No_vari); pLabl->Type(No_labl);                 break;
    default :                                                             break;
  }
  switch (state=next.ns) {
    case X :  return;
    case R :  return;
  }
  if (problem==true) break;           // May be set elsewhere
}
return;
}

//------------------------------------------------------------------------------

UIF::Node * UIF::pQal(Node * in)
// Given a node, this loads any children with a qualified list
{
                                       // And away we go...
enum loctok  {t0=0,t1,t2,t3,t4,t5} toktyp;
struct duple {int ns,ac;} next;
duple table[5][t5+1] =
// Incident symbol                             // Next
//    0      1      2      3      4      5     // state
{{{2, 6},{1, 1},{2, 3},{R, 8},{0, 7},{R, 5}},  // 0
 {{2, 2},{X, X},{3, 4},{R, 5},{0, 0},{R, 5}},  // 1
 {{R, 5},{R, 5},{3, 4},{R, 5},{0, 0},{R, 5}},  // 2
 {{X, X},{X, X},{X, X},{4, 0},{X, X},{X, X}},  // 3
 {{R, 5},{R, 5},{R, 5},{R, 5},{0, 0},{R, 5}}}; // 4

Node * tmp = 0;
for(int state=0;;) {
  Lx.GetTok(Td);                       // Get the next token...
                                       // No pre-transition (entry) actions
  if (Td.t == Lex::Sy_EOF) return in;  // Exceptional case
  if (Lx.IsError(Td)) problem = true;
  switch (Td.t) {                      // Map to array index
    case Lex::Sy_lrnb : toktyp = t2;  break;
    case Lex::Sy_rrnb : toktyp = t3;  break;
    case Lex::Sy_cmma : toktyp = t4;  break;
    default           : toktyp = t5;  break;
  }
  if (Lex::IsStr(Td.t)) toktyp = t0;
  if (Lex::IsOp(Td.t))  toktyp = t1;
  if (Td.t==Lex::Sy_cmma) toktyp = t4; // Override the reduction functions
  next = table[state][toktyp];         // Make the transition
  switch (next.ac) {                   // Post-transition (exit) actions
    case 0  :                                                             break;
    case X  : problem = true;                                             break;
    case 1  : // Create a new Node, holding a name and operator symbol
              in->Add(tmp = pNH->new_Node(Td.c,No_name,s_,Td.t));         break;
    case 2  : // *Overwrite* the string
              tmp->str = Td.s;                                            break;
    case 3  : // Found a '(' - add a new child node
              Lx.push_back();
              in->Add(tmp = pNH->new_Node(Td.c,No_name));                 break;
    case 4  : // Load the new child node
              pQal(tmp);                                                  break;
    case 5  : // Exit, pursued by a bear
              Lx.push_back();                                         return in;
    case 6  : // Qualified name has no operator
              in->Add(tmp = pNH->new_Node(Td.c,No_name,Td.s,Td.t));       break;
    case 7  : in->Add(pNH->new_Node(Td.c,No_name,s_,Td.t));               break;
    case 8  : in->Add(pNH->new_Node(Td.c,No_name,s_,Td.t));
              Lx.push_back();                                             break;
    default :                                                             break;
  }
  switch (state=next.ns) {
    case X :  return in;
    case R :  return in;
  }
  if (problem==true) break;            // May be set elsewhere
}
return in;
}

//------------------------------------------------------------------------------

void UIF::PruneRec()
// Routine to hack off empty record body, label, value and variable nodes
// There has to be a more elegant way of doing this, but, hey ....
{
if (pRecd==0) return;                  // Paranoia...
if (pRecd->Type()!=No_recd) return;    // Paranoia...

WALKVECTOR(Node *,pRecd->leaf,i) {     // The children must be body or attribute
  PruneRec2(*i,No_valu);               // Kill each type in turn
  PruneRec2(*i,No_vari);
  PruneRec2(*i,No_labl);
}
                                       // Now loop to kill the body or attribute
                                       // nodes themselves if they're empty
for (uint i=0;i<pRecd->leaf.size();i++) if (pRecd->leaf[i]->leaf.size()==0) {
  pNH->delete_Node(pRecd->leaf[i]);
  pRecd->leaf.erase(pRecd->leaf.begin()+i);
}

}

//------------------------------------------------------------------------------

void UIF::PruneRec2(Node * p,Notype t)
// Routine to kill off a subtree (of type t) if it's empty
{
WALKVECTOR(Node *,p->leaf,i) if (((*i)->Type()==t)&&((*i)->leaf.size()==0)) {
  pNH->delete_Node(*i);                // If the node is empty, kill it
  p->leaf.erase(i);
  break;
}

}

//------------------------------------------------------------------------------

Lex::tokdat & UIF::Query(bool * pe)
// Hand out the status (value) of the last token handed out from the lexer,
// plus an indication of if UIF thinks it's an error (useful when there's a
// problem connecting to input files and so on)
// The rather weird interface to this function is so that you can call it with
// the minimum of infrastructure build in the calling routine.
{
if (pe!=0) *pe = Lx.IsError(Td);
return Td;
}

//------------------------------------------------------------------------------

void UIF::RCB()
// Record callback
{
if (pRecd==0) return;                  // Suppress the very first one
if (pRecd->Type()==No_cmnd) return;    // Don't push a command out as a record
pRecd->Args(this);                     // Handle any preprocessor substitutions
if (cb.rec_cb!=0)cb.rec_cb(cb.rec_pt,(void *)this,(void *)pRecd);
}

//------------------------------------------------------------------------------

void UIF::Reset()
// Clear and reset all internal data structures
{
Destroy(this,UIF_root);
Init();
}

//------------------------------------------------------------------------------

UIF::Node * UIF::Root()
// The routine that blows holes in any remaining notion of encapsulation. It
// returns the address of the root node, from which you can do what you like.
// The original intention was to provide a name::address map of all the
// section nodes as well. Like in Trondheim, I buggered about with the
// STL::multimap until my head hurt and gave up. Life's too short.
{
return UIF_root;
}

//------------------------------------------------------------------------------

void UIF::Save(string savefile)
// What comes out is pretty much what went in....
{
sf = stdout;
string sv;                             // Line buffer
if (!savefile.empty()) sf = fopen(savefile.c_str(),"w");
sv.clear();                            // Clear line buffer
Save0(sf,UIF_root,sv);                 // Recurse the tree
if (!savefile.empty()) fclose(sf);
}

//------------------------------------------------------------------------------

void UIF::Save0(FILE * sf,Node * p,string & sv)
// Recursive pretty printer
{
if (p==0) return;
Notype t=p->Type();                    // Save some typing
Node * px=NULL;                        // Scratch node pointer
switch (t) {
                 // Root: Save each sector in turn
  case No_0000 : WALKVECTOR(Node *,p->leaf,i) Save0(sf,(*i),sv); return;
  case No_sect : // Find the name
                 WALKVECTOR(Node *,p->leaf,i) {
                   if ((px=(*i))->Type()==No_name) {
                     dprintf(sv,"[");
                     WALKVECTOR(Node *,(*i)->leaf,j) Save0(sf,(*j),sv);
                     dprintf(sv,"]");
                     break;
                   }
                 }
                 // Any comment ?
                 if (p->str.size()!=0) {
                   unsigned int x = px->pos;
                   if (x > sv.size())sv.append(x-sv.size(),' ');
                   sv.append(1,';');
                   sv.append(p->str);
                   //dprintf(sv,"\t\t\t; %s",p->str.c_str());
                 }
                 fprintf(sf,"%s\n",sv.c_str());
                 sv.clear();
                 // Find the records and commands
                 WALKVECTOR(Node *,p->leaf,i)
                   if (((*i)->Type()==No_recd)||((*i)->Type()==No_cmnd))
                     Save0(sf,(*i),sv);
                 break;
  case No_recd : {
                 bool fAttr = false;
                 // Find the body
                 px = p;   // In case there isn't one.
                 WALKVECTOR(Node *,p->leaf,i) {
                   if ((px=(*i))->Type()==No_body) Save0(sf,(*i),sv);
                   if ((*i)->Type()==No_attr) fAttr = true;
                 }
                 if (fAttr) {
                   dprintf(sv," {");
                   // Find the attribute list (if it exists)
                   WALKVECTOR(Node *,p->leaf,i) {
                     if ((*i)->Type()==No_attr) Save0(sf,(*i),sv);
                     if (i!=p->leaf.begin())
                       if (i!=(p->leaf.end()-1)) dprintf(sv,"; ");
                   } // WALKVECTOR
                   dprintf(sv,"}");
                 }
                 // Any comment ?
                 if (p->str.size()!=0) {
                   unsigned int x = px->pos;
                   if (x > sv.size())sv.append(x-sv.size(),' ');
                   sv.append(1,';');
                   sv.append(p->str);
                 }
                 fprintf(sf,"%s\n",sv.c_str());
                 sv.clear();
                 break;
                 }
  case No_cmnt : break;
  case No_cmnd : dprintf(sv,"<%s%s>",Lex::Sytype_str[p->qop],p->str.c_str());
                 fprintf(sf,"%s\n",sv.c_str());
                 sv.clear();
                 break;
  case No_body : // Find the single label node
                 WALKVECTOR(Node *,p->leaf,i)
                   if ((px=(*i))->Type()==No_labl) {
                     Save0(sf,(*i),sv);
                     if (px->leaf.size()!=0) dprintf(sv," : ");
                   }
                 // Find the single variable node
                 WALKVECTOR(Node *,p->leaf,i)
                   if ((*i)->Type()==No_vari) Save0(sf,(*i),sv);
                 // Find the single value node
                 WALKVECTOR(Node *,p->leaf,i)
                   if ((px=(*i))->Type()==No_valu) {
                 // Only if there is one do we need an "="
                     if (px->leaf.size()!=0)dprintf(sv," = ");
                 // And again
                     Save0(sf,px,sv);
                   }
                 break;
  case No_attr : // Find the single label node
                 WALKVECTOR(Node *,p->leaf,i)
                   if ((px=(*i))->Type()==No_labl) {
                     Save0(sf,(*i),sv);
                     if (px->leaf.size()!=0) dprintf(sv," : ");
                   }
                 // Find the single variable node
                 WALKVECTOR(Node *,p->leaf,i)
                   if ((*i)->Type()==No_vari) Save0(sf,(*i),sv);
                 // Find the single value node
     /*            WALKVECTOR(Node *,p->leaf,i)
                   if ((px=(*i))->Type()==No_valu) {
                 // Only if there is one do we need an "="
                     if (px->leaf.size()!=0)dprintf(sv," = ");
                 // And again
                     Save0(sf,px,sv);
                   }  */
                 // Find expression node
                 WALKVECTOR(Node *,p->leaf,i)
                   if ((px=(*i))->Type()==No_expr) {
                 // Only if there is one do we need an "="
                     if (px->leaf.size()!=0)dprintf(sv," = ");
                 // And again
                     Save0(sf,px,sv);
                   }
                 break;
/*
  case No_attr : // A single attribute
                 WALKVECTOR(Node *,p->leaf,i) Save0(sf,(*i),sv);
                 break;
*/
  case No_labl : WALKVECTOR(Node *,p->leaf,i) {
                   Save0(sf,(*i),sv);
                   if (i!=(p->leaf.end()-1)) dprintf(sv,",");
                 }
                 break;
  case No_vari : WALKVECTOR(Node *,p->leaf,i) {
                   Save0(sf,(*i),sv);
                   if (i!=(p->leaf.end()-1)) dprintf(sv,",");
                 }
                 break;
  case No_valu : WALKVECTOR(Node *,p->leaf,i) {
                   Save0(sf,(*i),sv);
                   if (i!=(p->leaf.end()-1)) dprintf(sv,",");
                 }
                 break;
// Name:
  case No_name :  //dprintf(sv,"%s%s",Lex::Sytype_str[p->qop],p->str.c_str());
                  if (p->qop==Lex::Sy_dqut) dprintf(sv,"\"");
                  dprintf(sv,"%s",p->str.c_str());
                  if (p->qop==Lex::Sy_dqut) dprintf(sv,"\"");
                  if (p->leaf.size()!=0) dprintf(sv,"(");
                  WALKVECTOR(Node *,p->leaf,i) {
                    Save0(sf,(*i),sv);
                    if (i!=(p->leaf.end()-1)) dprintf(sv,",");
                  }
                  if (p->leaf.size()!=0) dprintf(sv,")");
                  break;
  case No_expr :  if (UIF::TertL(p)!=0) Save0(sf,UIF::TertL(p),sv);
                  break;
  case No_e_ex :  switch(p->leaf.size()) {
                    case 0 : dprintf(sv,"%s",p->str.c_str());
                             break;
                    case 1 : if (p->qop==Lex::Sy_STR)
                               dprintf(sv,"%s(",p->str.c_str());
                             else
                               dprintf(sv,"%s(",Lex::Sytype_str[p->qop]);
                             Save0(sf,UIF::TertL(p),sv);
                             dprintf(sv,")");
                             break;
                    case 2 : dprintf(sv,"(");
                             Save0(sf,UIF::TertL(p),sv);
                             dprintf(sv,"%s",Lex::Sytype_str[p->qop]);
                             Save0(sf,UIF::TertR(p),sv);
                             dprintf(sv,")");
                             break;
                    case 3 : dprintf(sv,"(");
                             Save0(sf,UIF::TertL(p),sv);
                             dprintf(sv,"%s",Lex::Sytype_str[Lex::Sy_qst]);
                             Save0(sf,UIF::TertR(p),sv);
                             dprintf(sv,"%s",Lex::Sytype_str[Lex::Sy_col]);
                             Save0(sf,p->leaf[2],sv);
                             dprintf(sv,")");
                             break;
                    default: dprintf(sv," Operator with > 3 operands ");
                             break;
                  }
                  break;
  case No_e_op :  dprintf(sv," = **No_e_op**");   break;
  case No_e_LB :  dprintf(sv," = **No_e_LB**");   break;
  case No_e_RB :  dprintf(sv," = **No_e_RB**");   break;
  case No_XXXX :  dprintf(sv," = **No_XXXX**");   break;
  default : return;
}

}

//------------------------------------------------------------------------------

void UIF::SCB(bool b)
// Section callback; boolean flag indicates entry(T)/exit(F)
{
pSect->Args(this);                     // Handle any preprocessor substitutions
if (cb.sec_cb!=0)cb.sec_cb(cb.sec_pt,(void *)this,b,(void *)pSect);
}

//------------------------------------------------------------------------------

void UIF::SetCCB(void * pThis,void (* Ccb)(void *,void *,void *))
// Command callback
{
cb.com_cb = Ccb;
cb.com_pt = pThis;
}

//------------------------------------------------------------------------------

void UIF::SetECB(void * pThis,void (* Ecb)(void *,void *,int))
// Error callback
{
cb.err_cb = Ecb;
cb.err_pt = pThis;
}

//------------------------------------------------------------------------------

void UIF::SetOFP(FILE * f)
// Set output file stream
{
ofp = f;
}

//------------------------------------------------------------------------------

void UIF::SetRCB(void * pThis,void (* Rcb)(void *,void *,void *))
// Record callback
{
cb.rec_cb = Rcb;
cb.rec_pt = pThis;
}

//------------------------------------------------------------------------------

void UIF::SetSCB(void * pThis,void (* Scb)(void *,void *,bool,void *))
// Section callback
{
cb.sec_cb = Scb;
cb.sec_pt = pThis;
}

//------------------------------------------------------------------------------

void UIF::SetStop(bool f)
// Access the 'stop' flag
{
stop = f;
}

//------------------------------------------------------------------------------

bool UIF::t123(vector<Node *> * pt)
// Transform 1 handles the sequence  Ex-Op-Ex
//           2                      ~Ex-Op-Ex
//           3                          Op-Ex
// Note the code looks a touch clumsy in places; we *can't* do pointer
// arithmetic 'cos [] is dereferencing a vector, not an array here.
{
uint p = (unsigned int)(pt->size()-1); // p == Index of last node
if (p<1) return false;                 // Nothing to transform
if (!(*pt)[p]->IsEx()) return false;   // End node != expression, so go
if (!(*pt)[p-1]->IsOp()) return false; // Penultimate node != operator, so go
// The list has at least two elements in it, ending with an expr, so we're
// going to do *something* - and the first step is common to all three
// transforms: cut the end off and move it to the R child of the (new) end
Node * tmp0 = (*pt)[p];                // Point to the end node
pt->pop_back();                        // Pull it off
(*pt)[p-1]->R() = tmp0;                // Re-attach as R child of new end
tmp0->P() = (*pt)[p-1];                // Connect new child to its parent
(*pt)[p-1]->Type(No_e_ex);             // Change new end type to expression
if (p==1) return true;                 // Transform 3 - that was it.
if (!(*pt)[p-2]->IsEx()) return true;  // Transform 2 - that was it too.
tmp0 = (*pt)[p-1];                     // Point to new end
pt->pop_back();                        // Pull it off
Node * tmp1 = (*pt)[p-2];              // Point to new end
pt->pop_back();                        // Pull it off
pt->push_back(tmp0);                   // Re-attach first new end
tmp0->L() = tmp1;                      // Re-attach second new end
tmp1->P() = tmp0;                      // Connect new child to its parent
return true;                           // Transform 1 - that was it
}

//------------------------------------------------------------------------------

bool UIF::t456(vector<Node *> * pt)
// Transform 4 handles the sequence  Ex-(-Ex-)
//           5                       Op-(-Ex-)
//           6                          (-Ex-)
{
Node * x;
uint p = (unsigned int)(pt->size()-1); // p == Index of last node
if (p<2) return false;                 // Nothing for us here
if (!(*pt)[p]->IsRB()) return false;   // End node != ), so go
if (!(*pt)[p-1]->IsEx()) return false; // Penultimate node != expression, so go
if (!(*pt)[p-2]->IsLB()) return false; // Antepenultimate node != (, so go
if ((p>=3)&&((*pt)[p-3]->IsEx())) {    // Transform 4
  x = pt->back();                      // Discard the end ')'
  pNH->delete_Node(x);
  pt->pop_back();
  Node * tmp0 = (*pt)[p-1];            // Save the Ex
  pt->pop_back();                      // Pull it off
  x = pt->back();                      // Discard the '('
  pNH->delete_Node(x);
  pt->pop_back();
  Node * tmp1 = (*pt)[p-3];
  while (tmp1->R()!=0) tmp1=tmp1->R(); // Find the R-most leaf
  tmp1->R() = tmp0;                    // Tack on the Ex
  tmp0->P() = tmp1;                    // Connect the new child to its parent
  return true;                         // Transform 4 - that was it.
}

if ((p>=3)&&(!(*pt)[p-3]->IsEx())) {   // Transform 5
  x = pt->back();                      // Discard last node (')')
  pNH->delete_Node(x);
  pt->pop_back();
  Node * tmp0 = (*pt)[p-1];            // Save end (Ex)
  pt->pop_back();                      // Pull it off
  x = pt->back();                      // Discard last node ('(')
  pNH->delete_Node(x);
  pt->pop_back();
  pt->push_back(tmp0);                 // Re-attach Ex
  return true;                         // Transform 5 - that was it.
}

if (p==2) {                            // Transform 6
  x = pt->back();                      // Discard last node (')')
  pNH->delete_Node(x);
  pt->pop_back();
  Node * tmp0 = (*pt)[p-1];            // Save end (Ex)
  pt->pop_back();                      // Pull it off
  x = pt->back();                      // Discard last node ('(')
  pNH->delete_Node(x);
  pt->pop_back();
  pt->push_back(tmp0);                 // Re-attach Ex
  return true;
}
return  false;                         // Keep the compiler happy
}

//------------------------------------------------------------------------------
/* 23/8/12: These next routines - Tert???? - are all to do with re-parsing the
expression trees to extract the tertiary ?: operators and re-arrange the tree
as necessary. At the output of Expr(), the tree is (supposedly) binary, accessed
by P(), L() and R(). Which is fine, except that (a) there are null branches
in it, (b) there is a cryptic comment in the code implying that leaf[0] is
reserved for the parent backpointer but not used, (c) there is a node field
called .par that clearly *is* used as the backpointer, and (d) reading the
code of L() and R() shows that left==leaf[1] and right==leaf[2].
In other words, it's a bit of a mess. I'm uneasy about trying to fix it, because
my memory of developing the t123 and t456 code was that it made my head hurt for
days, it now works, and I don't want to mess with it.
Now, UIF::DeNull() strips out the null pointers from the expression subtree,
which makes TreeDumper happy, but it completely buggers subsequent use of L()
and R().
So: After UIF::DeNull(), the expression subtree is binary, inasmuch as the
nodes can have 0, 1 or 2 children, but no more. So we have
Node * UIF::TertL(Node *) and Node * UIF::TertR(Node *), which hand out values
(NOT references) of the left and right children. If there is only one child, it
is defined to be the LEFT : left=leaf[0], right = leaf[1].
Note these are methods of UIF, not UIF::Node.
*/
//------------------------------------------------------------------------------

void UIF::TertDo1(pair<Node *,Node *> & P)
// See UIF notes
{
Node * NQ = P.first;
Node * NC = P.second;
Node * NX = TertL(NQ);
Node * X  = TertL(NX);
Node * A  = TertR(NC);
Node * B  = TertR(NQ);
Node * C  = TertR(NX);
Node::Disconnect(X);
Node::Disconnect(C);
Node::Disconnect(B);
Node::Disconnect(A);
Node::Disconnect(NX);
Node::Disconnect(NQ);
NC->Add(X);
NC->Add(NX);
NX->Add(C);
NX->Add(B);
NX->Add(A);
pNH->delete_Node(NQ);
NC->qop=Lex::Sy_qst;      NC->str=Lex::Sytype_str[NC->qop];
NX->qop=Lex::Sy_T3;       NX->str=Lex::Sytype_str[NX->qop];
//NC->Dump();
}

//------------------------------------------------------------------------------

void UIF::TertDo2(pair<Node *,Node *> & P)
// See UIF notes
{
Node * NQ = P.first;
Node * NC = P.second;
Node * A  = TertR(NC);
Node * B  = TertR(NQ);
Node * T  = TertL(NQ);
Node::Disconnect(NQ);
Node::Disconnect(A);
Node::Disconnect(B);
Node::Disconnect(T);
NC->Add(T);
NC->Add(B);
NC->Add(A);
pNH->delete_Node(NQ);
NC->qop=Lex::Sy_T3;        NC->str=Lex::Sytype_str[NC->qop];
//NC->Dump();
}

//------------------------------------------------------------------------------

void UIF::Tertiaries(Node * N)
// Routine to re-parse the expression node tree to identify and re-arrange the
// ?: tertiary operators
{
if (N==0) return;                      // Broken already ?
vector<Node *> V1;                     // Colon stack
vector<pair<Node *,Node *> > V2;       // ?: pair stack
TertLocP(N,V1,V2);                     // Identify ?: pairs and transform them
if (!V1.empty()) {                     // Mis-matched ?: pair? Bail
  problem = true;
  return;
}
//printf("\n");
//WALKVECTOR(Node *,V1,i)(*i)->Dumpt();
//printf("\n");
//for(vector<pair<Node *,Node *> >::iterator i=V2.begin();i!=V2.end();i++) {
//  (*i).first->Dumpt();
//  (*i).second->Dumpt();
//  printf("..\n");
//}
//printf("\n");
                                       // Walk the ?: pairs
for(vector<pair<Node *,Node *> >::iterator i=V2.begin();i!=V2.end();i++)
  if (TertL((*i).first)->qop==Lex::Sy_qst) {
    TertDo1(*i);                       // Transform 1?
                                       // The pointers for any subsequent
                                       // transforms may need adjusting...
    for(vector<pair<Node *,Node *> >::iterator j=i;j!=V2.end();j++)
      if((*j).first==TertR((*i).second)) (*j).first=(*i).second;
  }
  else TertDo2(*i);                    // Transform 2?
}

//------------------------------------------------------------------------------

UIF::Node * UIF::TertL(Node * N)
// See stand-alone comment block above
{
if (N==0) return 0;
if (N->leaf.size()>0) return N->leaf[0];
return 0;
}

//------------------------------------------------------------------------------

void UIF::TertLocP(Node * N,vector<Node *> & V1,vector<pair<Node*,Node*> > & V2)
// Locate all ?: pairs of expression nodes
// V1 is a "colon stack". We recursively walk the tree, pushing ":" onto the
// stack. When we find a "?", we pop the last ":", and push the {?,:} pair
// onto the "pair stack".
// Error conditions:
// 1. On exit (from the TertLocP stack), the colon stack should be empty
// 2. Attempting to pop an empty colon stack at any time
{
if (N->qop==Lex::Sy_col) V1.push_back(N);
if (N->qop==Lex::Sy_qst) {
  if (V1.empty()) problem = true;
  else {
    V2.push_back(pair<Node *,Node *>(N,V1.back()));
    V1.pop_back();
  }
}
if (TertL(N)!=0) TertLocP(TertL(N),V1,V2);
if (TertR(N)!=0) TertLocP(TertR(N),V1,V2);
}

//------------------------------------------------------------------------------

UIF::Node * UIF::TertR(Node * N)
// See stand-alone comment block above
{
if (N==0) return 0;
if (N->leaf.size()>1) return N->leaf[1];
return 0;
}


//==============================================================================

const char * UIF::Notype_str[] = {
  "No_0000","No_sect","No_recd","No_cmnd","No_cmnt","No_body","No_attr",
  "No_labl","No_vari","No_valu","No_name","No_expr","No_e_ex","No_e_op",
  "No_e_LB","No_e_RB","No_XXXX"};

//==============================================================================

UIF::Node::Node(int p,Notype ty,const string & s,Lex::Sytype Op)
{
typ = ty;                              // Node type (sect/recd/...)
str = s;                               // Name
qop = Op;                              // Optional single opcode
pos = p;                               // Source position
par = 0;                               // No parents
tag = 0;
}

//------------------------------------------------------------------------------

UIF::Node::~Node()
{
}

//------------------------------------------------------------------------------

void UIF::Node::Add(Node * n)
// Add a child node to the current one
{
if (n==0) return;                      // If we've come in in the error state...
leaf.push_back(n);                     // Add it to the child list
n->par = this;                         // Set the parent
}

//------------------------------------------------------------------------------

void UIF::Node::Args(UIF * pUIF)
// Handle any preprocessor substitutions in the tree
{
if (Type()==No_name)                   // Name node?
                                       // String in substitution map?
  if (pUIF->argMap.find(str)!=pUIF->argMap.end())
    if (qop==Lex::Sy_hash)             // With the preprocessor '#'?
      str = pUIF->argMap[str];         // So finally do it

                                       // Walk the node tree
WALKVECTOR(Node *,leaf,i) if ((*i)!=0) (*i)->Args(pUIF);
}

//------------------------------------------------------------------------------

void UIF::Node::Disconnect(Node * & p)
// Disconnect the node(subtree) 'p' from any parent tree
{
if (p==0) return;                      // Nothing to do
Node * pP = p->P();                    // Find Mum (if any)
if (pP==0) return;                     // Nothing to do
vector<Node *>::iterator iX;           // Find Mummys iterator
WALKVECTOR(Node *,pP->leaf,i) if ((*i)==p) iX = i;
pP->leaf.erase(iX);                    // Delete iterator from Mums child vector
p->P()=0;                              // Disconnect parent pointer

}

//------------------------------------------------------------------------------

void UIF::Node::Dump(FILE * df,string s0)
{
fprintf(df,"\n---\n%sDumping node %p (par %p) type |%s|\n",
           s0.c_str(),(void*)this,(void*)par,Notype_str[typ]);
fprintf(df,"%sOpcode |%s|\n",s0.c_str(),Lex::Sytype_str[qop]);
fprintf(df,"%sString |%s|\n",s0.c_str(),str.c_str());
fprintf(df,"%sPos %d\n",s0.c_str(),pos);
//if (p->qop==No_expr) dynamic_cast<Exp_Node *>(p)->E->Dump();
fprintf(df,"%s has %lu children: ",s0.c_str(),leaf.size());
s0 += "  ";
WALKVECTOR(Node *,leaf,i) if ((*i)==0) fprintf(df,"Null child\n");
                          else (*i)->Dump(df,s0);
}

//------------------------------------------------------------------------------

void UIF::Node::Dumpt(FILE * fp)
// Tiny inline Node dump
{
fprintf(fp,"%6p(%2d) %s:%s[%s]\n",
       (void*)this,pos,Notype_str[typ],str.c_str(),Lex::Sytype_str[qop]);
}

//------------------------------------------------------------------------------

void UIF::Node::Dumpx(FILE * fp,string s0)
// Reasonably coherently structured dump for sections and records
{
string sv;
Node * px = 0;
switch (typ) {
  case UIF::No_sect : dprintf(sv,"["); // Note different behaviour to Save()
                      WALKVECTOR(Node *,leaf,i)  // Find the name
                        if ((px=(*i))->Type()==No_name)
                          WALKVECTOR(Node *,(*i)->leaf,j) Save0(stdout,(*j),sv);
                      dprintf(sv,"]");
                      if (str.size()!=0) {       // Any comment
                        if (px->pos > int(sv.size()))sv.append(px->pos-sv.size(),' ');
                        sv.append(1,';');
                        sv.append(str);
                      }
                      fprintf(stdout,"%s\n",sv.c_str());
                      sv.clear();
                      break;
  case UIF::No_recd : UIF::Save0(stdout,this,sv);  break;
  default           : Dump(fp,s0);
}

}

//------------------------------------------------------------------------------

bool UIF::Node::IsEx()
{
return typ == No_e_ex;
}

//------------------------------------------------------------------------------

bool UIF::Node::IsOp()
{
return typ == No_e_op;
}

//------------------------------------------------------------------------------

bool UIF::Node::IsLB()
{
return typ == No_e_LB;
}

//------------------------------------------------------------------------------

bool UIF::Node::IsRB()
{
return typ == No_e_RB;
}

//------------------------------------------------------------------------------

UIF::Node * & UIF::Node::L()
// The generic Node has an arbitrary number of children, but the expression tree
// needs only a binary tree, and the transforms conveniently operate on L and R
// children. Thus the code is written in terms of .L and .R, even though they
// don't exist. This routine makes it look like there *is* a left child, and it
// is accessed via the reference returned by this function.
{
// Make sure the L and R children actually exist before we return the reference
if (leaf.size()==0) for(int i=0; i<3; i++) leaf.push_back(0);
// And return child [1] as the left child.
return leaf[1];
// Node [0] is reserved for the parent backpointer, but I've not yet found it
// necessary
}

//------------------------------------------------------------------------------

UIF::Node * & UIF::Node::P()
// Returns the parent
{
return par;
}

//------------------------------------------------------------------------------

UIF::Node * & UIF::Node::R()
// As above; node[2] is the right child.
{
if (leaf.size()==0) for(int i=0; i<3; i++) leaf.push_back(0);
return leaf[2];
}

//------------------------------------------------------------------------------

void UIF::Node::Type(Notype ty)
{
typ = ty;
}

//------------------------------------------------------------------------------

UIF::Notype UIF::Node::Type(unsigned & i)
// Return node type and leaf size (i.e. provides empty test)
{
i = leaf.size();
return typ;
}

//------------------------------------------------------------------------------

UIF::Notype UIF::Node::Type()
{
return typ;
}

//------------------------------------------------------------------------------

void UIF::Node::Src(int & l,int & c)
// Routine to return the token source position
{
l=c=0;
if (typ==No_0000) return;              // It's the root
l = pos;
if (typ==No_sect) return;              // It's a section definition
if (typ==No_recd) return;              // It's a record definition
Node * p = this;                       // Something else....
                                       // Find the anchor recd/sect
                                       // Early return possible if node floating
while ((p->typ!=No_sect)&&(p->typ!=No_recd)) if ((p=p->par)==0) return;
l = p->pos;
c = pos;
}

//------------------------------------------------------------------------------

void UIF::Node::Sub(UIF * pUIF)
// Remove the last child node (and any associated children)
{
UIF::Destroy(pUIF,leaf.back());
leaf.pop_back();
}

//==============================================================================

UIF::NodeHeap::~NodeHeap()
{
WALKVECTOR(Node *,NodeVector,i) delete *i;
}

//------------------------------------------------------------------------------

void UIF::NodeHeap::delete_Node(Node * p)
{
NodeVector.push_back(p);
unsigned u = NodeVector.size();
if (u > maxcnt) maxcnt = u;
//printf("NodeHeap cnt = %d\n",maxcnt);
}

//------------------------------------------------------------------------------

UIF::Node * UIF::NodeHeap::new_Node(int p,Notype ty,const string & s,Lex::Sytype Op)
// Local memory heap for nodes
{
//printf("NodeHeap cnt = %d\n",maxcnt);
if (NodeVector.empty()) return new Node(p,ty,s,Op);
Node * pN = NodeVector.back();
NodeVector.pop_back();
pN->typ = ty;                              // Node type (sect/recd/...)
pN->str = s;                               // Name
pN->qop = Op;                              // Optional single opcode
pN->pos = p;                               // Source position
pN->par = 0;                               // No parents
pN->leaf.clear();                          // No children
pN->tag = 0;
return pN;
}

//==============================================================================


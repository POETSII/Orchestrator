#ifndef __UIF__H
#define __UIF__H

#include "lex.h"
#include <vector>
#include <map>
using namespace std;

//==============================================================================

class UIF {
public:
class Node;                            // Forward declare of subclass
enum Notype {No_0000 = 0,              // Types of node
     No_sect,No_recd,No_cmnd,No_cmnt,No_body,No_attr,No_labl,No_vari,
     No_valu,No_name,No_expr,No_e_ex,No_e_op,No_e_LB,No_e_RB,No_XXXX};
static const char * Notype_str[No_XXXX+1];

                   UIF();              // Empty constructor
                   UIF(int,char **);   // Command line constructor
virtual ~          UIF(void);
void               Add(string);        // Add the contents of another file
void               Addx(char *);       // Add the contents of another file
void               Args();             // Clear argument map
void               Args(string);       // Set entire argument map from file
void               Args(string,string);// Set element in argument map
void               CCB();              // Command callback - do it
void               Collapse(vector<Node *> *);
void               CmdProc(Node *);    // Pull out any embedded command
void               CmtProc(Node *,Node *);
static void        DeBody(UIF *,Node *);// Hack off and delete the body
static void        DefECB(void *,void *,int); // Default error callback
void               DeNull(Node *);     // Tidy expression subtree
static void        Destroy(UIF *,Node *);     // Static; called from within Node
void               Dump(string=s_);    // Diagnostic dump
void               ECB();              // Error callback - do it
int                ErrCnt();           // Number of times ECB invoked
Node *             Expr();             // Parse an expression
static Node *      FindNode(Node *,Notype);
static vector<Node *> FindNodes(Node *,Notype);
void               Init();             // Common constructor/reset code
void               pAtl(Node *);       // Parse an attribute list
Node *             pQal(Node *);       // Parse a qualified name
void               PruneRec();         // Delete unnecessary nodes from record
void               PruneRec2(Node *,Notype);
Lex::tokdat &      Query(bool * =0);   // Peek at last token and error status
void               RCB();              // Record callback - do it
void               Reset();            // Re-initialise the internal structure
Node *             Root();             // Take me, I'm yours.
void               Save(string=s_);    // Pretty print
static void        Save0(FILE *,Node *,string &);
void               SCB(bool);          // Section callback - do it
void SetCCB(void *,void(*)(void *,void *,void *));   // Set command callback
                  // Final argument should default to 0 but u$oft won't allow it
void SetECB(void *,void(*)(void *,void *,int));      // Set error callback
                  // Last two arguments default to 0
void SetOFP(FILE * = stdout);          // Set output file stream
void SetRCB(void *,void(*)(void *,void *,void *));     // Set record callback
                  // Last argument defaults to 0
void SetSCB(void *,void(*)(void *,void *,bool,void *));// Set section callback
                  // Last argument defaults to 0
void               SetStop(bool);      // Access the 'stop' flag
bool               t123(vector<Node *> *);
bool               t456(vector<Node *> *);
void               TertDo1(pair<Node *,Node *> &);
void               TertDo2(pair<Node *,Node *> &);
void               Tertiaries(Node *);
static Node *      TertL(Node *);
void               TertLocP(Node*,vector<Node*> &,vector<pair<Node*,Node*> > &);
static Node *      TertR(Node *);

Node *             UIF_root;           // Where all the data starts
bool               problem;            // Global error flag
bool               stop;               // Global closedown flag
int                errcnt;             // Number of times ECB invoked
static const int   X = -1;             // Cosmic error table entry
static const int   R = -2;             // Cosmic return table entry
Lex                Lx;                 // Lexer
Lex::tokdat        Td;                 // Current lexer token
Node *             pSect;              // Current section
Node *             pRecd;              // Current record
string             fname;              // Source name
FILE *             ofp;                // Output stream
FILE *             sf;                 // Pretty print stream
static const string s_;
struct cb_struct {                     // Callback holding structure
  void (* com_cb)(void *,void *,void *);      // Command method
  void * com_pt;                       // Command method object
  void (* err_cb)(void *,void *,int);         // Error method
  void * err_pt;                       // Error method object
  void (* rec_cb)(void *,void *,void *);      // Record method
  void * rec_pt;                       // Record method object
  void (* sec_cb)(void *,void *,bool,void *); // Section method
  void * sec_pt;
} cb;
class NodeHeap;                        // Internal memory manager for nodes
NodeHeap *         pNH;
map<string,string> argMap;             // Preprocessor string map            

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// A little foray into subclassing - not something I approve of, as a rule, but
// horses for courses...

class Node {
public:
                   Node(int=0,Notype=No_0000,const string & =s_,
                        Lex::Sytype=Lex::S_00);
virtual ~          Node(void);
void               Add(Node *);        // Bolt on a new child
void               Args(UIF *);        // Any preprocessor substitutions
static void        Disconnect(Node * &);
void               Dump(FILE * =stdout,string=s_);
void               Dumpt(FILE * =stdout);
void               Dumpx(FILE * =stdout,string=s_);
bool               IsEx();             // Is this an Node expression ?
bool               IsOp();             // Is this a Node operator? (!= UIF op)
bool               IsLB();             // Is this a Node '('?
bool               IsRB();             // Is this a Node ')'?
Node * &           L();                // Pretend the n-ary tree is binary
Node * &           P();
Node * &           R();
void               Src(int &,int &);   // Return token source position
void               Sub(UIF *);         // Hack off a tree branch
Notype             Type();             // Get the Node type
Notype             Type(unsigned &);   // ... and leaf size (i.e. empty?)
void               Type(Notype);       // Set the Node type

Notype             typ;                // Node type
Lex::Sytype        qop;                // Node operator (!= UIF type)
string             str;                // Node string
int                pos;                // Location in file
vector <Node *>    leaf;               // Children
Node *             par;                // Parent
void *             tag;                // In case anyone needs it...
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Internal memory manager for nodes - saves the system heap manager a shed-load
// of work, because all we do is handle Node objects, so we know what size
// everything is up front.

class NodeHeap {
public:
                   NodeHeap(){maxcnt = 0;}
virtual ~          NodeHeap();
void               delete_Node(Node *);
unsigned           Maxcnt() { return maxcnt; }
Node *             new_Node(int=0,Notype=No_0000,const string & =s_,
                            Lex::Sytype=Lex::S_00);
private:
vector<Node *>     NodeVector;      // Local heap manager 'unused' store
unsigned           maxcnt;          // How many Node objects I have
};

// End of subclass declares
};

//==============================================================================

#endif

#ifndef __jsonParser__H
#define __jsonParser__H

#include "Xlex.h"
#include "macros.h"
#include <vector>
using namespace std;

//==============================================================================

class jsonParser
{
public:
struct           node;
                 jsonParser();
virtual ~        jsonParser(void);
  /*
virtual bool     CDATA(             const void *,
                                    const unsigned &,
                                    const string &);
virtual bool     Comments(          const void *,
                                    const string &);
virtual bool     EndDocument(       const void *);
virtual bool     EndElement(        const void *,
                                    const string &); */
virtual bool     Error(             const void *,
                                    const unsigned &,
                                    const unsigned &,
                                    const string &);
void             ExposeErrs();                         /*
virtual bool     JSON(              const void *,
                                    const string &,
                                    const vector<pair<string,string> > &);  */
node *           AddNode();
void             Parse(FILE *);
void             Value();          /*
bool             SetPtr(            const void *);
virtual bool     StartDocument(     const void *,
                                    const string &,
                                    const vector<pair<string,string> > &);
virtual bool     StartElement(      const void *,
                                    const string &,
                                    const vector<pair<string,string> > &);
protected:
struct                       calpha_t;
void                         Attr();
calpha_t                     Calpha();  */
void                         Dump(FILE * = stdout);      /*
void                         Element();
bool                         Json(int &);

vector<calpha_t>             elename;  // Element name stack
vector<pair<string,string> > attr;
vector<pair<string,string> > json;
string                       jsonstr;
string                       xmlname;
string                       cdata;
string                       comments; */
bool                         problem;
struct err_t {
  err_t(unsigned _r,unsigned _c,string _s):row(_r),col(_c),sym(_s){}
  unsigned row;
  unsigned col;
  string sym;
};
vector<err_t> err_v;                     /*
struct calpha_t {
  calpha_t(string _R,string _S):R(_R),S(_S){}
  string c_str() {return R+(S.empty()?string():(":"+S));}
  operator!=(calpha_t X) { return ((X.R!=R)||(X.S!=S)); }
  string R;
  string S;
};
    */

struct node {
  node():par(0){}
  ~node(void) { WALKVECTOR(node *,node_v,i) delete(*i); }
  void Dump(FILE *,int=0);
  string jkey;
  char   jtype;
  string jdata;
  vector<node *> node_v;
  node * par;
};
node *                       root;
node *                       par;

void *                       phome;    // Tag for interface routines

static const int             X = -1;   // Cosmic exit table entry
static const int             R = -2;   // Cosmic return table entry
static const int             E = -3;   // Cosmic error table entry
Lex                          Lx;       // Lexer
Lex::tokdat                  Td;       // Current lexical token
};

//==============================================================================

#endif

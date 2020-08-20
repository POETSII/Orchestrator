#ifndef __XMLParser__H
#define __XMLParser__H

#include "lex.h"
#include <map>
#include <vector>
#include "OSFixes.hpp"
using namespace std;

//==============================================================================

class xmlP
{
public:
             xmlP();
virtual ~    xmlP(void);

virtual bool CDATA(const void *,const unsigned &,const string &);
virtual bool Comments(const void *,const string &);
void         Debug(unsigned f = 0){ dflag = f; }
virtual bool EndDocument(const void *);
virtual bool EndElement(const void *,const string &);
unsigned     ErrCnt();
virtual bool Error(const void *,const unsigned &,const unsigned &,
                   const unsigned &,const string &);
virtual bool JSON(const void *,const string &,
                  const vector<pair<string,string> > &);
void         Parse(FILE *);
void         SetOChan(FILE * _fp){ fo = _fp; }
bool         SetPtr(const void *);
virtual bool StartDocument(const void *,const string &,
                           const vector<pair<string,string> > &);
virtual bool StartElement(const void *,const string &,
                          const vector<pair<string,string> > &);
protected:
struct       calpha_t;
void         Attr();
calpha_t     Calpha();
void         Dump(unsigned = 0,FILE * = stdout);
void         Element();
string       GetString(Lex::Sytype);
bool         Json(int &); 

vector<calpha_t>             elename;  // Element name stack
vector<pair<string,string> > attr;
vector<pair<string,string> > json;
string                       jsonstr;
string                       xmlname;
string                       cdata;
string                       comments;
unsigned                     dflag;
unsigned                     problem;
struct err_t {
  err_t(unsigned _e,unsigned _r,unsigned _c,string _s):
        err(_e),row(_r),col(_c),sym(_s){}
  void Dump(FILE * fp = stdout)
   { fprintf(fp,"%u:(%u,%u) : %s\n",err,row,col,sym.c_str()); }
  unsigned err;
  unsigned row;
  unsigned col;
  string sym;
};
vector<err_t> err_v;
struct calpha_t {
  calpha_t(string _R,string _S):R(_R),S(_S){}
  string str() {return R+(S.empty()?string():(":"+S));}
  bool operator!=(calpha_t X) { return ((X.R!=R)||(X.S!=S)); }
  string R;
  string S;
};

void *                       phome;    // Tag for interface routines
FILE *                       fo;       // Text output stream
static const int             X = -1;   // Cosmic exit table entry
static const int             R = -2;   // Cosmic return table entry
static const int             E = -3;   // Cosmic error table entry
Lex                          Lx;       // Lexer
Lex::tokdat                  Td;       // Current lexical token
map<unsigned,string>         emap;     // Error messages

};

//==============================================================================

#endif

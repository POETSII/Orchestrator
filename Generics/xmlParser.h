#ifndef __XMLParser__H
#define __XMLParser__H

#include "Xlex.h"
#include <vector>
using namespace std;

//==============================================================================

class xmlParser {
public:
                 xmlParser(FILE *);
virtual ~        xmlParser(void);

virtual bool     CDATA(             const void *,
                                    const unsigned &,
                                    const string &);
virtual bool     Comments(          const void *,
                                    const string &);
virtual bool     EndDocument(       const void *);
virtual bool     EndElement(        const void *,
                                    const string &);
virtual bool     Error(             const void *,
                                    const unsigned &,
                                    const unsigned &,
                                    const string &);
virtual bool     JSON(              const void *,
                                    const string &,
                                    const vector<pair<string,string> > &);
bool             SetPtr(            const void *);
virtual bool     StartDocument(     const void *,
                                    const string &,
                                    const vector<pair<string,string> > &);
virtual bool     StartElement(      const void *,
                                    const string &,
                                    const vector<pair<string,string> > &);
private:
void                         Attr();
void                         Dump();
void                         Element();
bool                         Json(int &);

vector<string>               elename;  // Element name stack
vector<pair<string,string> > attr;
vector<pair<string,string> > json;
string                       jsonstr;
string                       xmlname;
string                       cdata;
string                       comments;
bool                         problem;
void *                       phome;

static const int             X = -1;   // Cosmic exit table entry
static const int             R = -2;   // Cosmic return table entry
static const int             E = -3;   // Cosmic error table entry
Lex                          Lx;       // Lexer
Lex::tokdat                  Td;
};

//==============================================================================

#endif

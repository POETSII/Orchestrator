#ifndef __xmlTreeDump__H
#define __xmlTreeDump__H

#include "xmlParser.h"
#include <vector>
using namespace std;

//==============================================================================

class xmlTreeDump : public xmlParser
{
public:
                 xmlTreeDump();
virtual ~        xmlTreeDump(void);

bool     CDATA(const void *,const unsigned &,const string &);
bool     Comments(const void *,const string &);
void     Dump(FILE * = stdout);
bool     EndDocument(const void *);
bool     EndElement(const void *,const string &);
bool     Error(const void *,const unsigned &,const unsigned &,const unsigned &,
               const string &);
bool     JSON(const void *,const string &,const vector<pair<string,string> > &);
void     Show(FILE * = stdout);
bool     StartDocument(const void *,const string &,
                       const vector<pair<string,string> > &);
bool     StartElement(const void *,const string &,
                      const vector<pair<string,string> > &);
unsigned Transform(FILE *);

private:

struct stats_t {
  stats_t():lines(0),elems(0),cmnts(0),cfrgs(0){}
  void Show(FILE * fp) { fprintf(fp,"\n%4u source lines\n%4u XML elements\n"
                                    "%4u comments\n%4u C fragments\n",
                                    lines,elems,cmnts,cfrgs); }
  unsigned lines;
  unsigned elems;
  unsigned cmnts;
  unsigned cfrgs;
} stats;

struct node {
  node();
  ~node(void);
  void Show(FILE * = stdout,int off=0);
  string ename;
  vector<pair<string,string> > attr;
  vector<node *> vnode;
  string cfrag;
  int lin,col;
  node * par;
};

node *   t_root;                       // Root of tag tree
node *   cur;                          // "Current" growth point in tree
node *   AddNode();
};

//==============================================================================

#endif

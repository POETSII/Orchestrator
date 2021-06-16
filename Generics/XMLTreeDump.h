#ifndef __xmlTreeDump__H
#define __xmlTreeDump__H

#include "xmlP.h"
#include <vector>
using namespace std;

//==============================================================================

class xmlTreeDump : public xmlP
{
public:
struct               node;
                     xmlTreeDump();
virtual ~            xmlTreeDump(void);

bool                 CDATA(const void *,const unsigned &,const string &);
bool                 Comments(const void *,const string &);
void                 Dump(unsigned = 0,FILE * = stdout);
bool                 Empty() { return t_root==0; }
bool                 EndDocument(const void *);
bool                 EndElement(const void *,const string &);
bool                 Error(const void *,const unsigned &,const unsigned &,
                           const unsigned &,const string &);
bool                 JSON(const void *,const string &,
                          const vector<pair<string,string> > &);
string               Name() { return fname; }
void                 Name(string s) { fname = s; }
node *               Root() { return t_root; }
void                 ShowP(FILE * = stdout);
void                 ShowV(FILE * = stdout);
bool                 StartDocument(const void *,const string &,
                                   const vector<pair<string,string> > &);
bool                 StartElement(const void *,const string &,
                                  const vector<pair<string,string> > &);
unsigned             Transform(FILE *);

struct stats_t {
  stats_t():lines(0),elems(0),cmnts(0),cfrgs(0){}
  void Show(FILE * fp) { fprintf(fp,"%4u source lines\n%4u XML elements\n"
                                    "%4u comments\n%4u C fragments\n\n",
                                    lines,elems,cmnts,cfrgs); }
  unsigned lines;
  unsigned elems;
  unsigned cmnts;
  unsigned cfrgs;
} stats;

struct node {                          // Generic XML tree node
         node();
        ~node(void);
  void   Dump(FILE * = stdout);        // NON-RECURSIVE
  string FindAttr(string);             // Locate named attribute
  node * FindChild(string);            // Locate named child
  string FullName();
  static string Prettify(string,unsigned = 30);  // Copy, truncate and decorate
  int &  rTyp() { return col; }
                                       // Pretty-printer and string generator
  void   Show(FILE * = stdout,string = string());
  void   TreeStrings(string &,string &,string &,string &);

  string ename;                        // Element name
  vector<pair<string,string> > attr;   // Ordered attribute list
  vector<node *> vnode;                // Child nodes
  string cfrag;                        // Any C-frag (not really XML?)
  int    lin,col;                      // Source coordinates
  node * par;                          // Parent node
  void * tag;                          // Polymorphism? Life's too short.
  typedef void (*pf)(FILE *,void *,const char *);   // Callback to dump tag
  static void (* tag_cb)(FILE *,void *,const char *);                            // Pointer
  static void SetTag_cb(void (*f)(FILE *,void *,const char *)=0){tag_cb=f;}      // Set
  void ExeTag_cb(FILE * fp,const char * s) {if(tag_cb!=0) (node::tag_cb)(fp,tag,s);} // Execute
};

string   fname;                        // My filename
node *   t_root;                       // Root of tag tree
node *   cur;                          // "Current" growth point in tree
node *   AddNode();
};

//==============================================================================

#endif

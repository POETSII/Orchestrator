#ifndef __StringTreeH__H
#define __StringTreeH__H

#include <string>
#include <vector>
#include <map>
using namespace std;

//==============================================================================

class StringTree
{
public:
class                 Node;
                      StringTree();
                      StringTree(string *);
                      StringTree(StringTree &);
virtual ~             StringTree(void);
void *                Add(string,int=0,void * = 0);
void *                Add(void *,string,int=0,void * =0);
void                  Clear();
void                  Dump();
void                  Get(void *,string &,int &,void *&);
void                  Get(void *,void(*)(StringTree *,void *));
StringTree::Node *    Root();
void                  Stream(string *);
void                  UnStream(string &);

private:
string                STint2str(int);
int                   STstr2int(string &,unsigned &);
string                STstr2str(string &,unsigned &);
unsigned              STstr2unsigned(string &,unsigned &);
void *                STstr2voidp(string &,unsigned &);
string                STunsigned2str(unsigned);
string                STvoidp2str(void *);
void                  WalkStream(Node *);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Subclass StringTree::Node
public:
class Node {
friend class          StringTree;
public :
                      Node(StringTree *,string,int,void *);
virtual ~             Node(void);
void                  Dump(int=0);
void                  WalkTree(void *,void(*)(void *,Node *));
string                str;
int                   image;
Node *                par;
vector<Node *>        child;
int                   cnt;
void *                pdata;
StringTree *          pST;
};
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

friend class Node;
static const unsigned BPP         = sizeof(void *);
static const unsigned BPI         = sizeof(int);
static const unsigned BPU         = sizeof(unsigned);
Node *                root;
int                   count;
unsigned              nfc;
map<Node *,unsigned>  smap;
map<unsigned,Node *>  umap;
string *              pstream;
union bodgeU;
friend union bodgeU;
union bodgeU {
  unsigned u;
  int      i;
  void *   p;
  char     cc[BPP];
} bodge;

};

//==============================================================================

#endif

//------------------------------------------------------------------------------

#include "stringtree.h"
#include "dfprintf.h"
#include "macros.h"
#include <stdio.h>

//==============================================================================

StringTree::StringTree()
// Empty constructor
{
root = 0;
Clear();                               // Guess
}

//------------------------------------------------------------------------------

StringTree::StringTree(string * ps)
// Constructor from string
{
root = 0;
UnStream(*ps);
}

//------------------------------------------------------------------------------

StringTree::StringTree(StringTree & rs)
// Copy constructor
{
root = 0;
string buffer;
rs.Stream(&buffer);
UnStream(buffer);                      // And why not?
}

//------------------------------------------------------------------------------

StringTree::~StringTree(void)
// Recursive deletion of node tree
{
if (root!=0) delete root;
}

//------------------------------------------------------------------------------

void * StringTree::Add(string str,int im,void * pd)
// Create the root image/string node
{
if (root!=0) delete root;              // Trash any existing
root = new Node(this,str,im,pd);       // New root node
return (void *)root;                   // And out it goes
}

//------------------------------------------------------------------------------

void * StringTree::Add(void * n,string str,int im,void * pd)
// Add a child node to "n", return the new node address
{
Node * pN = new Node(this,str,im,pd);  // New node
Node * pn = static_cast<Node *>(n);    // Type-force putative parent
pn->child.push_back(pN);               // Parent->child link
pN->par = pn;                          // Child->parent link
return (void *)pN;
}

//------------------------------------------------------------------------------

void StringTree::Clear()
{
if (root!=0) delete root;              // Lose any tree
root  = 0;                             // No tree root
count = 0;                             // No nodes ... obviously
pstream = 0;                           // No streaming string
smap.clear();                          // No streaming map
umap.clear();                          // No unstreaming map
nfc = 0;                               // Next free byte pointer
}

//------------------------------------------------------------------------------

void StringTree::Dump()
{
printf("================\n");
printf("StringTree dump:\n");
printf("BytesPerPointer  = %u\n",BPP); // Bytes per pointer
printf("BytesPerIinteger = %u\n",BPI); // Bytes per integer
printf("BytesPerUnsigned = %u\n",BPU); // Bytes per unsigned
printf("count            = %d\n",count);
printf("root             = %#010x\n",root);
if (root!=0) root->Dump();
printf("nfc              = %u\n",nfc);
printf("\nStream map:\n");
WALKMAP(Node *,unsigned,smap,i) printf("%#010x : %010u\n",(*i).first,(*i).second);
printf("\nUnstream map:\n");
WALKMAP(unsigned,Node *,umap,i) printf("%010u : %#010x\n",(*i).first,(*i).second);
printf("\n");
printf("================\n");
}

//------------------------------------------------------------------------------

void StringTree::Get(void * p,string & rs,int & ri,void *& rpv)
// Routine to extract the data from a node
{
Node * pN = static_cast<Node *>(p);    // Node handle
if (pN==0) pN = root;                  // 0 => use root
if (pN==0) return;                     // No root => no tree
rs  = pN->str;
ri  = pN->image;
rpv = pN->pdata;
}

//------------------------------------------------------------------------------

void StringTree::Get(void * p,void(*pcb)(StringTree*,void *))
// Routine to walk the children of a node, passing each handle to the callback
{
if (pcb==0) return;                    // No function to call
Node * pN = static_cast<Node *>(p);    // Node handle
if (pN==0) pN = root;                  // 0 => use root
if (pN==0) return;                     // No root => no tree
WALKVECTOR(Node *,pN->child,i) pcb(this,*i);
}

//------------------------------------------------------------------------------

StringTree::Node * StringTree::Root()
// I gave in with encapsulation bit. This gives the user-side complete access
// to the tree. Don't break it.
{
return root;
}

//------------------------------------------------------------------------------

void StringTree::Stream(string * buffer)
// Write the entire data structure out into a linear string, so it can be
// passed around (and even sent via MPI...)
{
if (buffer==0) return;                 // Empty call
if (root==0) return;                   // If we're empty, nothing to do
buffer->clear();                       // If it wasn't empty, it should have bin
pstream = buffer;
smap.clear();                          // Clear the streaming address map
nfc = 0;                               // Next free cell in buffer
WalkStream(root);                      // Build string
pstream = 0;                           // Disconnect it 
}

//------------------------------------------------------------------------------

string StringTree::STint2str(int i)
// Turns an integer into a byte string
{
bodge.i = i;                           // Copy value to the bodging union
string s;
s.assign(bodge.cc,BPI);                // Have to use assign() 'cos +=
return s;                              // truncates at the first '\0'
}

//------------------------------------------------------------------------------

int StringTree::STstr2int(string & s,unsigned & n)
// Turns a substring (starting at position n) string into an integer
{
                                       // Copy the bytes
for(unsigned i=n;i<n+BPI;i++) bodge.cc[i-n] = s[i];
n += BPI;                              // Increment the next free byte pointer
return bodge.i;
}

//------------------------------------------------------------------------------

string StringTree::STstr2str(string & s,unsigned & n)
// Pulls out a substring and increments the next free byte pointer.
// In the buffer, the substring is preceded by an unsigned length.
{
unsigned len = STstr2unsigned(s,n);    // Length of substring
string a = s.substr(n,len);            // Hoik it out
n += a.size();                         // Increment pointer
return a;
}

//------------------------------------------------------------------------------

unsigned StringTree::STstr2unsigned(string & s,unsigned & n)
// Turns a substring (starting at position n) string into an unsigned
{
for(unsigned i=n;i<n+BPU;i++) bodge.cc[i-n] = s[i];
n += BPU;
return bodge.u;
}

//------------------------------------------------------------------------------

void * StringTree::STstr2voidp(string & s,unsigned & n)
// Turns a substring (starting at position n) string into a void *
{
for(unsigned i=n;i<n+BPP;i++) bodge.cc[i-n] = s[i];
n += BPP;
return bodge.p;
}

//------------------------------------------------------------------------------

string StringTree::STunsigned2str(unsigned u)
// Turns an unsigned into a byte string
{
bodge.u = u;
string s;
s.assign(bodge.cc,BPU);
return s;
}

//------------------------------------------------------------------------------

string StringTree::STvoidp2str(void * p)
// Turns a void * into a byte string
{
bodge.p = p;
string s;
s.assign(bodge.cc,BPP);
return s;
}

//------------------------------------------------------------------------------

void StringTree::UnStream(string & s)
// Unpack - in a single pass - the tree structure packed into the string.
{
Clear();                               // Clear any existing data structure
while (nfc<s.size()) {                 // Walk the string
  unsigned tmp    = nfc;               // Save the start byte position
  unsigned par_   = STstr2unsigned(s,nfc);       // Parent offset
  int      image_ = STstr2int(s,nfc);            // Tree data
  void *   pdata_ = STstr2voidp(s,nfc);
  string   str_   = STstr2str(s,nfc);
  void * pN;                           // Create the new node
  if (umap.empty()) pN = Add(str_,image_,pdata_);// New root
  else pN = Add(umap[par_],str_,image_,pdata_);  // New not-root
  umap[tmp] = (Node *)pN;              // Map the start byte to the node
}

}

//------------------------------------------------------------------------------

void StringTree::WalkStream(Node * pN)
// Recursive walk to pack the stream string
{
smap[pN] = nfc;                        // Map the node to its start byte
*pstream += STunsigned2str(smap[pN->par]);       // Write the offset
*pstream += STint2str(pN->image);                // Write the node data
*pstream += STvoidp2str(pN->pdata);
*pstream += STunsigned2str(pN->str.size());
*pstream += pN->str;
nfc = pstream->size();                           // Update the next free byte
WALKVECTOR(Node *,pN->child,i) WalkStream(*i);   // Down we go....
}

//==============================================================================

StringTree::Node::Node(StringTree * p,string s,int im,void * pd)
{
str    = s;                            // String
image  = im;                           // Image index
par    = 0;                            // Parent node
cnt    = p->count++;                   // Node count
pdata  = pd;                           // Hanger for outside use
pST    = p;                            // Enclosing object
}

//------------------------------------------------------------------------------

StringTree::Node::~Node()
{
WALKVECTOR(Node *,child,i) delete *i;
}

//------------------------------------------------------------------------------

void StringTree::Node::Dump(int off)
{
string B;
for (int i=0;i<off;i++) dprintf(B,"  ");
dprintf(B,"this = %#010x, parent = %#010x, pdata = %#010x, image = %3d, string = %s\n",
         this,par,pdata,image,str.c_str());
printf("%s",B.c_str());
WALKVECTOR(Node *,child,i)
  if ((*i)->par!=this) printf("Node child->par link fault\n");
WALKVECTOR(Node *,child,i) (*i)->Dump(off+2);
}

//==============================================================================

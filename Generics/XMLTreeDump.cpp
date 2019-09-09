//---------------------------------------------------------------------------

#include "xmlTreeDump.h"
#include <algorithm>                   // For ::tolower
#include "macros.h"

//==============================================================================

xmlTreeDump::xmlTreeDump() : xmlParser()
//
{
t_root = new node();                   // Put the root tree node on the stack
}

//------------------------------------------------------------------------------

xmlTreeDump::~xmlTreeDump()
//
{
delete t_root;                         // Kill the tree
}

//------------------------------------------------------------------------------

bool xmlTreeDump::CDATA(const void * p,const unsigned & type,const string & s)
{
stats.cfrgs++;
Lx.GetLC(cur->lin,cur->col);
cur->cfrag = s;
if (dflag!=0)fprintf(fo,"\nxmlTreeDump::CDATA derived from...");
return xmlParser::CDATA(p,type,s);
}

//------------------------------------------------------------------------------

bool xmlTreeDump::Comments(const void * p,const string & s)
{
stats.cmnts++;
if (dflag!=0)fprintf(fo,"\nxmlTreeDump::Comments derived from...");
return xmlParser::Comments(p,s);
}

//------------------------------------------------------------------------------

void xmlTreeDump::Dump(FILE * fp)
{
fprintf(fp,"xmlTreeDmp+++++++++++++++++++++++++++++++++++++++++\n");
Show(fp);
xmlParser::Dump(fp);
fprintf(fp,"xmlTreeDump-----------------------------------------\n");
fflush(fp);
}

//------------------------------------------------------------------------------

bool xmlTreeDump::EndDocument(const void * p)
{
Lx.GetLC(cur->lin,cur->col);
stats.lines = (unsigned)(cur->lin);
if (dflag!=0)fprintf(fo,"\nxmlTreeDump::EndDocument derived from...");
return xmlParser::EndDocument(p);
}

//------------------------------------------------------------------------------

bool xmlTreeDump::EndElement(const void * p,const string & s)
// Make the parent of the current node the new start point
{
Lx.GetLC(cur->lin,cur->col);
if (dflag!=0)fprintf(fo,"\nxmlTreeDump::EndElement derived from...");
cur = cur->par;
return xmlParser::EndElement(p,s);
}

//------------------------------------------------------------------------------

bool xmlTreeDump::Error(const void * p,const unsigned & e,const unsigned & r,
                        const unsigned & c,const string & s)
{
if (dflag!=0)fprintf(fo,"\nxmlTreeDump::Error derived from...");
return xmlParser::Error(p,e,r,c,s);
}

//------------------------------------------------------------------------------

bool xmlTreeDump::JSON(const void * p,const string & s,
                     const vector<pair<string,string> > & vps)
{
if (dflag!=0)fprintf(fo,"\nxmlTreeDump::JSON derived from...");
return xmlParser::JSON(p,s,vps);
}

//------------------------------------------------------------------------------

void xmlTreeDump::Show(FILE * fp)
{
t_root->Show(fp);
stats.Show(fp);
fflush(fp);
}

//------------------------------------------------------------------------------

bool xmlTreeDump::StartDocument(const void * p,const string & s,
                              const vector<pair<string,string> > & vps)
{
Lx.GetLC(cur->lin,cur->col);
if (dflag!=0)fprintf(fo,"\nxmlTreeDump::StartDocument derived from...");
return xmlParser::StartDocument(p,s,vps);
}

//------------------------------------------------------------------------------

bool xmlTreeDump::StartElement(const void * p,const string & s,
                               const vector<pair<string,string> > & vps)
// Add a new element to the tag tree
{
stats.elems++;
node * n = AddNode();                  // Housekeeping pointers
n->ename = s;                          // Tag name
n->attr = vps;
Lx.GetLC(n->lin,n->col);
if (dflag!=0)fprintf(fo,"\nxmlTreeDump::StartElement derived from...");
xmlParser::StartElement(p,s,vps);
return true;
}

//------------------------------------------------------------------------------

unsigned xmlTreeDump::Transform(FILE * fp)
// Initialise the TreeDumper datastructure (i.e. cur = ...) and then call the
// base class XML parser. The derived class methods catch everything and build
// the xmlTreeDump element tree.
{
cur = t_root;                          // Current parent starts as root
Parse(fp);                             // And build the element tree
unsigned ecnt = ExposeErrs();          // Base class method - it's generic
Show(fo);
return ecnt;
}

//==============================================================================
// xmlTreeDump PRIVATE METHODS

xmlTreeDump::node * xmlTreeDump::AddNode()
// Routine to create an add in a new node to the tag tree
{
node * n = new node();
cur->vnode.push_back(n);
n->par = cur;
cur = n;
return n;
}

//==============================================================================
// SUBCLASS node METHODS

xmlTreeDump::node::node()
{
ename = "NoName";
lin = 0;
col = 0;
par = 0;
}

//------------------------------------------------------------------------------

xmlTreeDump::node::~node(void)
{
WALKVECTOR(node *,vnode,i) delete *i;
}

//------------------------------------------------------------------------------

void xmlTreeDump::node::Show(FILE * fp,int off)
{
string B(off,' ');                     // Create leading space indent
string s2 = '\n' + string(off+10+10,' ') + string("|| ");
                                       // Print this node
fprintf(fp,"(%4d,%3d)%s[E] %s\n",lin,col,B.c_str(),ename.c_str());
for(vector<pair<string,string> >::const_iterator i=attr.begin();
    i!=attr.end();i++)
  fprintf(fp,"(%4d,%3d)%s  [A] %s = %s\n",
          lin,col,B.c_str(),(*i).first.c_str(),(*i).second.c_str());
if (!cfrag.empty()) {
  for(unsigned j=0;j<cfrag.size();j++) {
    if (cfrag[j]=='\n') {
      cfrag.replace(j,1,s2);           // Stick a load of leading spaces in
      j+=s2.size()-1;
    }
  }
  cfrag = s2 + cfrag;
  fprintf(fp,"(%4d,%3d)%s  [C]     "
             "[[------------------------------------------------"
             "%s\n%s"
             "                    "
             "------------------------------------------------]]\n",
          lin,col,B.c_str(),cfrag.c_str(),B.c_str());
}
                                       // Pretty-print the children
WALKVECTOR(node *,vnode,i)(*i)->Show(fp,off+2);
}

//==============================================================================


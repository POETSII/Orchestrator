//------------------------------------------------------------------------------

#include "XMLTreeDump.h"
#include <algorithm>                   // For ::tolower
#include "macros.h"
#include <stdint.h>

//==============================================================================

xmlTreeDump::xmlTreeDump() : xmlP()
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
// CDATA rates it's own tree node.......
{
stats.cfrgs++;
node * n = AddNode();                  // Housekeeping pointers
n->ename = "CDATA";                    // Tag name
n->cfrag = s;                          // Store the CFRAG
cur = n->par;                          // CDATA is ALWAYS a leaf.....
Lx.GetLC(n->lin,n->col);
if (dflag!=0)fprintf(fo,"\nxmlTreeDump::CDATA derived from...");
xmlP::CDATA(p,type,s);
return true;

/*
stats.cfrgs++;
Lx.GetLC(cur->lin,cur->col);
cur->cfrag = s;
if (dflag!=0)fprintf(fo,"\nxmlTreeDump::CDATA derived from...");
return xmlP::CDATA(p,type,s);
*/
}

//------------------------------------------------------------------------------

bool xmlTreeDump::Comments(const void * p,const string & s)
{
stats.cmnts++;
if (dflag!=0)fprintf(fo,"\nxmlTreeDump::Comments derived from...");
return xmlP::Comments(p,s);
}

//------------------------------------------------------------------------------

void xmlTreeDump::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n%sxmlTreeDmp++++++++++++++++++++++++++++++++++++++++++++++\n",os);
ShowV(fp);
xmlP::Dump(off+2,fp);
fprintf(fp,"%sxmlTreeDump---------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

bool xmlTreeDump::EndDocument(const void * p)
{
Lx.GetLC(cur->lin,cur->col);
stats.lines = (unsigned)(cur->lin);
if (dflag!=0)fprintf(fo,"\nxmlTreeDump::EndDocument derived from...");
return xmlP::EndDocument(p);
}

//------------------------------------------------------------------------------

bool xmlTreeDump::EndElement(const void * p,const string & s)
// Make the parent of the current node the new start point
{
Lx.GetLC(cur->lin,cur->col);
if (dflag!=0)fprintf(fo,"\nxmlTreeDump::EndElement derived from...");
cur = cur->par;
return xmlP::EndElement(p,s);
}

//------------------------------------------------------------------------------

bool xmlTreeDump::Error(const void * p,const unsigned & e,const unsigned & r,
                        const unsigned & c,const string & s)
{
if (dflag!=0)fprintf(fo,"\nxmlTreeDump::Error derived from...");
return xmlP::Error(p,e,r,c,s);
}

//------------------------------------------------------------------------------

bool xmlTreeDump::JSON(const void * p,const string & s,
                     const vector<pair<string,string> > & vps)
{
if (dflag!=0)fprintf(fo,"\nxmlTreeDump::JSON derived from...");
return xmlP::JSON(p,s,vps);
}

//------------------------------------------------------------------------------

void xmlTreeDump::ShowP(FILE * fp)
{
fprintf(fp,"XML file %s\n",Name().c_str());
stats.Show(fp);
}

//------------------------------------------------------------------------------

void xmlTreeDump::ShowV(FILE * fp)
{
fprintf(fp,"XML file %s\n",Name().c_str());
t_root->Show(fp);
fprintf(fp,"\n");
ShowP(fp);
}

//------------------------------------------------------------------------------

bool xmlTreeDump::StartDocument(const void * p,const string & s,
                                const vector<pair<string,string> > & vps)
// Tree root name is already set to the relevant filename. The argument s
// above is the "xml" from <?xml version="1.0"?>, and I don't actually do
// anything with it.
{
Lx.GetLC(cur->lin,cur->col);
if (dflag!=0)fprintf(fo,"\nxmlTreeDump::StartDocument derived from...");
return xmlP::StartDocument(p,s,vps);
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
xmlP::StartElement(p,s,vps);
return true;
}

//------------------------------------------------------------------------------

unsigned xmlTreeDump::Transform(FILE * fp)
// Initialise the TreeDumper datastructure (i.e. cur = ...) and then call the
// base class XML parser. The derived class methods catch everything and build
// the xmlTreeDump element tree.
{
t_root->ename = fname;                 // Embed filename into tree
cur = t_root;                          // Current parent starts as root
Parse(fp);                             // And build the element tree
return ErrCnt();                       // Base class method - it's generic
}

//==============================================================================
// xmlTreeDump PRIVATE METHODS

xmlTreeDump::node * xmlTreeDump::AddNode()
// Routine to create and add in a new node to the tag tree
{
node * n = new node();
cur->vnode.push_back(n);
n->par = cur;
cur = n;
return n;
}

//==============================================================================
// SUBCLASS node METHODS (and static declares)

xmlTreeDump::node::pf xmlTreeDump::node::tag_cb = 0;

//==============================================================================

xmlTreeDump::node::node()
{
ename = "NoName";
lin   = 0;
col   = 0;
par   = 0;
tag   = 0;
}

//------------------------------------------------------------------------------

xmlTreeDump::node::~node(void)
{
WALKVECTOR(node *,vnode,i) delete *i;
}

//------------------------------------------------------------------------------

void xmlTreeDump::node::Dump(FILE * fp)
// Non-recursive developer-facing local node dump
{
fprintf(fp,"\nxmlTreeDump::node dump+++++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"ename               = %s\n",ename.c_str());
fprintf(fp,"child node count    = %lu\n",vnode.size());
fprintf(fp,"lin, col            = %d, %d\n",lin,col);
fprintf(fp,"CFRAG size          = %lu\n",cfrag.size());
fprintf(fp,"||%s||\n",Prettify(cfrag).c_str());
fprintf(fp,"this                = %p\n",(void *)this);
fprintf(fp,"parent, tag         = %p, %p\n",(void *)par,(void *)tag);
fprintf(fp,"attribute list size = %lu\nattributes:\n",attr.size());
for(vector<pair<string,string> >::const_iterator i=attr.begin();
    i!=attr.end();i++)
  fprintf(fp,"  %s = %s\n",(*i).first.c_str(),(*i).second.c_str());
fprintf(fp,"xmlTreeDump::node dump-------------------------------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

string xmlTreeDump::node::FindAttr(string s)
// Locate value of named attribute (empty string is OK)
{
for(unsigned i=0;i<attr.size();i++)
  if (attr[i].first==s) return attr[i].second;
return string();
}

//------------------------------------------------------------------------------

xmlTreeDump::node * xmlTreeDump::node::FindChild(string s)
// Locate named child
{
WALKVECTOR(node *,vnode,i) if ((*i)->ename==s) return (*i);
return 0;
}

//------------------------------------------------------------------------------

string xmlTreeDump::node::FullName()
// Return the full hierarchical node name
{
if (par==0) return ename;              // Unless I'm the root, that is
return par->FullName() + "." + ename;
}

//------------------------------------------------------------------------------

string xmlTreeDump::node::Prettify(string s,unsigned len)
{
s.resize(len,'.');
for (unsigned i=0;i<s.size();i++)
if (s[i]=='\n') s.replace(i,1,"<c/r>");
return s;
}

//------------------------------------------------------------------------------

void xmlTreeDump::node::Show(FILE * fp,string s0)
// This is used both by the user-facing ShowV() AND Dump(). There's a switch.
// Messy, I accept, but the thing recurses, and it's hard (i.e. I can't) to
// think of a better way.
// If there's a tag callback defined (node::tag_cb != 0 ?) then we get the
// callback on the tag - obviously - but ALSO we decorate the output with the
// node addresses
{
string sh,sb,st;
TreeStrings(s0,sh,sb,st);
                                       // Print this node
const char * csh = sh.c_str();
const char * csb = sb.c_str();
string s2 = '\n' + sh + "\\ ";
fprintf(fp,"(%4d,%3d)%s[E] %s\n",lin,col,csh,FullName().c_str());
if (tag_cb==0) fprintf(fp,"\n");       // Dump or Show ?
else fprintf(fp,"          %s(me:%" PTR_FMT ",par:%" PTR_FMT ")\n",csb,
            OSFixes::getAddrAsUint(this),OSFixes::getAddrAsUint(par));
ExeTag_cb(fp,csb);                     // Tag callback
                                       // Now the attributes
for(vector<pair<string,string> >::const_iterator i=attr.begin();
    i!=attr.end();i++)
  fprintf(fp,"(%4d,%3d)%s[A] %s = \"%s\"\n",
          lin,col,csb,(*i).first.c_str(),(*i).second.c_str());
if (ename=="CDATA") {
  fprintf(fp,"          %scfrag = ",csb);    // Now any cfrag ...
  if (cfrag.empty()) fprintf(fp,"empty\n");  // None there
  else fprintf(fp,"\"%s...\" (%lu chars)\n",
                  Prettify(cfrag).c_str(),cfrag.size());
}
                                       // Pretty-print the children
WALKVECTOR(node *,vnode,i)(*i)->Show(fp,s0);
}

//------------------------------------------------------------------------------

void xmlTreeDump::node::TreeStrings(string & rs0,string & rsh,string & rsb,string & rst)
// Routine to generate the pretty-print strings that decorate a node tree
// display. Can't make it general, because it requires access to the child-
// holding vector in each node. Yet another reason for bothering to make tree a
// template......
// rs0 : the 'offset' string that is handed down throughout the recursive tree
// display
// rsh : (header) derived from rs0; prefix for first line of the node display
// rsb : (body) derived from rs0; prefix for middle lines of the node display
// rst : (tail) derived from rs0; prefix for last line of the node display
// (This is just a bit of showing off, but it makes the display clearer.)
// There is a lot of mucking about to distinguish between the root node (i.e.
// the one with no parent) and the rest. I'm sure this could be cleaner.
{
                                       // Paranoia.... just in case
if (rs0.size()>256) rs0 = "Tree display reset: possible recursion?   ";

rsh = rs0;
rsb = rs0;
if (par==0) rs0 += "| ";               // Root node? handle next base
else {                                 // Not root node
  rsh.replace(rsh.size()-2,2,"+-");    // Header
  if (this==par->vnode.back()) rsb.replace(rsb.size()-2,2,"  ");   // Body
  if (this==par->vnode.back()) {       // Next base
    rs0[rs0.size()-2]=' ';
    rs0 += "| ";
  } else rs0 += "  ";
}

if (vnode.empty()) rsb += " == ";      // Body - same for root/non-root
else               rsb += "|== ";
rst = rsb;                             // Tail - same for root/non-root
rst.replace(rst.size()-4,3,"|  ");
if (vnode.empty()) rst.replace(rst.size()-4,1," ");
return ;
}

//==============================================================================

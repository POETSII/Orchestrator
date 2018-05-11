//---------------------------------------------------------------------------

#include "jnj.h"
#include "macros.h"

//==============================================================================

JNJ::JNJ():UIF()
{
}

//------------------------------------------------------------------------------

JNJ::JNJ(int argc, char ** argv):UIF(argc,argv)
{
}

//------------------------------------------------------------------------------

JNJ::JNJ(string fn):UIF()
{
Add(fn);
}

//------------------------------------------------------------------------------

JNJ::~JNJ()
{
}

//------------------------------------------------------------------------------

hJNJ JNJ::FndRecd(hJNJ n)
// Given name handle (can be label, variable or value), find parent record
{
hJNJ p;
for (p=n; p->Type()!=No_recd; p=p->par)
  if (p->par==0) return 0;             // Exit if called on a section name
return p;                              // Only here when parent is No_recd
}

//------------------------------------------------------------------------------

void JNJ::FndRecdLabl(hJNJ sect,string str,vH & recds)
// Given section and 0-level label string, find parent record(s)
{
recds.clear();                        // Clear the answer
if (sect==0) return;                  // Silly question
vH labs;
GetXXX(sect,labs,No_labl);            // Get the label nodes with the name
                                      // Get the corresponding records
WALKVECTOR(Node *,labs,i) if ((*i)->str==str) recds.push_back(FndRecd(*i));

}

//------------------------------------------------------------------------------

void JNJ::FndRecdVari(hJNJ sect,string str,vH & recds)
// Given section and variable string, find parent record(s)
{
recds.clear();                        // Clear the answer
if (sect==0) return;                  // Silly question
vH vars;
GetXXX(sect,vars,No_vari);            // Get all variable nodes
                                      // Get the corresponding records
WALKVECTOR(Node *,vars,i) if ((*i)->str==str) recds.push_back(FndRecd(*i));
}

//------------------------------------------------------------------------------

void JNJ::FndRecdValu(hJNJ sect,string str,vH & recds)
// Given section and value string, find parent record(s)
{
recds.clear();                        // Clear the answer
if (sect==0) return;                  // Silly question
vH valu;
GetXXX(sect,valu,No_valu);            // Get the value nodes with the name
                                      // Get the corresponding records
WALKVECTOR(Node *,valu,i) if ((*i)->str==str) recds.push_back(FndRecd(*i));

}

//------------------------------------------------------------------------------

void JNJ::FndSect(string name, vH & sects)
// Given a name string, find the sections that include it
{
sects.clear();
WALKVECTOR(Node *,UIF_root->leaf,i)    // Walk the sections
  WALKVECTOR(Node *,(*i)->leaf,j)      // Walk the names/records
    if ((*j)->Type()==No_name)         // Name?
      WALKVECTOR (Node *,(*j)->leaf,k) // Walk the subtree
        if ((*k)->str==name) sects.push_back(*i);
}

//------------------------------------------------------------------------------

hJNJ JNJ::FndSect(hJNJ n)
// Given name, find parent section
{
hJNJ p;
for (p=n; p->Type()==No_name; p=p->par)
  if (p->par==0) return 0;             // SNH
                                       // Here when parent is not No_name
if (p->Type()!=No_sect) return 0;      // SNH
return p;
}

//------------------------------------------------------------------------------

void JNJ::GetAExpr(hJNJ n,vH & v)
// Given section/record, find expression root nodes IN THE ATTRIBUTES
{
GetAAA(n,v,No_expr);
}

//------------------------------------------------------------------------------

void JNJ::GetALabl(hJNJ n,vH & v)
// Given section/record, find label names IN THE ATTRIBUTES
{
GetAAA(n,v,No_labl);
}

//------------------------------------------------------------------------------

void JNJ::GetAVari(hJNJ n,vH & v)
// Given section/record, find variables IN THE ATTRIBUTES
{
GetAAA(n,v,No_vari);
}

//------------------------------------------------------------------------------

void JNJ::GetLabl(hJNJ n,vector<hJNJ> & v)
// Given section/record, find label names
{
GetXXX(n,v,No_labl);
}

//------------------------------------------------------------------------------

void JNJ::GetNames(vector<hJNJ> & v)
// Get all the section (first level) names in the object
{
v.clear();
WALKVECTOR(Node *,UIF_root->leaf,i)
  WALKVECTOR(Node *,(*i)->leaf,j)
    if ((*j)->Type()==No_name) WALKVECTOR (Node *,(*j)->leaf,k)v.push_back(*k);
}

//------------------------------------------------------------------------------

void JNJ::GetNames(hJNJ n,vector<hJNJ> & v)
// Get all the section (first level) names of the section n
{
v.clear();
if (n==0) return;
if (n->Type()!=No_sect) return;
WALKVECTOR(Node *,n->leaf,i)
  if ((*i)->Type()==No_name)
    WALKVECTOR (Node *,(*i)->leaf,j)v.push_back(*j);
}

//------------------------------------------------------------------------------

void JNJ::GetRecd(hJNJ n,vH & v)
// Get all the records in a section (which is everything except the names and
// commands
{
v.clear();
WALKVECTOR(Node *,n->leaf,i) if ((*i)->Type()==No_recd) v.push_back(*i);
}

//------------------------------------------------------------------------------

void JNJ::GetSect(vH & v)
// Get the section header nodes for the entire object. It's a bit pointless,
// because we just copy the leaf vector of the root node, but for the sake of
// completeness.....
// The output *includes* the unnamed sector. Obviously.
{
v.clear();
v = UIF_root->leaf;
}

//------------------------------------------------------------------------------

void JNJ::GetSub(hJNJ n,vector<hJNJ> & v,int d)
// Given a name node, get the subnames
{
v.clear();
if (n->Type()!=No_name) return;        // Sanity check
d = abs(d);                            // Sanity check
if (d==0) {                            // Silly case
  v.push_back(n);
  return;
}

// BODGE FOR NOW   - we go down one level irrespective of what we asked for

                                       // Now down to business
WALKVECTOR(Node *,n->leaf,i) v.push_back(*i);
}

//------------------------------------------------------------------------------

void JNJ::GetValu(hJNJ n,vector<hJNJ> & v)
// Given section/record, find value names
{
GetXXX(n,v,No_valu);
}

//------------------------------------------------------------------------------

void JNJ::GetVari(hJNJ n,vector<hJNJ> & v)
// Given section/record, find variable names
{
GetXXX(n,v,No_vari);
}

//------------------------------------------------------------------------------

void JNJ::LocLabl(hJNJ n,vector<hJNJ> & v)
// This one goes sideways from a name to the set of label names associated with
// the record containing n
{
v.clear();
if (n==0) return;                      // Defensive programming.....
if (n->Type()!=No_name) return;
GetLabl(FndRecd(n),v);

}

//------------------------------------------------------------------------------

void JNJ::LocName(vector<hJNJ> & vi,string s,vector<hJNJ> & vo)
// Locate instances of the string 's' in the vector 'vi', and push those
// addresses into 'vo'
{
vo.clear();
WALKVECTOR(Node *,vi,i) if ((*i)->str == s) vo.push_back(*i);
}

//------------------------------------------------------------------------------

void JNJ::LocName(vector<hJNJ> & vi,char * s,vector<hJNJ> & vo)
// Locate instances of the string 's' in the vector 'vi', and push those
// addresses into 'vo'. We can't rely on automatic argument type promotion, 'cos
// BORLAND (at least) has trouble with resolving an actual argument of
// "(char *)0"
{
LocName(vi,string(s),vo);
}

//------------------------------------------------------------------------------

hJNJ JNJ::LocRecd(hJNJ s,hJNJ r)
// Routine to locate the record next to (r) in the section (s). This enables us
// to walk a loop of records within the current sector:
// 0->r1->r2->r3->...->rn->0 and so on
{
if (s==0) return 0;                    // Preconditions
if (s->Type()!=JNJ::No_sect) return 0;
if (r!=0) {
  if (r->Type()!=JNJ::No_recd) return 0;
  if (r->P()!=s) return 0;
}
if (s->leaf.size()==0) return 0;       // Weak precondition
/*                                       // OK, we're in non-silly business
static struct {                        // "Last one out"
  unsigned int i;
  hJNJ p;
} cache = {0,0};
  */
hJNJ fr = 0;                           // Find first record?
WALKVECTOR(Node *,s->leaf,i) if ((*i)->Type()==No_recd) {
  fr = *i;
  break;
}
if (r==0) return fr;                   // Was 0, now start
if (r== *(s->leaf.end()-1)) return 0;  // Was end, now 0
                                       // Still here? We need to find the index
                                       // of where we are now:
for(uint i=0;i<s->leaf.size();i++)
  if (s->leaf[i]==r)
    for (uint j=i+1;j<s->leaf.size();j++)
      if (s->leaf[j]->Type()==No_recd)
        return s->leaf[j];

return 0;                              // Never here - keep the compiler happy
}

//------------------------------------------------------------------------------

hJNJ JNJ::LocSect(hJNJ s)
// Routine to locate the section next to (s) in the root node (r0).
// Argument of 0 (the default) gives us the first section (which may or may not
// be blank). This enables us to walk a loop of sections within the current
// root: 0->s1->s2->s3->...->sn->0 and so on.
{
hJNJ r0 = UIF_root;
if (r0==0) return 0;                   // Preconditions
if (r0->Type()!=JNJ::No_0000) return 0;
if (s!=0) {
  if (s->Type()!=JNJ::No_sect) return 0;
  if (s->P()!=r0) return 0;
}
if (r0->leaf.size()==0) return 0;      // Weak precondition

                                       // OK, we're in non-silly business
if (s==0) return r0->leaf[0];          // Was 0, now start
if (s== *(r0->leaf.end()-1)) return 0; // Was end, now 0
                                       // Still here? We need to find the index
                                       // of where we are now:
for(uint i=0;i<r0->leaf.size();i++) if (r0->leaf[i]==s) return r0->leaf[++i];

return 0;                              // Never here - keep the compiler happy
}

//------------------------------------------------------------------------------

void JNJ::LocValu(hJNJ n,vector<hJNJ> & v)
// This one goes sideways from a name to the set of value names associated with
// the record containing n
{
v.clear();
if (n==0) return;                      // Defensive programming.....
if (n->Type()!=No_name) return;
GetValu(FndRecd(n),v);
}

//------------------------------------------------------------------------------

void JNJ::RecdSort(vector<hJNJ> & v)
// Sort the record variable names
{
}

//------------------------------------------------------------------------------

void JNJ::SectSort(vector<hJNJ> & v)
// Sort the section names
{
}

//------------------------------------------------------------------------------

void JNJ::printv(vector<hJNJ> & v)
{
printf("JNJ::printv: vector contains %u names\n",(unsigned)v.size());
WALKVECTOR(hJNJ,v,i) (*i)->Dumpt();
printf("\n");
}

//------------------------------------------------------------------------------

void JNJ::GetAA(hJNJ inNode,vector<hJNJ> & v,Notype tgt)
// Given a start record node, find label/variable/expression attribute nodes
{
if (inNode==0) return;                                     // psect->Dumpt();
WALKVECTOR(Node *,inNode->leaf,j) {                     // (*j)->Dumpt();
  if ((*j)->Type()==No_attr) {
    WALKVECTOR(Node *,(*j)->leaf,k) {                 // (*k)->Dumpt();
      if ((*k)->Type()==tgt) {
        WALKVECTOR(Node *,(*k)->leaf,l) {             // (*l)->Dumpt();
          if ((tgt!=No_expr)&&((*l)->Type()==No_name)) v.push_back(*l);
          if ((tgt==No_expr)&&((*l)->Type()==No_e_ex)) v.push_back(*l);
        }
      }
    }
  }
}
}

//------------------------------------------------------------------------------

void JNJ::GetAAA(hJNJ inNode,vector<hJNJ> & v,Notype tgt)
// Given section or record, find label/variable/expression attribute nodes
{
v.clear();
if (inNode==0) return;
if (inNode->Type()==No_recd) GetAA(inNode,v,tgt);  // It's a record
else WALKVECTOR(Node *,inNode->leaf,i)             // It's a section
       if ((*i)->Type()==No_recd) GetAA(*i,v,tgt);
}

//------------------------------------------------------------------------------

void JNJ::GetXX(hJNJ inNode,vector<hJNJ> & v,Notype tgt)
// Given a start record node, find label/variable/value name nodes
{
if (inNode==0) return;                                     // psect->Dumpt();
WALKVECTOR(Node *,inNode->leaf,j)                     // (*j)->Dumpt();
  if ((*j)->Type()==No_body)
    WALKVECTOR(Node *,(*j)->leaf,k)                  // (*k)->Dumpt();
      if ((*k)->Type()==tgt)
        WALKVECTOR(Node *,(*k)->leaf,l)              // (*l)->Dumpt();
          if ((*l)->Type()==No_name) v.push_back(*l);
}

//------------------------------------------------------------------------------

void JNJ::GetXXX(hJNJ inNode,vector<hJNJ> & v,Notype tgt)
// Given section or record, find label/variable/value name nodes
{
v.clear();
if (inNode==0) return;
if (inNode->Type()==No_recd) GetXX(inNode,v,tgt);  // It's a record
else WALKVECTOR(Node *,inNode->leaf,i)             // It's a section
       if ((*i)->Type()==No_recd) GetXX(*i,v,tgt);
}

//==============================================================================

double str2dble(hJNJ node)
// *Signed* string to double.....
{
if (node==0) return 0.0;
double ans = str2dble(node->str);
if (node->qop==Lex::Sy_sub) return -ans;
return ans;
}

//------------------------------------------------------------------------------

int str2int(hJNJ node)
// *Signed* string to double.....
{
if (node==0) return 0;
int ans = str2int(node->str);
if (node->qop==Lex::Sy_sub) return -ans;
return ans;
}

//==============================================================================

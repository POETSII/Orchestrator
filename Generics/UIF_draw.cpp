//---------------------------------------------------------------------------

#include "uif_draw.h"
#include "macros.h"

const char * UIF_DRAW::DCtype_str[] = {
"_DC_0000","_DC_SIZE","_DC_LINE","_DC_MOVE","_DC_CIRC","_DC_PEN ","_DC_BRSH",
"_DC_TEXT","_DC_MEMO","_DC_OPER","_DC_TYPE","_DC_BLCK","_DC_GREY","_DC_RED ","_DC_WHIT",
"_DC_YELL","_DC_GREE","_DC_BLUE","_DC_CYAN","_DC_MARO","_DC_OLIV","_DC_NAVY",
"_DC_PURP","_DC_TEAL","_DC_SILV","_DC_LIME","_DC_FUSC","_DC_LTGR","_DC_MDGR",
"_DC_DKGR","_DC_MGRN","_DC_SKBL","_DC_CREA","_DC_XXXX"};

const UIF_DRAW::DCtype UIF_DRAW::Notype_attr[No_XXXX+1][2] = {
// Pen    Brush
 {_DC_BLCK,_DC_BLCK},       //   No_0000
 {_DC_BLCK,_DC_GREY},       //   No_sect
 {_DC_GREY,_DC_GREY},       //   No_recd
 {_DC_RED ,_DC_WHIT},       //   No_cmnd
 {_DC_YELL,_DC_YELL},       //   No_cmnt
 {_DC_GREE,_DC_GREY},       //   No_body
 {_DC_GREE,_DC_GREY},       //   No_attr
 {_DC_GREE,_DC_WHIT},       //   No_labl
 {_DC_BLUE,_DC_WHIT},       //   No_vari
 {_DC_CYAN,_DC_WHIT},       //   No_valu
 {_DC_BLCK,_DC_YELL},       //   No_name
 {_DC_PURP,_DC_LTGR},       //   No_expr
 {_DC_PURP,_DC_MDGR},       //   No_e_ex
 {_DC_PURP,_DC_DKGR},       //   No_e_op
 {_DC_PURP,_DC_YELL},       //   No_e_LB
 {_DC_PURP,_DC_YELL},       //   No_e_RB
 {_DC_RED ,_DC_RED }        //   No_XXXX
};

//==============================================================================

UIF_DRAW::UIF_DRAW():UIF()
{
geometry = 0;                          // "Geometry not defined" flag
//fp = stdout;                           // Default output stream
//size.minx = size.maxx = 0.0;           // Size rectangle. Just being tidy -
//size.miny = size.maxy = 0.0;           // some/all of these will be overwritten

SetECB(this,UIF_DRAW::DefECB);         // Catch any errors
}

//------------------------------------------------------------------------------

UIF_DRAW::~UIF_DRAW()
// We need to explicitly destroy the tag structures here because the base class
// destructor can't look past the (void *) tags
// Beware diagnostic writes in the destructors - if UIF_DRAW is created
// automatically, the user may well (had bloody well better) have closed the
// output stream by now.
{
DestroyTags(UIF_root);
}

//------------------------------------------------------------------------------

void UIF_DRAW::Add(string name)
{
UIF::Add(name);                        // Load the file into the class
Add_0(UIF_root);                       // Create the drawing datastructure
geometry = 0;                          // Flag says structure is invalid
}

//------------------------------------------------------------------------------

void UIF_DRAW::Add_0(UIF::Node * nd)
{
//nd->Dump();
//fprintf(ofp,"\n-------------\n");
nd->tag = new draw_0();
WALKVECTOR(UIF::Node *,nd->leaf,i) if ((*i)!=0) Add_0(*i);
}

//------------------------------------------------------------------------------

void UIF_DRAW::Attrib(uint attr)
// Add (i.e. OR) the attribute 'attr' with every node attribute word
{
Attrib(UIF_root,attr);
}

//------------------------------------------------------------------------------

void UIF_DRAW::Attrib(UIF::Node * nd,uint attr,int lv)
// Add (i.e. OR) the attribute 'attr' with every node attribute word, starting
// with the root 'nd', down 'lv' levels
{
Attrib_0(nd,attr,lv,lv);
}

//------------------------------------------------------------------------------

void UIF_DRAW::Attrib(vector<UIF::Node *> & nv,uint attr,int lv)
// Add (i.e. OR) the attribute 'attr' with every node (in the vector) attribute
// word, starting with the root 'nd', down 'lv' levels
{
WALKVECTOR(UIF::Node *,nv,i) if ((*i)!=0) Attrib_0(*i,attr,lv,lv);
}

//------------------------------------------------------------------------------

void UIF_DRAW::Attrib_0(UIF::Node * nd,uint attr,int lv,int cnt)
{
static_cast<draw_0 *>(nd->tag)->attr |= attr;
if (--cnt==0) return;
WALKVECTOR(UIF::Node *,nd->leaf,i) if ((*i)!=0) Attrib_0(*i,attr,lv,cnt);
}

//------------------------------------------------------------------------------

void UIF_DRAW::DefECB(void * pThis,void * p,int id)
// Overload of the base class (UIF) default error callback
// pThis is the object address, which is n/u in this here default handler
// Never called with id=0, which is just as well, 'cos Node::Dump() is
// expecting a string argument.....
// The problem here is that we're parsing the input file, and Windoze hasn't
// yet asked us to create the drawing command vector, so we haven't got it.
// So we store all this stuff in a local vector, which has to be static
// so it stays in scope when poked *from* Windoze....
{
static vector<string> message;
string s0;
switch (id) {
//  case 0  : static_cast<UIF::Node *>(p)->Dump(message);         break;
  case 1  : static_cast<UIF *>(p)->Lx.Hst.Dump(message);        break;
  default : dprintf(s0,"Unrecognised error identifier\n");
            message.push_back(s0);                              break;
}
// So here and now we have a vector of strings......
WALKVECTOR(string,message,i)
  static_cast<UIF_DRAW *>(pThis)->locdv.push_back(DC(UIF_DRAW::_DC_MEMO,
    const_cast<char *>((*i).c_str())));

}

//------------------------------------------------------------------------------

void UIF_DRAW::DestroyTags(Node * p)
// Trash the tag structure of one node
{
if (p==0) return;
WALKVECTOR(Node *,p->leaf,i)DestroyTags(*i);
draw_0 * d0 = static_cast<draw_0 *>(p->tag);
delete d0;
}

//------------------------------------------------------------------------------

void UIF_DRAW::Draw(vector<DC> &dv,UIF::Node * nd,int lev)
// Routine to load the DC vector with draw commands (i.e. it's a u$oft-free
// drawing routine). We start at the node "nd", and go down "lev" levels
// NOTE THAT the drawing vector takes only *pointers* to strings, so it is
// ESSENTIAL that the UIF_DRAW object is still in scope at the receiving end
// when the stuff is unpacked. 
{
size_0 s(nd);                          // Initialise the bounding rectangle
Size0(nd,s);                           // Calculate the bounding rectangle
dv.push_back(DC(_DC_SIZE,s.minx,s.maxx,(int)s.miny,(int)s.maxy));
// This HAS to be the first one

PushPen(dv,_DC_BLCK);                  // Force initial pen & brush loads
PushBrush(dv,_DC_BLCK);
Draw0(dv,nd,lev);                      // And recurse away.....
                                       // And finally the memo messages
dv.insert(dv.end(),locdv.begin(),locdv.end());
}

//------------------------------------------------------------------------------

void UIF_DRAW::PushPen(vector<DC> &dv,DCtype p)
{
static DCtype kpen = _DC_XXXX;
if (kpen==p) return;
dv.push_back(DC(_DC_PEN,kpen=p));
}

//------------------------------------------------------------------------------

void UIF_DRAW::PushBrush(vector<DC> &dv,DCtype b)
{
static DCtype kbrush = _DC_XXXX;
if (kbrush==b) return;
dv.push_back(DC(_DC_BRSH,kbrush=b));
}

//------------------------------------------------------------------------------

void UIF_DRAW::Draw0(vector<DC> &dv,UIF::Node * nd,int lev)
// Recursive bit of the above that actually does the work.
{
                                       // Pull out the tag pointer
draw_0 * p0 = static_cast<draw_0 *>(nd->tag);
                                       // Draw the child arcs

WALKVECTOR(UIF::Node *,nd->leaf,i) if ((*i)!=0) {
  PushPen(dv,((*i)->P()==nd)? _DC_BLCK:_DC_RED);
  draw_0 * p1 = static_cast<draw_0 *>((*i)->tag);
  dv.push_back(DC(_DC_MOVE,p0->x,p0->y));
  dv.push_back(DC(_DC_LINE,p1->x,p1->y));
}
PushPen(dv,Notype_attr[nd->Type()][0]);// Node settings
PushBrush(dv,Notype_attr[nd->Type()][1]);
dv.push_back(DC(_DC_CIRC,p0->x,p0->y));// The node itself
dv.push_back(DC(_DC_TYPE,p0->x,p0->y,(int)(nd->Type()))); // The type
if ((nd->str.c_str())[0]!=0)           // Any text ?
  dv.push_back(DC(_DC_TEXT,p0->x,p0->y,const_cast<char *>(nd->str.c_str())));
if (nd->qop!=Lex::S_00)                // Any operator ?
  dv.push_back(DC(_DC_OPER,p0->x,p0->y,const_cast<char *>(Lex::Sytype_str[nd->qop])));
dv.push_back(DC(_DC_POSI,p0->x,p0->y,(int)(nd->pos)));    // Source position
if (--lev==0) return;                  // Recursion terminated early ?
WALKVECTOR(UIF::Node *,nd->leaf,i) if ((*i)!=0) Draw0(dv,(*i),lev);
}

//------------------------------------------------------------------------------

void UIF_DRAW::Geometry()
// Routine you actually call from outside to start the whole thing off.
// The whole algorithm set is (should be...) tolerant of null children.
{
scan(UIF_root);
gW(UIF_root);                          // Set up node widths
//Dump();
gInt(UIF_root);                        // Calculate width integrals
gXY1(UIF_root,0.0,0.0);                // Derive coordinate values
Dump();
geometry = 1;                          // "Geometry defined" flag
}

//------------------------------------------------------------------------------

void UIF_DRAW::Hit(UIF::Node *&,int,int){}
void UIF_DRAW::Hit(vector<UIF::Node *> &,int,int,int,int){}

void UIF_DRAW::Size(int &,int &,int &,int &){}
void UIF_DRAW::Size(UIF::Node *,int &,int &,int &,int &){}
void UIF_DRAW::Size(vector<UIF::Node *> &,int &,int &,int &,int &){}

void UIF_DRAW::SetDefaults(){}
void UIF_DRAW::SetSep(int,int){}
void UIF_DRAW::SetScale(int){}
void UIF_DRAW::SetBorder(){}

//==============================================================================
     /*
void UIF_DRAW::Draw(void (* D0)(UIF::Node *))
// Routine to actually draw the tree
{
if (geometry==0) return;               // Geometry not yet defined
//DrawFunc = D0;                         // Load callback
//Size0(UIF_root);                       // Load the size rectangle
Draw0(UIF_root);                       // And do it...
}

//------------------------------------------------------------------------------

void UIF_DRAW::Draw0(UIF::Node * p)
{
//(*DrawFunc)(p);                        // Draw the node
                                       // Walk the children
WALKVECTOR(Node *,p->leaf,i) if (*i!=0) Draw0(*i);
}
       */
//------------------------------------------------------------------------------

void UIF_DRAW::Dump()
{
Dump0(UIF_root);
}

//------------------------------------------------------------------------------

void UIF_DRAW::Dump0(UIF::Node * p)
{
if (p==0) fprintf(ofp,"Node 000000\n");
else {
  Dump00(p);
  WALKVECTOR(Node *,p->leaf,i) fprintf(ofp,"| %6x |",*i);
  fprintf(ofp,"\n");
  WALKVECTOR(Node *,p->leaf,i) Dump0(*i);
}
}

//------------------------------------------------------------------------------

void UIF_DRAW::Dump00(UIF::Node * p)
{

if (p==0) fprintf(ofp,"Node 000000");
else {
  draw_0 * d0 = static_cast<draw_0 *>(p->tag);
  if (d0==0) fprintf(ofp,"Node %6x: NO TAG ",p);
  else fprintf(ofp,"Node %6x: x: %9.2e, y: %9.2e, w: %3d, Il: %3d | Parent : %6x ",
                  p,d0->x,d0->y,d0->w,d0->Il,p->P());
}
}

//------------------------------------------------------------------------------

float UIF_DRAW::fX1(Node * p)
// Calculate the abscissa value for the node p
{
if (p->P()==0) return 0.0;

draw_0 * d0 = static_cast<draw_0 *>(p->tag);
draw_0 * dp = static_cast<draw_0 *>(p->P()->tag);
fprintf(ofp,"%x w: %3d, Il: %3d, Pw: %3d, Px: %9.2e",
        p,d0->w,d0->Il,dp->w,dp->x);

float ans = d0->Il + dp->x + float(d0->w - dp->w)/2.0;
fprintf(ofp," ans: %9.2e\n",ans);

return d0->Il + dp->x + float(d0->w - dp->w)/2.0;
}

//------------------------------------------------------------------------------

void UIF_DRAW::gInt(Node * p)
// Calculate integral node widths. The rather convoluted logic is because the
// child vector could contain an arbitrary numvber of nulls
{
if (p==0) return;                      // Null node
int cw = 0;                            // Cumulative width
// The simple logic is
//  (*i)->d.Il = cw;                   // Write the integral
//  cw += (*i)->d.w;                   // Update the integral
// BUT we have to cope with anything not being there, so it gets complicated:
                                       // Walk the children
WALKVECTOR(Node *,p->leaf,i) if (*i!=0) {
    draw_0 * d0 = static_cast<draw_0 *>((*i)->tag);
    d0->Il = cw;
    cw += d0->w;
} else cw += 1;
WALKVECTOR(Node *,p->leaf,i) gInt(*i); // Walk on down
}

//------------------------------------------------------------------------------

void UIF_DRAW::gW(Node * p)
// Calculate node widths. This is tolerant of null children, who contribute a
// fixed width of 1.
{
if (p==0) return;                      // Null child
p->Dumpt(ofp);
draw_0 * d0 = static_cast<draw_0 *>(p->tag);
d0->w = 0;                             // Paranoia
WALKVECTOR(Node *,p->leaf,i) gW(*i);   // Walk the children
if (p->leaf.empty()) d0->w = 1;        // It's a leaf...
else d0->w = gWchild(p);               // No; get the sum of the child widths
}

//------------------------------------------------------------------------------

int UIF_DRAW::gWchild(Node * p)
// Calculate the sum of the widths of the children of p
{
if (p==0) return 1;                    // Null node
int ans = 0;                           // Accumulate width
draw_0 * d0;
WALKVECTOR(Node *,p->leaf,i) if (*i!=0) {
  d0 = static_cast<draw_0 *>((*i)->tag);
  ans += d0->w;
} else ans += 1;
return ans;
}

//------------------------------------------------------------------------------

void UIF_DRAW::gXY1(Node * p,float y,float & minx)
// Derive the coordinate values for the node tree
{
if (p==0) return;                      // Null node
draw_0 * d0 = static_cast<draw_0 *>(p->tag);
d0->x = fX1(p);
d0->y = y;
y += 1.0;
minx = std::min(minx,d0->x);           // Left-most extent
WALKVECTOR(Node *,p->leaf,i) gXY1(*i,y,minx);
d0->x -= minx;                         // Normalise the tree
}

//------------------------------------------------------------------------------

void UIF_DRAW::scan(UIF::Node * p)
// Scan for null children in the tree.
{
if (p->P()==0)fprintf(ofp,"Scan located null parent ... (of %p)\n",p);
WALKVECTOR(Node *,p->leaf,i)
  if (*i!=0) scan(*i);
  else {
    fprintf(ofp,"Scan located null child... %x\nParent : ",p);
    Dump00(p);
    fprintf(ofp,"\n");
    WALKVECTOR(Node *,p->leaf,i)
      if (*i!=0) {
        fprintf(ofp,"Child  : ");
        Dump00(*i);
        fprintf(ofp,"\n");
      }
      else fprintf(ofp,"Child  : NULL\n");
  }
}

//------------------------------------------------------------------------------
 
void UIF_DRAW::Size0(UIF::Node * p,size_0 & s)
{
draw_0 * d0 = static_cast<draw_0 *>(p->tag);
s.minx = std::min(s.minx,d0->x);       // Update the size structure
s.maxx = std::max(s.maxx,d0->x);
s.miny = std::min(s.miny,d0->y);
s.maxy = std::max(s.maxy,d0->y);
s.Dump(ofp);
                                       // Walk the children
WALKVECTOR(Node *,p->leaf,i) if (*i!=0) Size0(*i,s);
}

//------------------------------------------------------------------------------
 /*
void UIF_DRAW::Setfp(FILE * f)
// Guess
{
fp = f;
}
   */
//------------------------------------------------------------------------------






#ifndef __UIF_DRAW__H
#define __UIF_DRAW__H

#include "uif.h"
#include "flat.h"

//==============================================================================
/* This is a Windoze-free derived class from UIF. We wander through the UIF
node tree, and work out the node positions and colours to draw the thing.
The 'host' Windoze program creates a vector (dv) that is then passed into
UIF_DRAW::Draw() (by reference). This oversees the loading of the vector,
so when Draw() returns the vector has a bunch of commands in it that the Windoze
program can interpret.
Why do it this way? so UIF_DRAW doesn't - ever - have to know anything about
Windoze. Windoze has to know about UIF_DRAW, to get the declarations for the
caommand vector right, but, hey,.....
*/
//==============================================================================

class UIF_DRAW : public UIF {

public:
                   UIF_DRAW();
virtual ~          UIF_DRAW();

struct DC {
  void Init()
    {f[0]=f[1]=f[2]=0.0;i[0]=i[1]=i[2]=0;s=0;};
  DC(int c,int i0)
    {Init(); cmnd=c; i[0]=i0;};
  DC(int c,float f0,float f1)
    {Init(); cmnd=c; f[0]=f0; f[1]=f1;};
  DC(int c,float f0,float f1,int i0)
    {Init(); cmnd=c; f[0]=f0; f[1]=f1; i[0]=i0;};
  DC(int c,float f0,float f1,int i0,int i1)
    {Init(); cmnd=c; f[0]=f0; f[1]=f1; i[0]=i0; i[1]=i1;};
  DC(int c,float f0,float f1,char * s0)
    {Init(); cmnd=c; f[0]=f0; f[1]=f1; s=s0;};
  DC(int c,char * s0)
    {Init(); cmnd=c; s=s0;};
  void Dump(FILE * of=stdout){
    fprintf(of,"%s(%2d) %2d %2d %2d %8.2f %8.2f %8.2f %s\n",
            DCtype_str[cmnd],cmnd,i[0],i[1],i[2],f[0],f[1],f[2],s==0?"**0**":s);

  };
  int    cmnd;
  int    i[3];
  float  f[3];
  char * s;
};

enum DCtype {_DC_0000=0,_DC_SIZE,_DC_LINE,_DC_MOVE,_DC_CIRC,_DC_PEN,_DC_BRSH,
_DC_TEXT,_DC_MEMO,_DC_OPER,_DC_TYPE,_DC_POSI,_DC_BLCK,_DC_GREY,_DC_RED ,
_DC_WHIT,_DC_YELL,_DC_GREE,_DC_BLUE,_DC_CYAN,_DC_MARO,_DC_OLIV,_DC_NAVY,
_DC_PURP,_DC_TEAL,_DC_SILV,_DC_LIME,_DC_FUSC,_DC_LTGR,_DC_MDGR,_DC_DKGR,
_DC_MGRN,_DC_SKBL,_DC_CREA,_DC_ORAN,_DC_XXXX};
static const char * DCtype_str[_DC_XXXX+1];
static const DCtype Notype_attr[No_XXXX+1][2];     // Pen/brush attributes for nodes

void               Add(string name);

void               Attrib(uint);
void               Attrib(UIF::Node *,uint,int=0);
void               Attrib(vector<UIF::Node *> &,uint,int=0);

void               Draw(vector<DC> &,UIF::Node *,int=-1);

void               Dump();
void               Geometry();
void               Hit(UIF::Node *&,int,int);
void               Hit(vector<UIF::Node *> &,int,int,int,int);

void               Size(int &,int &,int &,int &);
void               Size(UIF::Node *,int &,int &,int &,int &);
void               Size(vector<UIF::Node *> &,int &,int &,int &,int &);

void               SetDefaults();
void               SetSep(int,int);
void               SetScale(int);
void               SetBorder();

private:
int                geometry;
//FILE *             fp;
struct size_0 {
  size_0(UIF::Node * n){
    draw_0 * d0 = static_cast<draw_0 *>(n->tag);
    minx = maxx = d0->x;
    miny = maxy = d0->y;
  };
  void Dump(FILE * f){
    fprintf(f,"minx = %8.2f maxx = %8.2f miny = %8.2f maxy = %8.2f\n",
               minx,maxx,miny,maxy);
  };
  float            minx;
  float            maxx;
  float            miny;
  float            maxy;
};

struct draw_0 {
  draw_0():y(0.0),x(0.0),w(0),Il(0){};
  float            y;                  // y coordinate w.r.t root
  float            x;                  // x coordinate w.r.t. root
  int              w;                  // Node width
  int              Il;                 // Sum of left sibling widths
  uint             attr;               // Drawing attributes
};

vector<DC>         locdv;              // Local drawing vector

void               Add_0(UIF::Node *);
void               Attrib_0(UIF::Node *,uint,int,int);
static void        DefECB(void *,void *,int);
void               DestroyTags(Node *);    // Static because called from within Node
void               Draw(void (*)(UIF::Node *));
void               Draw0(vector<DC> &,UIF::Node *,int);
void               Draw0(UIF::Node *);

void               Dump0(UIF::Node *);
void               Dump00(UIF::Node *);
float              fX1(UIF::Node *);

void               gInt(UIF::Node *);
void               gW(UIF::Node *);
int                gWchild(UIF::Node *);
void               gXY1(UIF::Node *,float,float &);
void               PushPen(vector<DC> &,DCtype);
void               PushBrush(vector<DC> &,DCtype);
void               scan(UIF::Node *);
void               Size0(UIF::Node *,size_0 &);
//void               Setfp(FILE * = stdout);

};

//==============================================================================

#endif

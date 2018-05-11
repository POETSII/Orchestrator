//------------------------------------------------------------------------------

#include "ncube.h"
#include "macros.h"
#include <stdio.h>

//==============================================================================

void PV(vector<int> v)
{
printf("PV: [%d] ",v.size());
for (unsigned int i=0;i<v.size();i++) printf(" %d ",v[i]);
}

//==============================================================================

Ncube::Ncube(int dims,int hi[],int lo[],bool wr[])
// The Cosmic Constructor. Handles the defaults if some arguments are missing.
{
//ndims = dims;                          // Store the dimensionality

int  * lt0 = new int[dims];           // Default values
bool * lw0 = new bool[dims];
for (int i=0;i<dims;i++) {
  lt0[i] = 0;
  lw0[i] = false;
}
                                       // Bugger about replacing the missing
int  * plo = lo;                       // with real ones filled with stuff
bool * pwr = wr;

if (plo==0)plo=lt0;
if (pwr==0)pwr=lw0;

doit(dims,hi,plo,pwr);                 // Zer guts

delete [] lt0;                         // Kill the local defaults
delete [] lw0;
}

//------------------------------------------------------------------------------

Ncube::~Ncube()
{
//printf("~Ncube\n");

WALKVECTOR(_L *,garbage,i) delete *i;
delete [] cell0;

}

//------------------------------------------------------------------------------

void Ncube::doit(int dims,int hi[],int lo[],bool wr[])
// Constructor proper
{
extent.resize(dims);
ncells = 1;                            // Seed total cell count
for (int j=0;j<dims;j++) {
  extent[j].lo = lo[j];                // Low index
  extent[j].hi = hi[j];                // High index
  extent[j].wr = wr[j];                // Wrap-around flag
  extent[j].wi = hi[j]-lo[j]+1;        // Index extent
  ncells *= hi[j]-lo[j]+1;             // Cumulative total cell count
}
                                       // The stride of each dimension depends
                                       // on *everything* before it
for (int j=1;j<dims;j++) extent[j].st = extent[j-1].st * extent[j-1].wi;

cell0 = new _N[ncells];                // Heave the linear memory into existence

for (int i=0;i<ncells;i++)             // Create the X-link data structure
  for (int j=0;j<dims;j++) cell0[i].links.push_back(pair<_L *,_L *>(0,0));

for (int i=0;i<ncells;i++) {           // And walk it...
//  printf("Cell %d, link.size() = %d\n",i,cell0[i].links.size());
  cell0[i].id = i;                     // So we can go ptr->array location
  for (int j=0;j<dims;j++) {           // For each dimension

    int st = extent[j].st;             // Unpack the stride
    int wi = extent[j].wi;             // And the width

    int f = (i%(wi*st))/st;            // Index in this dimension

    int d = (wi-1)*st;                 // Wrap-around distance
                                       // Now compute the neighbour addresses,
                                       // ignoring the wrap flags
//    int adr1 = i + ((f==0   ) ? +d : -st);
    int adr2 = i + ((f==wi-1) ? -d : +st);
//    _N * c1 = &cell0[adr1];            // And the corresponding cell addresses,
    _N * c2 = &cell0[adr2];            // ignoring the wrap flags
    if (!extent[j].wr) {               // Now look at the flags....
//      if (f==0   ) c1=0;               // ... and mask the links if required
      if (f==wi-1) c2=0;
    }
        // And load the link data structure between cell [i] and cell *c2
    if (c2!=0) xLink(j,c2,&cell0[i]);
//    cell0[i].links.push_back(pair<_N *,_N *>(c1,c2));
  }
}
//Dump(true);
}

//------------------------------------------------------------------------------

void Ncube::Dump(bool full)
{
printf("Dumping Ncube %s\n",cname.c_str());
printf("cells : %d, dimensions : %d\n",ncells,getD());
for (unsigned int j=0;j<getD();j++)
  printf("dim:%03d, lo:%03d, hi:%03d, wr:%c, st:%03d, wi:%03d\n",
    j,extent[j].lo,extent[j].hi,extent[j].wr?'T':'F',extent[j].st,extent[j].wi);

if (!full) return;

printf("\n Ord: ( Coords         )");
for (unsigned int j=0;j<getD();j++) printf("   Nd[ Link]<Dim%1d>[ Link]  Nd",j);
printf("  Val \n");
printf("=====+=");
for (unsigned int j=0;j<getD();j++) printf("=====+");
for (unsigned int j=0;j<getD();j++) printf("============================+");
printf("====\n");

vector<int> tmp;
for (int i=0;i<ncells;i++) {
  printf("%04d: ",i);
  ord2vec(i,tmp);
  for (unsigned int j=0;j<getD();j++)
    printf("%c%04d%c",(j==0)?'(':' ',tmp[j],(j==getD()-1)?')':',');
  for (unsigned int j=0;j<getD();j++) {
    _N * c0 = cell0[i].L0(j);
    _N * c1 = cell0[i].L1(j);
    _L * l0 = cell0[i].Lab0(j);
    _L * l1 = cell0[i].Lab1(j);
    if (c0==0) printf(" ----[-----]");
    else printf(" %04d[%5s]",c0->id,l0->name.c_str());
    printf("<%04d>",i);
    if (c1==0) printf("[-----]----");
    else printf("[%5s]%04d",l1->name.c_str(),c1->id);
  }
  printf(" %04d\n",cell0[i].val);
}

}

//------------------------------------------------------------------------------

bool Ncube::chkvec(vector<int> & crds)
{
if ((unsigned int)(crds.size())!=getD()) return false;
for (unsigned int i=0;i<getD();i++) {
  if (crds[i] < extent[i].lo) return false;
  if (crds[i] > extent[i].hi) return false;
}
return true;
}

//------------------------------------------------------------------------------

int Ncube::adr2ord(_N * in,int & out)
{
if (in==0) return out = -1;
return out = in->id;
}

//------------------------------------------------------------------------------

Ncube::_N * Ncube::ord2adr(int in,_N *& out)
{
if (in<0) return out = 0;
if (in>size()) return out = 0;
return out = &cell0[in];
}

//------------------------------------------------------------------------------

vector<int> & Ncube::ord2vec(int in,vector<int> & out)
{
out.clear();
if (in<0) return out;
if (in>size()) return out;
for (unsigned int j=0;j<getD();j++)
  out.push_back(((in%(extent[j].wi*extent[j].st))/extent[j].st)+extent[j].lo);
return out;
}

//------------------------------------------------------------------------------

int Ncube::vec2ord(vector<int> crds,int & out)
{
if (!chkvec(crds)) return out = -1;
out = 0;
for (unsigned int i=0;i<getD();i++) {
  out += (crds[i]-extent[i].lo) * extent[i].st;
}
return out;
}

//------------------------------------------------------------------------------

vector<int> & Ncube::vecADDvec(vector<int> crds,vector<int> offset,vector<int> & out)
// If the offset takes you out of the space, the out vector is returned empty
{
out.clear();                           // Default looney
if (!chkvec(crds)) return out;
if ((unsigned int)(offset.size())!=getD()) return out;
for (unsigned int i=0;i<getD();i++)
  if(!vecADDscl2(crds,i,offset[i],out)) {
    out.clear();
    return out;
  }
return out;

}

//------------------------------------------------------------------------------

vector <int> &  Ncube::vecSUBvec(vector<int> crds,vector<int> offset,vector<int> & out)
{
out.clear();
if (!chkvec(crds)) return out;
if ((unsigned int)(offset.size())!=getD()) return out;
WALKVECTOR(int,offset,i) (*i) = -(*i);
vecADDvec(crds,offset,out);
return out;
}

//------------------------------------------------------------------------------

vector <int> &  Ncube::vecADDscl(vector<int> crds,int indim,int inoff,vector<int> & out)
{
out.clear();                           // Default looney
if (!chkvec(crds)) return out;         // Looney trap
if ((indim<0)||(indim>int(getD()))) return out;
                                       // If we have a valid answer, go
if (vecADDscl2(crds,indim,inoff,out)) return out;
out.clear();                           // No, so clear the output vector
return out;
}

//------------------------------------------------------------------------------

bool Ncube::vecADDscl2(vector<int> crds,int indim,int inoff,vector<int> & out)
{
crds[indim] -= extent[indim].lo;       // Normalise the input
int p = crds[indim]+inoff;             // Handle the offset
if ((p>=0)&&(p<extent[indim].wi))      // p in range ?
  out.push_back(p + extent[indim].lo); // Yes, push out the renormalised answer
else {                                 // p not in range
  if (extent[indim].wr) {              // Wrap on ?
    p %= extent[indim].wi;             // Yes, modulusification
    if (p<0) p += extent[indim].wi;    // Fix the -ve answer
    out.push_back(p+extent[indim].lo); // Push out the renormalised answer
  }
  else return false;                   // Wrap off AND overflow
}
return true;
}

//------------------------------------------------------------------------------

vector <int> &  Ncube::vecSUBscl(vector<int> crds,int indim,int inoff,vector<int> & out)
{
return vecADDscl(crds,indim,-inoff,out);
}

//------------------------------------------------------------------------------

void Ncube::getN(_N * p,int i,_N ** L0,_N ** L1)
// Given a cell (p) and a dimension (i), retrun the two neighbours in that
// dimension. I can't use _N *& for the arguments in case i is out of range and
// I have to return null pointers.
{
*L0 = *L1 = 0;                         // Error condition
if (i>int(getD())) return;
*L0 = p->L0(i);                        // Valid data.....
*L1 = p->L1(i);
return;
}

//------------------------------------------------------------------------------

void Ncube::getL(_N * p,int i,_L ** L0,_L ** L1)
// Given a cell (p) and a dimension (i), return the two neighbours in that
// dimension. I can't use _N *& for the arguments in case i is out of range and
// I have to return null pointers.
{
*L0 = *L1 = 0;                         // Error condition
if (i>int(getD())) return;
*L0 = p->Lab0(i);                      // Valid data.....
*L1 = p->Lab1(i);
return;
}

//------------------------------------------------------------------------------

void Ncube::xLink(int d,_N * c0,_N * c2)
// Routine to build the cross link structure between nodes c0 and c2 in
// dimension d.
// The link data structures have been created, but just contain nulls at the
// moment.
{
//printf("\nInside xLink: d = %d, c0->id = %d, c2->id = %d\n",d,c0->id,c2->id);
//printf("c0->links.size() = %d, c2->links.size() = %d\n",
//             c0->links.size(),c2->links.size());
_L * pl = new _L(c0,c2);               // New link core
c0->links[d].first = pl;
c2->links[d].second = pl;
garbage.push_back(pl);                 // For the destructor
}

//------------------------------------------------------------------------------

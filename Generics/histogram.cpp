//------------------------------------------------------------------------------

#include "histogram.h"
#include "macros.h"

//==============================================================================

Histogram::Histogram(unsigned b)
{
binsiz=b;
}

//------------------------------------------------------------------------------

void Histogram::Add(unsigned d,int n)
// Add another value to the histogram, n times
{
D[d/binsiz] += n;
}

//------------------------------------------------------------------------------

unsigned Histogram::Bin()
// Return the bin size
{
return binsiz;
}

//------------------------------------------------------------------------------

unsigned Histogram::Bin(unsigned b)
// Change bin size; return old value. As this doesn't change the existing bin
// contents, its usefullness is infrequent. Use only if you really (a) know what
// you're doing and (b) mean it
{
unsigned old = binsiz;
binsiz = b;
return old;
}

//------------------------------------------------------------------------------

void Histogram::Dump(FILE * fp)
{
fprintf(fp,"---------------------------\n");
fprintf(fp,"Bin size %u, bin count %u\n",binsiz,D.size());
unsigned fr,to;
MinBin(fr,to);
printf("MinBin [%4u-%-4u] : %5d\n",fr,to,(*(D.begin())).second);
MaxBin(fr,to);
printf("MaxBin [%4u-%-4u] : %5d\n",fr,to,(*(--D.end())).second);
printf("Mean bin size      : %10.2e\n",Mean());
printf("Variance           : %10.2e\n",Variance());
unsigned cnt = 0;
WALKMAP(unsigned,int,D,i)
  fprintf(fp,"%03u:[%4u-%-4u] : %5d\n",
              cnt++,(*i).first*binsiz,((*i).first*binsiz)+binsiz-1,(*i).second);
fprintf(fp,"---------------------------\n");
}

//------------------------------------------------------------------------------

int Histogram::Get(unsigned d)
// Routine to establish the current value of the histogram at the data value 'd'
{
return D[d/binsiz];
}

//------------------------------------------------------------------------------

unsigned Histogram::InBin(unsigned key,unsigned & rfr,unsigned & rto)
// Routine to establish IF key is in the histogram, and if so, where.
// Returns the number of items in that bin
{
rfr = (key/binsiz) * binsiz;           // Lower bin range
rto = rfr + binsiz - 1;                // Upper bin range
map<unsigned,int>::iterator it = D.find(key/binsiz);
return it==D.end() ? 0 : D[key/binsiz];
}

//------------------------------------------------------------------------------

unsigned Histogram::Key(unsigned d)
// Routine to return the key value associated with the data value 'd'
{
return d/binsiz;
}

//------------------------------------------------------------------------------

void Histogram::MaxBin(unsigned & rfr,unsigned & rto)
// Return the extents of the largest bin in the object
{
rfr = (*(--D.end())).first * binsiz;
rto = rfr + binsiz - 1;
}

//------------------------------------------------------------------------------

double Histogram::Mean()
// Guess
{
double total = 0.0;
WALKMAP(unsigned,int,D,i) total += double((*i).second);
return total/double(D.size());
}

//------------------------------------------------------------------------------

void Histogram::MinBin(unsigned & rfr,unsigned & rto)
// Return the extents of the smallest bin in the object
{
rfr = (*D.begin()).first * binsiz;
rto = rfr + binsiz - 1;
}

//------------------------------------------------------------------------------

void Histogram::Pad()
// As created, the map only has entries for non-0 data counts.
// Which may be what you want.
// This routine walks from the lowest key value to the highest key value,
// and references all the key values in between. This has the side-effect of
// *creating* a {key,0} pair in the map if it wasn't there already, so when you
// Dump() it, you get a uniform field.
// STL maps are sorted associative containers, but we don't rely on that here.
// Is this dirty or elegant? Can't decide.
{
for(unsigned i=(*D.begin()).first;i<(*(--D.end())).first;i++) D[i];
}

//------------------------------------------------------------------------------

void Histogram::ReBin(unsigned nbin)
// Change the granularity of the histogram. To do this, we walk the data, and
// build a second temporary histogram from it. Then swapsy, and delete the
// temporary.
{
Histogram *pHist = new Histogram(nbin);// Create the temporary
WALKMAP(unsigned,int,D,i) pHist->Add(binsiz*(*i).first,(*i).second);
D      = pHist->D;                     // Overwrite *this with temporary data
binsiz = pHist->binsiz;
delete pHist;                          // Kill the temporary
}

//------------------------------------------------------------------------------

void Histogram::Reset()
{
D.clear();
binsiz=1;
}

//------------------------------------------------------------------------------

void Histogram::Unpad()
// This is the reverse of the Pad() function. It walks the map and removes
// any {key,data} pairs with data==0
// You have to do it twice because if you have a contiguous set of zero keys
// you can end up with an orphan {x,0} pair left in the middle.
// OK, I could walk it myself and pull out the 0 expicitly, but this works, and
// it's not clear to me which is quicker anyway.
{
WALKMAP(unsigned,int,D,i) if ((*i).second==0) D.erase(i);
WALKMAP(unsigned,int,D,i) if ((*i).second==0) D.erase(i);
}

//------------------------------------------------------------------------------

double Histogram::Variance()
// Guess
{
double v = 0.0;
double m = Mean();
WALKMAP(unsigned,int,D,i) {
  double val = double((*i).second);
  v += ((m-val)*(m-val));
}

return v/(double)(D.size());
}

//------------------------------------------------------------------------------

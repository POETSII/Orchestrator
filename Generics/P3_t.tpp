//------------------------------------------------------------------------------

#include <stdio.h>
#include <math.h>
#include <iostream.h>
#include <algorithm>
#include "macros.h"
using namespace std;

//==============================================================================

template<typename T> P3_t<T>::P3_t()
{
x = y = z = T();
pBox = (Box_t<T> *)0;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T>::P3_t(T _x,T _y,T _z)
{
x = _x;
y = _y;
z = _z;
pBox = (Box_t<T> *)0;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T>::P3_t(T _p[3])
{
x = _p[0];
y = _p[1];
z = _p[2];
pBox = (Box_t<T> *)0;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T>::P3_t(const P3_t<T> & rP)
{
x = rP.x;
y = rP.y;
z = rP.z;
pBox = rP.pBox;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T>::~P3_t()
// DON'T kill any box here - it might be shared....
{
}

//------------------------------------------------------------------------------

template<typename T> T P3_t<T>::Abs(T r)
{
if (r>T()) return r;
return -r;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> P3_t<T>::Abs(P3_t & rP)
{
P3_t<T> ans rP;
ans.x = P3_t<int>::Abs(ans.x);
ans.y = P3_t<int>::Abs(ans.y);
ans.z = P3_t<int>::Abs(ans.z);
return ans;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> P3_t<T>::Ang(P3_t & rP)
// Return the 'in-box' angle between the points
{
P3_t dij = *this - rP;
if (pBox== (Box_t<T> *)0) return dij;
double xsize = pBox->xsize;
dij.x -= xsize*nint(dij.x/xsize);
double ysize = pBox->ysize;
dij.y -= ysize*nint(dij.y/ysize);
double zsize = pBox->zsize;
dij.z -= zsize*nint(dij.z/zsize);
return dij;
}

//------------------------------------------------------------------------------

template<typename T> T P3_t<T>::Dist(P3_t & rP)
{
return (*this - rP).Mag();
}

//------------------------------------------------------------------------------

template<typename T> void P3_t<T>::Dump()
// Have to use streams 'cos we don't know the type
{
cout << "{x,y,z} = {" << x << "," << y << "," << z << "}";
if (pBox== (Box_t<T> *)0) printf(" no box\n");
else {
  printf("\n");
  pBox->Dump();
}
}

//------------------------------------------------------------------------------

template<typename T> void P3_t<T>::Dump1(string eol)
// Have to use streams 'cos we don't know the type
{
cout << "{" << x << "," << y << "," << z << "}";
printf("%s",eol.c_str());
}

//------------------------------------------------------------------------------

template<typename T> bool P3_t<T>::Empty()
{
if (x!=T()) return false;
if (y!=T()) return false;
if (z!=T()) return false;
return true;
}

//------------------------------------------------------------------------------

template<typename T> int P3_t<T>::Fprn(FILE * fp)
{
return fprintf(fp,"%10.2e %10.2e %10.2e ",x,y,z);
}

//------------------------------------------------------------------------------

template<typename T> T P3_t<T>::Mag()
{
return sqrt((x*x) + (y*y) + (z*z));
}

//------------------------------------------------------------------------------

template<typename T> void P3_t<T>::P3(T x_,T y_,T z_)
{
x = x_;
y = y_;
z = z_;
}

//------------------------------------------------------------------------------

template<typename T> void P3_t<T>::P3(T * px,T * py,T * pz)
// So much for data hiding (sigh)
{
*px = x;
*py = y;
*pz = z;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> P3_t<T>::operator+(const P3_t & rP)
{
P3_t ans;
ans.x    = x + rP.x;
ans.y    = y + rP.y;
ans.z    = z + rP.z;
ans.pBox = pBox;
if (pBox!=(Box_t<T> *)0) pBox->Box(*this);
return ans;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> P3_t<T>::operator-(const P3_t & rP)
{
P3_t ans;                              // Dummy answer
ans.x     = x - rP.x;                  // {x,y,z} coordinates
ans.y     = y - rP.y;
ans.z     = z - rP.z;
ans.pBox  = pBox;
if (pBox!=(Box_t<T> *)0) pBox->Box(*this);
return ans;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> P3_t<T>::operator*(const P3_t & rP)
// Scalar product
{
P3_t ans;
ans.x    = x * rP.x;
ans.y    = y * rP.y;
ans.z    = z * rP.z;
ans.pBox = pBox;
if (pBox!=(Box_t<T> *)0) pBox->Box(*this);
return ans;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> P3_t<T>::operator/(const P3_t & rP)
{
P3_t ans;
ans.x    = x / rP.x;
ans.y    = y / rP.y;
ans.z    = z / rP.z;
ans.pBox = pBox;
if (pBox!=(Box_t<T> *)0) pBox->Box(*this);
return ans;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> P3_t<T>::operator%(const P3_t & rP)
{
P3_t ans;
ans.x    = x % rP.x;
ans.y    = y % rP.y;
ans.z    = z % rP.z;
ans.pBox = pBox;
if (pBox!=(Box_t<T> *)0) pBox->Box(*this);
return ans;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> P3_t<T>::operator*(const T r)
{
P3_t ans;
ans.x    = x * r;
ans.y    = y * r;
ans.z    = z * r;
ans.pBox = pBox;
if (pBox!=(Box_t<T> *)0) pBox->Box(*this);
return ans;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> P3_t<T>::operator/(const T r)
{
P3_t ans;
ans.x = x / r;
ans.y = y / r;
ans.z = z / r;
ans.pBox = pBox;
if (pBox!=(Box_t<T> *)0) pBox->Box(*this);
return ans;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> P3_t<T>::operator%(const T r)
{
printf("NOT IMPLEMENTED\n");
return r;
}

//------------------------------------------------------------------------------

template<typename T> T P3_t<T>::operator^(const int p)
// Integer exponentiation
{
double mag2 = Mag();
return pow(mag2,p);
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> P3_t<T>::operator=(const P3_t & rP)
{
if (*this == rP) return *this;
x    = rP.x;
y    = rP.y;
z    = rP.z;
pBox = rP.pBox;
if (pBox!=(Box_t<T> *)0) pBox->Box(*this);
return *this;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> & P3_t<T>::operator=(const T r)
{
x = y = z = r;
if (pBox!=(Box_t<T> *)0) pBox->Box(*this);
return *this;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> P3_t<T>::operator+=(const P3_t & rP)
{
x += rP.x;
y += rP.y;
z += rP.z;
if (pBox!=(Box_t<T> *)0) pBox->Box(*this);
return *this;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> P3_t<T>::operator+=(const T r)
{
x += r;
y += r;
z += r;
if (pBox!=(Box_t<T> *)0) pBox->Box(*this);
return *this;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> P3_t<T>::operator-=(const P3_t & rP)
{
x -= rP.x;
y -= rP.y;
z -= rP.z;
if (pBox!=(Box_t<T> *)0) pBox->Box(*this);
return *this;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> P3_t<T>::operator-=(const T r)
{
x -= r;
y -= r;
z -= r;
if (pBox!=(Box_t<T> *)0) pBox->Box(*this);
return *this;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> P3_t<T>::operator*=(const T r)
{
x *= r;
y *= r;
z *= r;
if (pBox!=(Box_t<T> *)0) pBox->Box(*this);
return *this;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> P3_t<T>::operator/=(const T r)
{
x /= r;
y /= r;
z /= r;
if (pBox!=(Box_t<T> *)0) pBox->Box(*this);
return *this;
}

//------------------------------------------------------------------------------

template<typename T> P3_t<T> P3_t<T>::operator%=(const T m)
{
x = fmod(x,m);
y = fmod(y,m);
z = fmod(z,m);
if (pBox!=(Box_t<T> *)0) pBox->Box(*this);
return *this;
}

//------------------------------------------------------------------------------

template<typename T> bool P3_t<T>::operator==(const P3_t & r)
{
if (x!=r.x)        return false;
if (y!=r.y)        return false;
if (z!=r.z)        return false;
if (pBox!= r.pBox) return false;
return true;
}

//------------------------------------------------------------------------------

template<typename T> bool P3_t<T>::operator!=(const P3_t & r)
{
return !(*this==r);
}

//------------------------------------------------------------------------------

template<typename T> bool P3_t<T>::operator> (const T r)
// Establishes if any component of the point is > r
{
if (P3_t<T>::Abs(x)>r) return true;
if (P3_t<T>::Abs(y)>r) return true;
if (P3_t<T>::Abs(z)>r) return true;
return false;
}

//------------------------------------------------------------------------------

template<typename T> bool P3_t<T>::operator< (const T r)
// Establishes if any component of the point is < r
{
if (P3_t<T>::Abs(x)<r) return true;
if (P3_t<T>::Abs(y)<r) return true;
if (P3_t<T>::Abs(z)<r) return true;
return false;
}

//------------------------------------------------------------------------------




//==============================================================================

template<typename T2> Box_t<T2>::Box_t(T2 x0,T2 y0,T2 z0,T2 x1,T2 y1,T2 z1)
// Establish the bounds of the box....
{
Box(x0,y0,z0,x1,y1,z1);
}

//------------------------------------------------------------------------------

template<typename T2> Box_t<T2>::Box_t(T2 x1,T2 y1,T2 z1)
// Establish the bounds of the box....
{
Box(0,0,0,x1,y1,z1);
}

//------------------------------------------------------------------------------

template<typename T2> Box_t<T2>::Box_t(P3_t<T2> p)
// Establish the bounds of the box....
{
T2 x1,y1,z1;
p.P3(&x1,&y1,&z1);
Box(0,0,0,x1-1,y1-1,z1-1);
}

//------------------------------------------------------------------------------

template<typename T2> void Box_t<T2>::Box(P3_t<T2> & pt)
// Make sure the point is in the box....
// This is the routine that does all the work
// This routine gets used quite a lot; I don't use modulus because it's a non-
// specialised template and quite honestly I couldn't be bothered to fiddle
// with ranges and stuff. Equally, I don't want to waste a lot of time idiot-
// proofing it - IF YOU PUT DUD DATA IN, IT'S INCREDIBLY EASY TO LOCK IT INTO A
// SPIN LOOP
{
while (pt.x > xmax) pt.x -= xsize;
while (pt.x < xmin) pt.x += xsize;
while (pt.y > ymax) pt.y -= ysize;
while (pt.y < ymin) pt.y += ysize;
while (pt.z > zmax) pt.z -= zsize;
while (pt.z < zmin) pt.z += zsize;
return;
}

//------------------------------------------------------------------------------

template<typename T2> void Box_t<T2>::Box(T2 x0,T2 y0,T2 z0,T2 x1,T2 y1,T2 z1)
// Establish the bounds of the box....
{
xmin  = min<T2>(x0,x1);
ymin  = min<T2>(y0,y1);
zmin  = min<T2>(z0,z1);
xmax  = max<T2>(x0,x1);
ymax  = max<T2>(y0,y1);
zmax  = max<T2>(z0,z1);
xsize = ABS(x0,x1)+1; //x0>x1 ? x0-x1 : x1-x0;
ysize = ABS(y0,y1)+1; // ? y0-y1 : y1-y0;
zsize = ABS(z0,z1)+1; // ? z0-z1 : z1-z0;
}

//------------------------------------------------------------------------------

template<typename T2> void Box_t<T2>::Box(T2 * px0,T2 * py0,T2 * pz0,
                                        T2 * px1,T2 * py1,T2 * pz1)
// Output the bounds of the box....
{
* px0 = xmin;
* py0 = ymin;
* pz0 = zmin;
* px1 = xmax;
* py1 = ymax;
* pz1 = zmax;
}

//------------------------------------------------------------------------------

template<typename T2> void Box_t<T2>::Dump()
{
cout << "xmin,xmax,xsize = " << xmin << "," << xmax << "," << xsize << "\n";
cout << "ymin,ymax,ysize = " << ymin << "," << ymax << "," << ysize << "\n";
cout << "zmin,zmax,zsize = " << zmin << "," << zmax << "," << zsize << "\n";
}

//------------------------------------------------------------------------------

template<typename T2> void Box_t<T2>::Dump1(string eol)
{
cout << "xmin,xmax,xsize = " << xmin << "," << xmax << "," << xsize << ":";
cout << "ymin,ymax,ysize = " << ymin << "," << ymax << "," << ysize << ":";
cout << "zmin,zmax,zsize = " << zmin << "," << zmax << "," << zsize << ":";
printf("%s",eol.c_str());
}

//==============================================================================

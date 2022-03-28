#ifndef __P3_tH__H
#define __P3_tH__H

#include <stdio.h>
#include <string>
using namespace std;

//==============================================================================

template <typename T2> class P3_t;

template <typename T2> class Box_t
{
public:
                  Box_t(){}
                  Box_t(T2,T2,T2,T2,T2,T2);
                  Box_t(T2,T2,T2);
                  Box_t(P3_t<T2>);
virtual ~         Box_t(){}

void              Box(P3_t<T2> &);
void              Box(T2,T2,T2,T2,T2,T2);
void              Box(T2 *,T2 *,T2 *,T2 *,T2 *,T2 *);
void              Dump();
void              Dump1(string = "");

private:
T2                xmin,xmax;
T2                ymin,ymax;
T2                zmin,zmax;
T2                xsize,ysize,zsize;

};

//==============================================================================

template <typename T> class P3_t
{
template <typename T> friend class Box_t;
public:
                  P3_t();
                  P3_t(T,T,T=T());
                  P3_t(T[3]);
                  P3_t(const P3_t<T> &);
virtual ~         P3_t();

static T          Abs(T);
P3_t              Abs(P3_t &);
P3_t              Ang(P3_t &);
void              Box(Box_t<T> * pb=0)           { pBox = pb;          }
T                 Dist(P3_t &);
void              Dump();
void              Dump1(string = "");
bool              Empty();
int               Fprn(FILE *);
T                 Mag();
void              P3(T,T,T);
void              P3(T *,T *,T *);
T                 Size()                         { return x*y*z;       }
void              Zero()                         { x = y = z = T();    }

P3_t              operator+ (const P3_t &);
P3_t              operator- (const P3_t &);
P3_t              operator* (const P3_t &);
P3_t              operator/ (const P3_t &);
P3_t              operator% (const P3_t &);
P3_t              operator* (const T);
P3_t              operator/ (const T);
P3_t              operator% (const T);
T                 operator^ (const int);
P3_t              operator= (const P3_t &) ;
P3_t &            operator= (const T);
P3_t              operator+=(const P3_t &);
P3_t              operator+=(const T);
P3_t              operator-=(const P3_t &);
P3_t              operator-=(const T);
P3_t              operator*=(const T);
P3_t              operator/=(const T);
P3_t              operator%=(const T);
bool              operator==(const P3_t &);
bool              operator!=(const P3_t &);
bool              operator> (const T);
bool              operator< (const T);

T                 x,y,z;
Box_t<T> *        pBox;

};

//==============================================================================

#include "P3_t.tpp"
#endif

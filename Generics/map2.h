#ifndef __map2__H
#define __map2__H

#include <assert.h>
#include <typeinfo>
#include <map>
using namespace std;

//==============================================================================

#define MAP2_ template<class T1,class T2>
#define _MAP2 map2<T1,T2>

MAP2_ class map2 {
public : map2();
public : map2(const map2 &);
public : T2 Add(T1,T2);
public : T1 Add(T2,T1);
public : void clear();
public : bool Has(T1);
public : bool Has(T2);
public : unsigned int size();
public : T1 & operator[](T2);
public : T2 & operator[](T1);

private: map <T1,T2> T1_2_T2_map;
private: map <T2,T1> T2_2_T1_map;
private: T1 T10;
private: T2 T20;

};

#include "map2.tpp"                 // Go get the code
#undef MAP2_
#undef _MAP2

//==============================================================================

#endif

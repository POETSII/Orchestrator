//------------------------------------------------------------------------------

#ifndef __macros__H
#define __macros__H

#define za_start(xp,fmt) (xp = (((unsigned char *)fmt) + sizeof(*fmt)))

#define ABS(X,Y) ((X)>(Y) ? ((X)-(Y)) : ((Y)-(X)))

#define BYTESPERUNSIGNED 4

#define TN typename
#if (defined  _MSC_VER || defined __BORLANDC__)
#undef  TN
#define TN
#endif

#define WALKDEQUE(T,v,i) \
        for(TN deque<T>::iterator i=v.begin();i!=v.end();i++)
#define WALKVECTOR(T,v,i) \
        for(TN vector<T>::iterator i=v.begin();i!=v.end();i++)
#define WALKCVECTOR(T,v,i) \
        for(TN vector<T>::const_iterator i=v.begin();i!=v.end();i++)
#define WALKVECTORREVERSE(T,v,i) \
        for(TN vector<T>::reverse_iterator i=v.rbegin();i!=v.rend();i++)
#define WALKLIST(T,v,i) \
        for(TN list<T>::iterator i=v.begin();i!=v.end();i++)
#define WALKSET(T,v,i) \
        for(TN set<T>::iterator i=v.begin();i!=v.end();i++)
#define WALKSET2(T,op,v,i) \
        for(TN set<T,op>::iterator i=v.begin();i!=v.end();i++)
#define WALKMAP(KT,DT,v,i) \
        for(TN map<KT,DT>::iterator i=v.begin();i!=v.end();i++)
#define WALKMAPop(KT,DT,op,v,i) \
        for(TN map<KT,DT,op>::iterator i=v.begin();i!=v.end();i++)
#define WALKMULTIMAP(KT,DT,v,i) \
        for(TN multimap<KT,DT>::iterator i=v.begin();i!=v.end();i++)
/*
#define TWALKDEQUE(T,v,i) \
        for(typename deque<T>::iterator i=v.begin();i!=v.end();i++)
#define TWALKVECTOR(T,v,i) \
        for(typename vector<T>::iterator i=v.begin();i!=v.end();i++)
#define TWALKCVECTOR(T,v,i) \
        for(typename vector<T>::const_iterator i=v.begin();i!=v.end();i++)
#define TWALKVECTORREVERSE(T,v,i) \
        for(typename vector<T>::reverse_iterator i=v.rbegin();i!=v.rend();i++)
#define TWALKLIST(T,v,i) \
        for(typename list<T>::iterator i=v.begin();i!=v.end();i++)
#define TWALKSET(T,v,i) \
        for(typename set<T>::iterator i=v.begin();i!=v.end();i++)
#define TWALKSET2(T,op,v,i) \
        for(typename set<T,op>::iterator i=v.begin();i!=v.end();i++)
#define TWALKMAP(KT,DT,v,i) \
        for(typename map<KT,DT>::iterator i=v.begin();i!=v.end();i++)
#define TWALKMULTIMAP(KT,DT,v,i) \
        for(typename multimap<KT,DT>::iterator i=v.begin();i!=v.end();i++)
*/
#endif

//--------------------------------------------------------------------------------


#ifndef __MDecoderH__H
#define __MDecoderH__H

#include "msg_p.hpp"
#include <stdio.h>
#include <string>
#include <map>
using namespace std;
//class Root;
class CommonBase;

//==============================================================================

class MDecoder
{
public:

                       MDecoder(void *);
virtual ~              MDecoder();
unsigned               Decode(Msg_p *);
void                   Dump(FILE * = stdout);

//void                   MPIcallback(string &);
//static bool            MPIcallback(void *,string &);

void * par;

typedef unsigned (CommonBase::*pMeth)(Msg_p *);
//typedef unsigned (Root::*pMeth)(Msg_p *);

map<unsigned,pMeth> FnMap2;





};

//==============================================================================

//#include "mdecoder.tpp"
#endif

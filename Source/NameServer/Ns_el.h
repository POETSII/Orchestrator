#ifndef __Ns_elH__H
#define __Ns_elH__H

#include "PMsg_p.hpp"
#include "P_addr.h"
#include "NameBase.h"
#include <string>
#include <set>
using namespace std;

//==============================================================================

class Ns_0;
class Ns_el : public PMsg_p
{
public:
                 Ns_el();
                 Ns_el(byte *,int);
                 Ns_el(PMsg_p *);
virtual ~        Ns_el();

vector<Ns_0 *> * Construct();
void             Dump(unsigned = 0,FILE * = stdout);
void             Dump0(unsigned = 0,FILE * = stdout,unsigned = 0);
int              Length();
void             PutD(string,string,string,string,string,vector<string>,
                      vector<string>,vector<string>,vector<string>,unsigned,
                      P_addr_t,unsigned,unsigned);
void             PutO(string,unsigned);
void             PutS(string,string,string,unsigned,P_addr_t,unsigned,unsigned);
void             PutT(string,string,unsigned);

struct Dindex_t {
  unsigned key;
  P_addr_t addr;
  unsigned attr;
  unsigned bin;
  unsigned inP;
  unsigned ouP;
};
struct Oindex_t {
  unsigned key;
};
struct Sindex_t {
  unsigned key;
  P_addr_t addr;
  unsigned attr;
  unsigned bin;
};
struct Tindex_t {
  unsigned key;
};

vector<unsigned> keyv;

};

//==============================================================================

class Ns_0 : public NameBase
{
public:
                 Ns_0();
virtual ~        Ns_0(){}

virtual void     Dump(unsigned = 0,FILE * = stdout);

unsigned         key;
unsigned char    type;
string           Ename;

};

//==============================================================================

class Ns_X : public Ns_0
{
public:
                 Ns_X(string = ""){}
Ns_X *           operator()(string);
virtual ~        Ns_X(){}

private:
string           msg;

protected:
virtual void     Dump(unsigned = 0,FILE * = stdout){}
};

//==============================================================================

class Ns_dev : public Ns_0
{
public:
                 Ns_dev(Ns_el::Dindex_t *,vector<string> &);
virtual ~        Ns_dev(){}

protected:
virtual void     Dump(unsigned = 0, FILE * = stdout);

private:
string           Etype;
string           Sname;
string           Tname;
string           Oname;
P_addr_t         addr;
unsigned         attr;
unsigned         bin;
vector<string>   inpin;
vector<string>   inpintype;
vector<string>   oupin;
vector<string>   oupintype;
};

//==============================================================================

class Ns_sup : public Ns_0
{
public:
                 Ns_sup(Ns_el::Sindex_t *,vector<string> &);
virtual ~        Ns_sup(){}

protected:
virtual void     Dump(unsigned = 0,FILE * = stdout);

private:
string           Tname;
string           Oname;
P_addr_t         addr;
unsigned         attr;
unsigned         bin;

};

//==============================================================================

class Ns_tsk : public Ns_0
{
public:
                 Ns_tsk(Ns_el::Tindex_t *,vector<string> &);
virtual ~        Ns_tsk(){}

protected:
virtual void     Dump(unsigned = 0,FILE * = stdout);

private:
string           Oname;

};

//==============================================================================

class Ns_own : public Ns_0
{
public:
                 Ns_own(Ns_el::Oindex_t *,vector<string> &);
virtual ~        Ns_own(){}

protected:
virtual void     Dump(unsigned = 0,FILE * = stdout);
};

//==============================================================================

#endif





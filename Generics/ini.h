#ifndef __INI__H
#define __INI__H

#include "flat.h"
#include "uif.h"

//==============================================================================

class S2 {

public:
                   S2(string ss1,string ss2):_s1(ss1),_s2(ss2){}
                   S2(string ss1):_s1(ss1),_s2(""){}
                   S2(const S2 & y):_s1(y._s1),_s2(y._s2){}
                   S2():_s1(""),_s2(""){}
virtual ~          S2(void){}
string &           s1(){return _s1;}
string &           s2(){return _s2;}
void               Dump() {
                     printf("|%s|(|%s|)\n",s1().c_str(),s2().c_str());
                   }
bool               operator == (S2 &);

private:
string             _s1;
string             _s2;
};

//==============================================================================

class S4 {

public:
                   S4(string ss1,string ss2,string ss3,string ss4):
                     _s1(ss1),_s2(ss2),_s3(ss3),_s4(ss4){}
                   S4(const S4 & y):_s1(y._s1),_s2(y._s2),_s3(y._s3),_s4(y._s4){}
                   S4():_s1(""),_s2(""),_s3(""),_s4(""){}
                   S4(string s,int p):_s1(""),_s2(""),_s3(""),_s4("") {
                     switch (p) {
                       case 1 : _s1 = s; break;
                       case 2 : _s2 = s; break;
                       case 3 : _s3 = s; break;
                       case 4 : _s4 = s; break;
                     }
                   }
                   S4(S2 & a1,S2 & a2):_s1(a1.s1()),_s2(a1.s2()),_s3(a2.s1()),_s4(a2.s2()){}
virtual ~          S4(void){}

string &           s1(){return _s1;}
string &           s2(){return _s2;}
string &           s3(){return _s3;}
string &           s4(){return _s4;}
void               Clear() {_s1=_s2=_s3=_s4=string("");}
void               Dump() {
                     printf("%s(%s):%s(%s)\n",
                       s1().c_str(),s2().c_str(),
                       s3().c_str(),s4().c_str());
                   }
bool               operator == (S4 &);

private:
string             _s1;
string             _s2;
string             _s3;
string             _s4;
};

//==============================================================================

class INI : public UIF {

public:
                   INI(char *,void (*)(void *)=0,void (*)(void *,int)=0,void (*)(void *)=0,void (*)(void *)=0);        // File constructor
       //            INI(void (*)(void *)=0,void (*)(void *)=0,void (*)(void *)=0,void (*)(void *)=0);
                   INI(int,char **);   // Command line constructor
                   INI();
virtual ~          INI(void);

const static uchar INI_MATCH   = 0x01;
const static uchar INI_NOMATCH = 0x02;

static bool        Decode(string,Lex::Sytype &,vector<string> &);
static bool        Decode(void *,Lex::Sytype &,vector<string> &);
void               ECB();              // Error callback

vector<UIF::Node *>FindCommands(UIF::Node *);
Node *             FindSection(S2 &);
vector<S2>         GetSections();
vector<S2>         GetValues(S2 &,S4 &);
vector<S2>         GetValues(string &,string &);
vector<S2>         GetValues(char * &,char * &);
vector<S4>         GetVariables(S2 &);
//template<class T> static void Mask(vector<T> &,T,uchar=INI_MATCH);
static void        Mask(vector<S2> &,S2,uchar=INI_MATCH);
static void        Mask(vector<S4> &,S4,uchar=INI_MATCH);
bool               SectionExists(S2);
bool               SectionExists(string,string);
bool               SectionExists(string);
bool               VariableExists(S2 &,S4 &);
bool               VariableExists(string &,string &);
bool               VariableExists(char * &,char * &);

private:

S2                 GetSectName(Node *);
S2                 GetRecdLabl(Node *);
vector<S2>         GetRecdValu(Node *);
S2                 GetRecdVari(Node *);

static bool        Match(S2  ,S2  );
static bool        Match(S4  ,S4  );
};

//==============================================================================

#endif

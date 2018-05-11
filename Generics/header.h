//------------------------------------------------------------------------------

#ifndef __Header__H
#define __Header__H

#include <stdio.h>
#include <string>
#include <algorithm>
using namespace std;

//==============================================================================

class Header
{
public:
                Header();
                Header(string);
                Header(FILE *);
virtual ~       Header();

void            Author(string a)     {author = a;}
string          Author()             {return author;}
void            Dump();
int             Err()                {return err;}
void            Hname(string h)      {hname = h;}
string          Hname()              {return hname;}
int             Init(FILE *);
void            Name(string n)       {name = n;}
string          Name()               {return name;}
void            SaveA(FILE *);
void            SaveA(string);
void            SaveB(FILE *);
void            SaveB(string);
void            Tst_day(int d)       {tstamp.day = d;}
int             Tst_day()            {return tstamp.day;}
void            Tst_mnth(int m)      {tstamp.mnth = m;}
int             Tst_mnth()           {return tstamp.mnth;}
void            Tst_year(int y)      {tstamp.year = y;}
int             Tst_year()           {return tstamp.year;}
void            Tst_hour(int h)      {tstamp.hour = h;}
int             Tst_hour()           {return tstamp.hour;}
void            Tst_mins(int m)      {tstamp.mins = m;}
int             Tst_mins()           {return tstamp.mins;}
void            Tst_secs(int s)      {tstamp.secs = s;}
int             Tst_secs()           {return tstamp.secs;}
void            Tst_msecs(int ms)    {tstamp.msecs = ms;}
int             Tst_msecs()          {return tstamp.msecs;}
void            Tstamp(string);
string          Tstamp()             {return tstamp.Tstamp();}
void            Typ(unsigned t)      {ftyp = t;}
unsigned        Typ()                {return ftyp;}
void            Ver(int,int,int,int);
unsigned char * Ver()                {return ver;}
static void     wEOFB(FILE *);

private:
string          GetStr(FILE *);
void            PutStr(FILE *,string);

static const unsigned char Rtype;
static const unsigned int  UnSecSig;

string hname;
unsigned ftyp;
unsigned char ver[4];
struct timestamp {
  timestamp(){day=mnth=year=hour=mins=secs=msecs=0;};
  void GetTS(FILE *);
  void PutTS(FILE *);
  void Set();
  string Tstamp();
  int day;
  int mnth;
  int year;
  int hour;
  int mins;
  int secs;
  int msecs;
} tstamp;
string name;
string author;
int err;

};

//==============================================================================

#endif

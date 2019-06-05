#ifndef __NameBaseH__H
#define __NameBaseH__H

#include <stdio.h>
#include <string>
#include <map>
using namespace std;

//==============================================================================

class NameBase
{
public:
                                NameBase();
virtual ~                       NameBase();
string                          AutoName(string = "_");
void                            Dump(FILE * = stdout);
static NameBase *               Find(unsigned);
string                          FullName(unsigned = 1);
unsigned                        Id();
void                            Id(unsigned);
string                          Name();
void                            Name(string);
NameBase *                      Npar();
void                            Npar(NameBase *);
unsigned                        Uid();

private:
NameBase *                      npar;
string                          name;
unsigned                        id;
bool                            rtrap;
static unsigned                 uid;
static map<unsigned,NameBase *> NBmap;

};

//==============================================================================

#endif

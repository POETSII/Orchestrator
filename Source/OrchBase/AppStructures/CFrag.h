#ifndef __CFragH__H
#define __CFragH__H

#include "DS_XML.h"
#include <stdio.h>
#include <string>
using namespace std;

//==============================================================================

class CFrag
{
public:
                    CFrag();
                    CFrag(std::string src);
virtual ~           CFrag();

void                C_src(const string & rs){ c_src = rs;          }
string &            C_src()                 { return c_src;        }
void                Dump(unsigned = 0,FILE * = stdout);
void                Show(unsigned = 0,FILE * = stdout);
static void         preDS_XML(DS_XML::xnode *,DS_XML *);
static void         pstDS_XML(DS_XML::xnode *,DS_XML *);
unsigned            Size()                  { return c_src.size(); }

private:
string              c_src;

};

//==============================================================================

#endif

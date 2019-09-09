#ifndef __CMeta_tH__H
#define __CMeta_tH__H

#include <stdio.h>
#include <string>
#include <vector>
using namespace std;

//==============================================================================

class Meta_t
{
public:
              Meta_t();
              Meta_t(const vector<pair<string,string> > &);
virtual ~     Meta_t();
void          Dump(FILE * = stdout);

vector<pair<string,string> > vps;

};

//==============================================================================

#endif





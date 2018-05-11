#ifndef __Unrec_tH__H
#define __Unrec_tH__H

#include <string>
using namespace std;

//==============================================================================

class Unrec_t
{
public:
          Unrec_t(unsigned,string,string);
virtual ~ Unrec_t();
void      Post();

private:
unsigned  i;                           // Identifying code
string    d;                           // Derived (parent) class
string    r;                           // Throwing routine

};

//==============================================================================
   
#endif

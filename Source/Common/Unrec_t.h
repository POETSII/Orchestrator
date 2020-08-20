#ifndef __Unrec_tH__H
#define __Unrec_tH__H

#include <stdexcept>
#include <string>
using namespace std;

//==============================================================================

class Unrec_t : public runtime_error
{
public:
          Unrec_t(unsigned,string,string);
void      Post();

private:
unsigned  i;                           // Identifying code
string    d;                           // Derived (parent) class
string    r;                           // Throwing routine

};

//==============================================================================
   
#endif

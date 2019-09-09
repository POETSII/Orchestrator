#ifndef __CFragH__H
#define __CFragH__H

//#include "NameBase.h"
//#include "DefRef.h"

#include <stdio.h>
#include <string>
using namespace std;

//==============================================================================

class CFrag //: public NameBase, public DefRef
{
public:
                    CFrag();
                    CFrag(std::string src);
		    
virtual ~           CFrag();

void                Dump(FILE * = stdout);

string              c_src;

};

//==============================================================================

#endif





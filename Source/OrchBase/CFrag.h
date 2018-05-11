#ifndef __CFragH__H
#define __CFragH__H

#include <stdio.h>
#include <string>

//==============================================================================

class CFrag
{
public:
                    CFrag();
                    CFrag(std::string src);
		    
virtual ~           CFrag();

void                Dump(FILE * = stdout);

std::string              c_src;

};

//==============================================================================

#endif





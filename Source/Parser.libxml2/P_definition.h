#ifndef _P_DEFINITION_H_
#define _P_DEFINITION_H_

#include "relaxng.h"
#include "stdio.h"
#include "P_Pparser.h"

class P_definition : public P_Pparser
{
public:
              P_definition(OrchBase*, string file="");
    virtual   ~P_definition();	 

    void      Load(string file);
    void      Validate();
    int       ParseNextSubObject();
    
private:

    FILE*     xmlDef;
};

#endif

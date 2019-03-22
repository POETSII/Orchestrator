#ifndef _P_GRAPHTYPEREFERENCE_H_
#define _P_GRAPHTYPEREFERENCE_H_

#include "P_Graphs.h"

class P_GraphTypeReference: public P_Graphs
{
  
public:

      P_GraphTypeReference(OrchBase*);
      ~P_GraphTypeReference();

      int       InsertSubObject(string);
      int       SetObjectProperty(string, string);

private:

             string graphTypeID;

static const char* attrs_array[3];
static const set<string>attributes;
 
};

#endif

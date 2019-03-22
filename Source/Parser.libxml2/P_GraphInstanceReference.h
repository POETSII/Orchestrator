#ifndef _P_GRAPHINSTANCEREFERENCE_H_
#define _P_GRAPHINSTANCEREFERENCE_H_

#include "P_Graphs.h"

class P_GraphInstanceReference: public P_Graphs
{
  
public:

      P_GraphInstanceReference(OrchBase*);
      ~P_GraphInstanceReference();

      int       InsertSubObject(string);
      int       SetObjectProperty(string, string);

private:

             string graphInstanceID;

static const char* attrs_array[3];
static const set<string>attributes;
 
};

#endif

#ifndef _P_DATADEF_H_
#define _P_DATADEF_H_

// #include "P_typdcl.h"
#include "D_graph.h"
#include "P_Pparser.h"
#include "CFrag.h"
// #include "P_datatype.h"
// class    P_datatype;
// #include "P_datavalue.h"
// class    P_datavalue;

class P_DataDef : public P_Pparser
{
public:

       P_DataDef(xmlTextReaderPtr,D_graph*);
       ~P_DataDef();

       int          InsertSubObject(string);
       int          SetObjectProperty(string, string);

       // P_datavalue* value; // V3 : an object that generates an initialiser
       CFrag*          code;  // V4 : an initialiser literal in a C fragment

private:

static const set<string>tags;
static const set<string>attributes;
  
             D_graph* parent;
             // P_datatype* dType; // need type to decode value
};

#endif

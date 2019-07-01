#ifndef _P_DATADECL_H_
#define _P_DATADECL_H_

#include "P_typdcl.h"
#include "P_Pparser.h"
#include "CFrag.h"
// #include "P_datatype.h"
// class P_datatype;

class P_DataDecl : public P_Pparser
{
public:

       P_DataDecl(xmlTextReaderPtr,P_typdcl*,string="");
       ~P_DataDecl();

       int         InsertSubObject(string);
       int         SetObjectProperty(string, string);

       string      typName;
       // P_datatype* type; // V3: data is a type object
       CFrag*      code;    // V4: data is a code fragment

private:

// static const set<string> tags_init();
static const set<string> attrs_init();

static const set<string>tags;
static const set<string>attributes;

             P_typdcl* parent; // all data declarations are children of the graph type
};

#endif

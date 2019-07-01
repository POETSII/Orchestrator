#ifndef _P_TYPEDEF_H_
#define _P_TYPEDEF_H_

#include "P_typdcl.h"
#include "P_Pparser.h"
class P_datatype;
//#include "P_datatype.h"

class P_TypeDef : public P_Pparser
{
public:

       P_TypeDef(xmlTextReaderPtr,P_typdcl*);
       ~P_TypeDef();

       int         InsertSubObject(string);
       int         SetObjectProperty(string, string);

       string      id;
       P_datatype* typdef;

private:

static const set<string> tags_init();
static const set<string> attrs_init();

static const set<string>tags;
static const set<string>attributes;
  
             bool isText; // detect Documentation internal tags
             P_typdcl* parent;
};

#endif

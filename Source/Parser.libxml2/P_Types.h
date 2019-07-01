#ifndef _P_TYPES_H_
#define _P_TYPES_H_

#include "P_Pparser.h"
#include "P_typdcl.h"

class P_Types : public P_Pparser
{
public:

       P_Types(xmlTextReaderPtr,P_typdcl*);
       ~P_Types();

       int       InsertSubObject(string);
inline int       SetObjectProperty(string prop, string value) {return -ERR_UNEXPECTED_ATTR;};

private:

static const set<string> tags_init();

static const set<string>tags;
static const set<string>attributes;

             P_typdcl* parent;
};

#endif

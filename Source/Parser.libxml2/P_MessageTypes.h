#ifndef _P_MESSAGETYPES_H_
#define _P_MESSAGETYPES_H_

#include "P_Pparser.h"
#include "P_typdcl.h"

class P_MessageTypes : public P_Pparser
{
public:

       P_MessageTypes(xmlTextReaderPtr,P_typdcl*);
       ~P_MessageTypes();

       int       InsertSubObject(string);
inline int       SetObjectProperty(string prop, string value) {return -ERR_UNEXPECTED_ATTR;};

private:

static const set<string> tags_init();

static const set<string>tags;
static const set<string>attributes;

             P_typdcl* parent;
};

#endif

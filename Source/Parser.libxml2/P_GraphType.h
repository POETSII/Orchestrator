#ifndef _P_GRAPHTYPE_H_
#define _P_GRAPHTYPE_H_

#include "P_Annotation.h"
#include "P_typdcl.h"
#include "OrchBase.h"

class P_GraphType : public P_Annotation
{
public:

       P_GraphType(xmlTextReaderPtr,OrchBase*);
       ~P_GraphType();

// stub hash check for pure virtual function in P_Annotation
inline bool      CheckHash() {return false;};
 
       int       InsertSubObject(string);
       int       SetObjectProperty(string, string);

       string    id;
       P_typdcl* typdcl;

private:

static const set<string> tags_init();
static const set<string> attrs_init();

static const set<string>tags;
static const set<string>attributes;

             OrchBase* parent;
};

#endif

#ifndef _P_MESSAGETYPE_H_
#define _P_MESSAGETYPE_H_

#include "P_Annotation.h"
#include "P_typdcl.h"
#include "P_devtyp.h"

class P_MessageType : public P_Annotation
{
public:

       P_MessageType(xmlTextReaderPtr,P_typdcl*);
       ~P_MessageType();

inline bool       CheckHash() {return false;};
       
       int        InsertSubObject(string);
       int        SetObjectProperty(string, string);

       string     id;
       P_message* message;

private:

static const set<string> tags_init();
static const set<string> attrs_init();

static const set<string>tags;
static const set<string>attributes;

             P_typdcl* parent;
};

#endif

#ifndef _P_DEVICETYPE_H_
#define _P_DEVICETYPE_H_

#include "P_Annotation.h"
#include "P_typdcl.h"
#include "P_devtyp.h"

class P_DeviceType : public P_Annotation
{
public:

       P_DeviceType(xmlTextReaderPtr,P_typdcl*);
       ~P_DeviceType();

// stub hash check for pure virtual function in P_Annotation
inline bool      CheckHash() {return false;};       

       int       InsertSubObject(string);
       int       SetObjectProperty(string, string);

       string    id;
       P_devtyp* devtyp;

private:

static const set<string> tags_init();
static const set<string> attrs_init();

static const set<string>tags;
static const set<string>attributes;

             P_typdcl* parent;
};

#endif

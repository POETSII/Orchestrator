#ifndef _P_INPUTPIN_H_
#define _P_INPUTPIN_H_

#include "P_devtyp.h"
#include "P_pintyp.h"
#include "P_Pparser.h"

class P_InputPin : public P_Pparser
{
public:

       P_InputPin(xmlTextReaderPtr,P_devtyp*);
       ~P_InputPin();

       int       InsertSubObject(string);
       int       SetObjectProperty(string, string);

       string    id;
       string    msgId;
       P_pintyp* pintyp;

private:

static const set<string> tags_init();
static const set<string> attrs_init();

static const set<string>tags;
static const set<string>attributes;

             P_devtyp* parent;
};

#endif

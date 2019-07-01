#ifndef _P_CODEFRAGMENT_H_
#define _P_CODEFRAGMENT_H_

#include "P_Pparser.h"
#include "CFrag.h"

class P_CodeFragment : public P_Pparser
{
public:

       P_CodeFragment(xmlTextReaderPtr);
       ~P_CodeFragment();

       int       InsertSubObject(string);
inline int       SetObjectProperty(string prop, string value) {return -ERR_UNEXPECTED_ATTR;};

       CFrag*    code;

private:

static const set<string>tags;
static const set<string>attributes;

};

#endif

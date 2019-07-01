#ifndef _P_EDGEINSTANCES_H_
#define _P_EDGEINSTANCES_H_

#include "P_Pparser.h"
#include "P_task.h"

class P_EdgeInstances : public P_Pparser
{
public:

       P_EdgeInstances(xmlTextReaderPtr,P_task*);
       ~P_EdgeInstances();

       int       InsertSubObject(string);
       int       SetObjectProperty(string, string);

private:

static const set<string> tags_init();
static const set<string> attrs_init();

static const set<string>tags;
static const set<string>attributes;

             P_task* parent;
};

#endif

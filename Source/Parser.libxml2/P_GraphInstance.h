#ifndef _P_GRAPHINSTANCE_H_
#define _P_GRAPHINSTANCE_H_

#include "P_Annotation.h"
#include "P_task.h"

class P_GraphInstance:public P_Annotation
{
public:

       P_GraphInstance(xmlTextReaderPtr,OrchBase*);
       ~P_GraphInstance();

       int       InsertSubObject(string);
       int       SetObjectProperty(string, string);

       string    id;
       P_task*   task;

protected:

             string    graphTypeID;
             string    supervisorID;
       
private:

static const set<string> groups_init();
static const set<string> tags_init();
static const set<string> attrs_init();

static const set<string>groups;
static const set<string>tags;
static const set<string>attributes;

       OrchBase* parent;
};

#endif

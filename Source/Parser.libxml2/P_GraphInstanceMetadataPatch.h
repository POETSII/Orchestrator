#ifndef _P_GRAPHINSTANCEMETADATAPATCH_H_
#define _P_GRAPHINSTANCEMETADATAPATCH_H_

#include "P_Annotation.h"
#include "P_task.h"

class P_GraphInstanceMetadataPatch:public P_Annotation
{
public:

       P_GraphInstanceMetadataPatch(xmlTextReaderPtr,OrchBase*);
       ~P_GraphInstanceMetadataPatch();

inline bool      CheckHash() {return false;};
       
       int       InsertSubObject(string);
       int       SetObjectProperty(string, string);

       P_task*   graph;   // pointer to the task

private:

       string            graphInstanceID; // id of the task being patched

static const set<string> tags_init();
static const set<string> attrs_init();

static const set<string>tags;
static const set<string>attributes;

       OrchBase* parent;
};

#endif

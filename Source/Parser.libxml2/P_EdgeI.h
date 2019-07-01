#ifndef _P_EDGEI_H_
#define _P_EDGEI_H_

#include "P_task.h"
#include "P_Pparser.h"
#include "P_pin.h"

// auxiliary structure to hold graph data that may
// need to be looked up later
typedef struct EdgeEnd
{
        P_pin* pin;
        unsigned devIdx; // device index in the graph
} EdgeEnd_t;

class P_EdgeI : public P_Pparser
{
public:

       P_EdgeI(xmlTextReaderPtr,P_task*);
       ~P_EdgeI();

       int        InsertSubObject(string);
       int        SetObjectProperty(string, string);

       string     edgeStr;
       unsigned   gIdx;
       unsigned   sIdx;
       EdgeEnd_t  src;
       EdgeEnd_t  dst;

private:

static const set<string> tags_init();
static const set<string> attrs_init();

static const set<string>tags;
static const set<string>attributes;

             string inProps;
	     string inState;
             bool isMetaData;
             P_task* parent;
};

#endif

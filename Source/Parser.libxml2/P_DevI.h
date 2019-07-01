#ifndef _P_DEVI_H_
#define _P_DEVI_H_

#include "P_task.h"
#include "P_Pparser.h"
#include "P_device.h"

class P_DevI : public P_Pparser
{
public:

       P_DevI(xmlTextReaderPtr,P_task*);
       ~P_DevI();

       int        InsertSubObject(string);
       int        SetObjectProperty(string, string);

       string     id;
       string     devtype;
       P_device*  device;

private:

static const set<string> tags_init();
static const set<string> attrs_init();

static const set<string>tags;
static const set<string>attributes;

             string properties; // V4 data initialisers
	     string state;
             bool isMetaData;
             P_task* parent;
};

#endif
